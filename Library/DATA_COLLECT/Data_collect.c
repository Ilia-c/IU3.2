#include "Data_collect.h"

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

void format_uint8_t_2(char *buffer, size_t size, uint8_t data)
{
    snprintf(buffer, size, "%u", data);
}

void base62_encode(uint64_t value, char *buffer, size_t bufferSize)
{
    if (bufferSize < 12)
    {
        // Недостаточно места в буфере
        return;
    }

    char result[12];
    result[11] = '\0';

    // Алфавит Base62: цифры, заглавные и строчные буквы.
    const char *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    int pos = 10; // Индекс для заполнения с конца (11 символов, индексы 0..10, где 10 – последний символ до '\0')
    do
    {
        result[pos--] = alphabet[value % 62];
        value /= 62;
    } while (value > 0);

    // Если число меньше максимального, заполним оставшиеся позиции ведущими '0'
    while (pos >= 0)
    {
        result[pos--] = '0';
    }

    // Копируем результат в выходной буфер
    strncpy(buffer, result, bufferSize);
}

extern char save_data[CMD_BUFFER_SIZE];
extern RTC_HandleTypeDef hrtc;


int containsRussian(const char *str)
{
    while (*str)
    {
        unsigned char ch = (unsigned char)*str;
        if (ch >= 0xC0 && ch <= 0xFF)
        {
            return 1; // Найден русский символ
        }
        str++;
    }
    return 0;
}
void remove_whitespace(char *str)
{
    char *dst = str;
    while (*str)
    {
        if (!isspace((unsigned char)*str))
            *dst++ = *str;
        str++;
    }
    *dst = '\0';
}

void remove_braces_inplace(char *str) {
    char *dst = str;
    while (*str) {
        if (*str != '{') {
            *dst++ = *str;
        }
        str++;
    }
    *dst = '\0';
}

const char CableB[] = "cable_break";
void Collect_DATA()
{
    HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
    HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));

    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);
    char Password[20] = {0};
    strncpy(Password, Main_data.version.password, sizeof(Password)-1);
    remove_braces_inplace(Password);
    // Если в ADC_data.ADC_SI_value_char или ADC_data.ADC_1_SI_value_correct_char содержатся русские символы,
    // заменяем их на "cable break"
    if (containsRussian(ADC_data.ADC_SI_value_char[0]) || containsRussian(ADC_data.ADC_SI_value_correct_char[0]))
    {
        strcpy(ADC_data.ADC_SI_value_char[0], CableB);
        strcpy(ADC_data.ADC_SI_value_correct_char[0], CableB);
    }

    snprintf(save_data, CMD_BUFFER_SIZE,
            "[%s;%s;%s;%s;%s;%s;%s;%s;%02d:%02d%s%02d/%02d/%02d;%s;%s;%u;%u]",
             Version,         // строка
             Password,            // строка
             ADC_data.ADC_value_char[0],            // строка
             ADC_data.ADC_SI_value_char[0],         // строка (возможно заменённая)
             ADC_data.ADC_SI_value_correct_char[0], // строка (возможно заменённая)
             IntADC.ADC_AKB_volts_char,          // строка
             IntADC.ADC_AKB_Proc_char,           // строка
             ERRCODE.STATUSCHAR,                 // строка
             Time.Hours, Time.Minutes,           // время (часы, минуты, секунды)
             "-",
             Date.Date, Date.Month, Date.Year, // дата (день, месяц, год)
             "0",   // time_sleep_mode всегда "0"
             VERSION_PROGRAMM,
             EEPROM.time_sleep_m,  // число
             EEPROM.time_sleep_h); // число
    remove_whitespace(save_data);
}

void SETTINGS_REQUEST_DATA()
{
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));

    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);
    char Password[20] = {0};
    strncpy(Password, Main_data.version.password, sizeof(Password)-1);
    remove_braces_inplace(Password);

    snprintf(save_data, CMD_BUFFER_SIZE,
             "[%s;%s]",
             Version, // строка
             Password);   // число
}