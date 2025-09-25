#ifndef SETTINGS_DEFAULT_H
#define SETTINGS_DEFAULT_H

// Компиляция
#define Debug_mode 1                                // Включен или выключен режим отладки

#define Version3_75 0xFAEE                       // Версия 3.75
#define Version3_80 0x0EA3                       // Версия 3.80
#define BOARD_VERSION Version3_80                // Под какую версию собирать проект

#if BOARD_VERSION == Version3_80 
    #define i2cDisplay hi2c3
#elif BOARD_VERSION == Version3_75
    #define i2cDisplay hi2c2
#endif

// Значения по умолчанию для EEPROM
#define DEFAULT_VERSION_PROGRAMM       "Ver0.86"    // Версия программы
#define DEFAULT_VERSION_PROGRAMM_UINT16_t  86       // Версия программы
#define DEFAULT_VERSION_BOOTLOADER     "Uncnown"    // Версия программы
#define BOOTLOADER_MAGIC                0xB00B1E00
#define EEPROM_FORMAT_VERSION           0x00000010U //? Версия формата данных EEPROM

#if BOARD_VERSION == Version3_80 
    #define DEFAULT_VERSION_PCB            "3.80-A000{" // Версия печатной платы
#elif BOARD_VERSION == Version3_75
    #define DEFAULT_VERSION_PCB            "3.75-A000{" // Версия печатной платы
#endif

#define DEFAULT_PASSWORD               "Defau"
#define DEFAULT_TIME_WORK_CHAR         "0"          // Время работы в виде строки
#define DEFAULT_TIME_WORK_H            0

#define DEFAULT_TIME_SLEEP_H           1  // Время сна (часы)
#define DEFAULT_TIME_SLEEP_M           0  // Время сна (минуты)

#define DEFAULT_PHONE                  "+79653003557"

// Параметры АЦП:
#define DEFAULT_ADC_ION                1.17f
#define DEFAULT_ADC_RESISTOR           49.99
#define DEFAULT_GVL_CORRECT_4M         0.004
#define DEFAULT_GVL_CORRECT_20M        0.02

// Канал 1
#define DEFAULT_GVL_CORRECT_1            0
#define DEFAULT_REAL_CURRENT_4MA_1      0.004
#define DEFAULT_REAL_CURRENT_20MA_1     0.02
#define DEFAULT_K_KOEFF_1                1
#define DEFAULT_B_KOEFF_1                0.0f
#define DEFAULT_MAX_LVL_1                15
#define DEFAULT_ZERO_LVL_1               0
// Канал 2
#define DEFAULT_GVL_CORRECT_2            0
#define DEFAULT_REAL_CURRENT_4MA_2      0.004
#define DEFAULT_REAL_CURRENT_20MA_2     0.02
#define DEFAULT_K_KOEFF_2                1
#define DEFAULT_B_KOEFF_2                0.0f
#define DEFAULT_MAX_LVL_2                15
#define DEFAULT_ZERO_LVL_2               0
// Канал 3
#define DEFAULT_GVL_CORRECT_3            0
#define DEFAULT_REAL_CURRENT_4MA_3      0.004
#define DEFAULT_REAL_CURRENT_20MA_3     0.02
#define DEFAULT_K_KOEFF_3                1
#define DEFAULT_B_KOEFF_3                0.0f
#define DEFAULT_MAX_LVL_3                15
#define DEFAULT_ZERO_LVL_3               0

#define DEFAULT_TIME_STABLIZED         10  // Время стабилизации сигнала (сек)


// Коррекция температуры:
#define DEFAULT_CRORRECT_TEMP_A        0
#define DEFAULT_COLIBRATE_KOEFF        10.81

#define MQTT 1  // MQTT или HTTP 1- MQTT, 0 - HTTP
#define HTTP 0  // MQTT или HTTP 1- MQTT, 0 - HTTP

// протокол rs-485
#define RS485_OFF 0
#define RS485_ONLY 1

// Параметры select_bar:
#define DEFAULT_MODE                   0  // Режим работы: 0 - текущие показания, 1 - циклический, 2 - выставка
#define DEFAULT_COMMUNICATION          1  // Включен GSM или нет
#define DEFAULT_COMMUNICATION_HTTP_MQTT 1  // MQTT или HTTP 1- MQTT, 0 - HTTP
#define DEFAULT_RS485_PROT             0  // Протокол RS-485
#define DEFAULT_UNITS_MES              1  // Единицы измерения (метры)
#define DEFAULT_SCREEN_SEVER_MODE      1  // Включить заставку при включении
#define DEFAULT_USB_MODE               0  // Режим работы USB
#define DEFAULT_SAVE_IN                0  // Куда сохранять: 0 - FLASH, 1 - SD, 2 - USB, 3 - Сайт
#define DEFAULT_LEN                    0  // Язык меню
#define DEFAULT_MODE_ADC               0  // Режим работы АЦП: 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
#define DEFAULT_BLOCK                  0  // Блокировка: 0 - разблокировано, 1 - заблокировано




#endif // SETTINGS_H