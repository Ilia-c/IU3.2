#include "SD.h"

#include "fatfs.h"
#include "string.h"
#include "Settings.h"
#include "main.h"
#include "fatfs.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

/* Внешние переменные, определённые в другом файле */
extern FATFS SDFatFS;
extern SD_HandleTypeDef hsd1;
extern FIL SDFile;
extern char SDPath[4]; // Предположим, что здесь лежит "0:"


extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
extern FRESULT res;
const static char file_log[12] = "0:log.txt";

void format_uint8_t_2(char *buffer, size_t size, uint8_t data) {
    snprintf(buffer, size, "%u", data);
}


HAL_StatusTypeDef SD_mount(void)
{
    // Если температура вне допустимого диапазона – выходим
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE)
    {
        return HAL_ERROR;
    }

    // Проверяем, вставлена ли SD-карта через сигнал определения (CD)
    if (HAL_GPIO_ReadPin(SDMMC1_DET_GPIO_Port, SDMMC1_DET_Pin) != GPIO_PIN_RESET)
    {
        ERRCODE.STATUS |= STATUS_SD_INIT_ERROR;
        // Размонтируем и сбрасываем состояние FATFS
        f_mount(NULL, SDPath, 0);
        memset(&SDFatFS, 0, sizeof(FATFS));
        return HAL_ERROR;
    }

    // Размонтируем файловую систему и сбрасываем структуру FATFS
    f_mount(NULL, SDPath, 0);
    memset(&SDFatFS, 0, sizeof(FATFS));
    
    // Переинициализируем SD-интерфейс:
    // Сначала деинициализируем SD-периферийное устройство,
    // затем инициализируем его заново.
    if (HAL_SD_DeInit(&hsd1) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_SD_INIT_ERROR;
        return HAL_ERROR;
    }
    if (HAL_SD_Init(&hsd1) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_SD_INIT_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_SD_INIT_ERROR;
    
    // Пытаемся смонтировать файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK)
    {
        ERRCODE.STATUS |= STATUS_SD_MOUNT_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_SD_MOUNT_ERROR;
    
    return HAL_OK;
}

void SD_unmount(){
    f_mount(NULL, SDPath, 0);
    memset(&SDFatFS, 0, sizeof(FATFS));
}

void SD_check(void)
{

    if (SD_mount() == HAL_ERROR) return;
    res = f_open(&SDFile, file_log, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        SD_unmount();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_SD_FILE_OPEN_ERROR;

    // Закрываем файл и отмонтируем файловую систему
    f_close(&SDFile);
    SD_unmount();
}

void SD_write_log(char *string, char *file_name)
{
    if (SD_mount() == HAL_ERROR) return;
    char filename[16] = {'\0'};
    sprintf(filename, file_name, SDPath); 
    res = f_open(&SDFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        SD_unmount();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_SD_FILE_OPEN_ERROR;
    res = f_open(&SDFile, filename, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Ошибка открытия/создания файла
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        SD_unmount();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_SD_FILE_OPEN_ERROR;

    res = f_lseek(&SDFile, f_size(&SDFile));
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_SD_CORRUPTED_DATA;
        f_close(&SDFile);
        SD_unmount();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_SD_CORRUPTED_DATA;

    // Пишем строку в файл
    size_t stringLength = strlen(string);
    UINT bytesWritten = 0;
    res = f_write(&SDFile, string, stringLength, &bytesWritten);
    if (res != FR_OK || bytesWritten < stringLength) {
        ERRCODE.STATUS |= STATUS_SD_WRITE_ERROR;
        f_close(&SDFile);
        SD_unmount();
        return;
    }
    ERRCODE.STATUS &= ~STATUS_SD_WRITE_ERROR;

    // Закрываем файл и отмонтируем файловую систему
    f_close(&SDFile);
    SD_unmount();
}


extern uint32_t data_read_adc_in;
void WriteToSDCard(void)
{
    Collect_DATA();
    SD_write_log(save_data, EEPROM.version.VERSION_PCB);
}

void base62_encode(uint64_t value, char *buffer, size_t bufferSize) {
    if (bufferSize < 12) {
        // Недостаточно места в буфере
        return;
    }

    char result[12];
    result[11] = '\0';

    // Алфавит Base62: цифры, заглавные и строчные буквы.
    const char *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    int pos = 10;  // Индекс для заполнения с конца (11 символов, индексы 0..10, где 10 – последний символ до '\0')
    do {
        result[pos--] = alphabet[value % 62];
        value /= 62;
    } while (value > 0);

    // Если число меньше максимального, заполним оставшиеся позиции ведущими '0'
    while (pos >= 0) {
        result[pos--] = '0';
    }

    // Копируем результат в выходной буфер
    strncpy(buffer, result, bufferSize);
}


extern char save_data[CMD_BUFFER_SIZE];
extern RTC_HandleTypeDef hrtc;
extern EEPROM_Settings_item EEPROM;

int containsRussian(const char *str) {
    while (*str) {
        unsigned char ch = (unsigned char)*str;
        if (ch >= 0xC0 && ch <= 0xFF) {
            return 1; // Найден русский символ
        }
        str++;
    }
    return 0;
}
void remove_whitespace(char *str) {
    char *dst = str;
    while (*str) {
        if (!isspace((unsigned char)*str))
            *dst++ = *str;
        str++;
    }
    *dst = '\0';
}

void Collect_DATA(){
    HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));

    // Если в ADC_data.ADC_SI_value_char или ADC_data.ADC_SI_value_correct_char содержатся русские символы,
    // заменяем их на "cable break"
    if (containsRussian(ADC_data.ADC_SI_value_char) || containsRussian(ADC_data.ADC_SI_value_correct_char)) {
        strcpy(ADC_data.ADC_SI_value_char, "cable_break");
        strcpy(ADC_data.ADC_SI_value_correct_char, "cable_break");
    }

    snprintf(save_data, CMD_BUFFER_SIZE,
        "[%s;%s;%s;%s;%s;%s;%s;%s;%02d:%02d%s%02d/%02d/%02d;%s;%s;%u;%u]",
        EEPROM.version.VERSION_PCB,                // строка
        EEPROM.version.password,                   // строка
        ADC_data.ADC_value_char,                   // строка
        ADC_data.ADC_SI_value_char,                // строка (возможно заменённая)
        ADC_data.ADC_SI_value_correct_char,        // строка (возможно заменённая)
        IntADC.ADC_AKB_volts_char,                 // строка
        IntADC.ADC_AKB_Proc_char,                  // строка
        ERRCODE.STATUSCHAR,                        // строка
        Time.Hours, Time.Minutes,     // время (часы, минуты, секунды)
        "-",
        Date.Date, Date.Month, Date.Year,          // дата (день, месяц, год)
        "0",                                       // time_sleep_mode всегда "0"
        "0", 
        EEPROM.time_sleep_m,                       // число
        EEPROM.time_sleep_h);                      // число
        remove_whitespace(save_data);
}

void SETTINGS_REQUEST_DATA(){
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));
    
    snprintf(save_data, CMD_BUFFER_SIZE,
    "[%s;%s]",
    EEPROM.version.VERSION_PCB,              // строка
    EEPROM.version.password);                      // число
}