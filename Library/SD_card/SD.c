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
    uint64_to_hex_str(ERRCODE.STATUS, buffer, sizeof(buffer));
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

void uint64_to_hex_str(uint64_t value, char *buffer, size_t bufferSize)
{
    snprintf(buffer, bufferSize, "0x%016l", (unsigned long long)value);
}

extern uint32_t data_read_adc_in;
void WriteToSDCard(void)
{
    if (ERRCODE.STATUS & STATUS_SD_TEMP_OUT_OF_RANGE) return;
    FRESULT res;
    UINT bytesWritten;

    char string[80] = {'\0'};
    char buffer[50] = {'\0'};
    char dw[2] = ":";
    char rz[2] = ";";
    char po[2] = ".";
    char end[2] = "\n";
    format_uint8_t_2(buffer, sizeof(buffer), Time.Hours);
    strcat(string, buffer);
    strcat(string, dw);
    format_uint8_t_2(buffer, sizeof(buffer), Time.Minutes);
    strcat(string, buffer);
    strcat(string, rz);

    format_uint8_t_2(buffer, sizeof(buffer), Date.Date);
    strcat(string, buffer);
    strcat(string, po);
    format_uint8_t_2(buffer, sizeof(buffer), Date.Month);
    strcat(string, buffer);
    strcat(string, po);
    format_uint8_t_2(buffer, sizeof(buffer), Date.Year);
    strcat(string, buffer);
    strcat(string, rz);

    strcat(string, ADC_data.ADC_MS5193T_temp_char);
    strcat(string, rz);
    strcat(string, ADC_data.ADC_value_char);
    strcat(string, rz);

    sprintf(buffer, "%lu", data_read_adc_in);
    strcat(string, buffer);
    strcat(string, rz);
    strcat(string, end);

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
    res = f_write(&SDFile, string, strlen(string), &bytesWritten);
    if (res != FR_OK || bytesWritten < strlen(string)) {
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