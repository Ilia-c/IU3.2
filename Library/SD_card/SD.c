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

void format_uint8_t_2(char *buffer, size_t size, uint8_t data) {
    snprintf(buffer, size, "%u", data);
}

void SD_check(void)
{
    // Если температура вне допустимого диапазона – выходим
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE)
        return;

    FRESULT res;
    // Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK) {
        // Обработка ошибки: карта не смонтировалась
        ERRCODE.STATUS |= STATUS_SD_MOUNT_ERROR;
        return;
    }

    char filePath[32];
    // Формируем путь к файлу log.txt
    snprintf(filePath, sizeof(filePath), "%s/log.txt", SDPath);

    // Открываем (или создаем) файл для проверки доступности
    res = f_open(&SDFile, filePath, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Ошибка открытия/создания файла
        ERRCODE.STATUS |= STATUS_SD_FILE_OPEN_ERROR;
        f_mount(NULL, SDPath, 1);
        return;
    }

    // Закрываем файл и отмонтируем файловую систему
    f_close(&SDFile);
    f_mount(NULL, SDPath, 1);
}

void SD_write_log(const char *string)
{
    // Если температура вне допустимого диапазона – выходим
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE)
        return;

    FRESULT res;
    // Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);
    if (res != FR_OK) {
        // Если карта не смонтировалась, выставляем флаг ошибки и выходим
        ERRCODE.STATUS |= STATUS_SD_MOUNT_ERROR;
        return;
    }

    char filePath[32];
    // Формируем путь к файлу log.txt (используем snprintf для безопасности)
    snprintf(filePath, sizeof(filePath), "%s/log.txt", SDPath);

    // Открываем (или создаем) файл для записи в режиме append
    res = f_open(&SDFile, filePath, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Ошибка открытия/создания файла
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

    // Пишем строку в файл
    size_t stringLength = strlen(string);
    UINT bytesWritten = 0;
    res = f_write(&SDFile, string, stringLength, &bytesWritten);
    if (res != FR_OK || bytesWritten < stringLength) {
        ERRCODE.STATUS |= STATUS_SD_WRITE_ERROR;
        f_close(&SDFile);
        f_mount(NULL, SDPath, 1);
        return;
    }

    // Закрываем файл и отмонтируем файловую систему
    f_close(&SDFile);
    f_mount(NULL, SDPath, 1);
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
    char filename[16];
    createFilename(filename, sizeof(filename));
    sprintf(filePath, filename, SDPath); 
    
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
        EEPROM.version.VERSION_PCB,              // строка
        EEPROM.version.password,                 // строка
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