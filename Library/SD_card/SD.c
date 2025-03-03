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

void SD_check_conect(){

}

void SD_check(){
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE) return;
    char read_buff[20] = {0};
	char write_buff[20] = {0};
    
    FRESULT res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    UINT bytesWritten;
    if (res != FR_OK) {
        // Обработка ошибки: карта не смонтировалась
        ERRCODE.STATUS |= STATUS_SD_MOUNT_ERROR;
        return;
    }

    char filePath[32];
    sprintf(filePath, "%s/log.txt", SDPath); 
    res = f_open(&SDFile, filePath, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Ошибка открытия/создания
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        f_mount(NULL, SDPath, 1);
        return;
    }

    char string[100] = "Загрузка в режиме настройки. Дата:";
    char buffer[60] = {'\0'};
    format_uint8_t_2(buffer, sizeof(buffer), Time.Hours);
    strcat(string, buffer);
    strcat(string, ":");
    format_uint8_t_2(buffer, sizeof(buffer), Time.Minutes);
    strcat(string, buffer);
    strcat(string, " Дата:");
    format_uint8_t_2(buffer, sizeof(buffer), Date.Date);
    strcat(string, buffer);
    strcat(string, ".");
    format_uint8_t_2(buffer, sizeof(buffer), Date.Month);
    strcat(string, buffer);
    strcat(string, ". КОД:");
    format_uint8_t_2(buffer, sizeof(buffer), Date.Year);
    strcat(string, buffer);
    base62_encode(ERRCODE.STATUS, buffer, sizeof(buffer));
    strcat(string, buffer);
    strcat(string, "\n");

    // Переходим в конец файла (append-режим)
    res = f_lseek(&SDFile, f_size(&SDFile));
    if (res != FR_OK)
    {
        ERRCODE.STATUS |= STATUS_SD_CORRUPTED_DATA;
        f_close(&SDFile);
        f_mount(NULL, SDPath, 1);
        return;
    }

    res = f_write(&SDFile, string, strlen(string), &bytesWritten);
    if (res != FR_OK || bytesWritten < strlen(string))
    {
        ERRCODE.STATUS |= STATUS_SD_WRITE_ERROR;
        f_close(&SDFile);
        f_mount(NULL, SDPath, 1);
        return;
    }
    // Закрываем файл
    f_close(&SDFile);
    // Отмонтируем
    f_mount(NULL, SDPath, 1);
}

void format_uint8_t_2(char *buffer, size_t size, uint8_t data) {
    snprintf(buffer, size, "%u", data);
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


extern uint32_t data_read_adc_in;
void WriteToSDCard(void)
{
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE) return;
    FRESULT res;
    UINT bytesWritten;

    Collect_DATA();

    // Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK) {
        // Обработка ошибки: карта не смонтировалась
        ERRCODE.STATUS |= STATUS_SD_MOUNT_ERROR;
        return;
    }

    // Открываем (или создаём) файл для записи в корне SD-карты
    // Обратите внимание на использование SDPath и sprintf
    char filePath[32];
    sprintf(filePath, "%s/test.txt", SDPath); // будет "0:/test.txt"
    
    res = f_open(&SDFile, filePath, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Ошибка открытия/создания
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        f_mount(NULL, SDPath, 1);
        return;
    }

    // Переходим в конец файла (append-режим)
    res = f_lseek(&SDFile, f_size(&SDFile));
    if (res != FR_OK) {
        ERRCODE.STATUS |= STATUS_SD_CORRUPTED_DATA;
        f_close(&SDFile);
        f_mount(NULL, SDPath, 1);
        return;
    }

    // Записываем данные
    res = f_write(&SDFile, save_data, strlen(save_data), &bytesWritten);
    if (res != FR_OK || bytesWritten < strlen(save_data)) {
        // Ошибка записи
        ERRCODE.STATUS |= STATUS_SD_WRITE_ERROR;
        f_close(&SDFile);
        f_mount(NULL, SDPath, 1);
        return;
    }

    // Закрываем файл
    f_close(&SDFile);

    // Отмонтируем
    f_mount(NULL, SDPath, 1);
}


extern char save_data[CMD_BUFFER_SIZE];
extern RTC_HandleTypeDef hrtc;
extern EEPROM_Settings_item EEPROM;

void Collect_DATA(){
    HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));

    snprintf(save_data, CMD_BUFFER_SIZE,
    "[%s;%s;%s;%s;%s;%s;%s;%s;%02d/%02d/%02d%s%02d:%02d:%02d;%s;%s;%u;%u]",
    EEPROM.version.VERSION_PCB,              // строка
    EEPROM.version.password,                 // строка
    ADC_data.ADC_value_char,                   // строка
    ADC_data.ADC_SI_value_char,                // строка
    ADC_data.ADC_SI_value_correct_char,        // строка
    IntADC.ADC_AKB_volts_char,                 // строка
    IntADC.ADC_AKB_Proc_char,                  // строка
    ERRCODE.STATUSCHAR,                        // строка
    Date.Date, Date.Month, Date.Year,          // форматируем дату (день, месяц, год)
    ".", 
    Time.Hours, Time.Minutes, Time.Seconds,     // форматируем время (часы, минуты, секунды)
    "0",                                       // time_sleep_mode всегда "0"
    "0", 
    EEPROM.time_sleep_m,                       // число
    EEPROM.time_sleep_h);                      // число
}