#include "Data_collect.h"

#include "fatfs.h"
#include "string.h"
#include "Settings.h"
#include "main.h"
#include "fatfs.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

/* ������� ����������, ����������� � ������ ����� */
extern FATFS SDFatFS;
extern SD_HandleTypeDef hsd1;
extern FIL SDFile;
extern char SDPath[4]; // �����������, ��� ����� ����� "0:"

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
        // ������������ ����� � ������
        return;
    }

    char result[12];
    result[11] = '\0';

    // ������� Base62: �����, ��������� � �������� �����.
    const char *alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    int pos = 10; // ������ ��� ���������� � ����� (11 ��������, ������� 0..10, ��� 10 � ��������� ������ �� '\0')
    do
    {
        result[pos--] = alphabet[value % 62];
        value /= 62;
    } while (value > 0);

    // ���� ����� ������ �������������, �������� ���������� ������� �������� '0'
    while (pos >= 0)
    {
        result[pos--] = '0';
    }

    // �������� ��������� � �������� �����
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
            return 1; // ������ ������� ������
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
    // ���� � ADC_data.ADC_SI_value_char ��� ADC_data.ADC_1_SI_value_correct_char ���������� ������� �������,
    // �������� �� �� "cable break"
    if (containsRussian(ADC_data.ADC_SI_value_char[0]) || containsRussian(ADC_data.ADC_SI_value_correct_char[0]))
    {
        strcpy(ADC_data.ADC_SI_value_char[0], CableB);
        strcpy(ADC_data.ADC_SI_value_correct_char[0], CableB);
    }

    snprintf(save_data, CMD_BUFFER_SIZE,
            "[%s;%s;%s;%s;%s;%s;%s;%s;%02d:%02d%s%02d/%02d/%02d;%s;%s;%u;%u]",
             Version,         // ������
             Password,            // ������
             ADC_data.ADC_value_char[0],            // ������
             ADC_data.ADC_SI_value_char[0],         // ������ (�������� ���������)
             ADC_data.ADC_SI_value_correct_char[0], // ������ (�������� ���������)
             IntADC.ADC_AKB_volts_char,          // ������
             IntADC.ADC_AKB_Proc_char,           // ������
             ERRCODE.STATUSCHAR,                 // ������
             Time.Hours, Time.Minutes,           // ����� (����, ������, �������)
             "-",
             Date.Date, Date.Month, Date.Year, // ���� (����, �����, ���)
             "0",   // time_sleep_mode ������ "0"
             VERSION_PROGRAMM,
             EEPROM.time_sleep_m,  // �����
             EEPROM.time_sleep_h); // �����
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
             Version, // ������
             Password);   // �����
}