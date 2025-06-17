#ifndef SETTINGS_DEFAULT_H
#define SETTINGS_DEFAULT_H

// Компиляция
#define Debug_mode 1                                // Включен или выключен режим отладки

#define Version3_75 1                            // Версия 3.75
#define Version3_79 2                            // Версия 3.79
#define BOARD_VERSION Version3_75                // Под какую версию собирать проект

// Значения по умолчанию для EEPROM
#define DEFAULT_VERSION_PROGRAMM       "Ver0.78"    // Версия программы
#define EEPROM_FORMAT_VERSION           0x00000010U //? Версия формата данных EEPROM

#define DEFAULT_VERSION_PCB            "3.75-A000{" // Версия печатной платы
#define DEFAULT_PASSWORD               "Defau"
#define DEFAULT_TIME_WORK_CHAR         "0"          // Время работы в виде строки

#define DEFAULT_LAST_ERROR_CODE        {0x00, 0x00, 0x00, 0x00}
#define DEFAULT_TIME_WORK_H            0

#define DEFAULT_TIME_SLEEP_H           1  // Время сна (часы)
#define DEFAULT_TIME_SLEEP_M           0  // Время сна (минуты)

#define DEFAULT_PHONE                  "+79653003557"

// Параметры АЦП:
#define DEFAULT_ADC_ION                1.17
#define DEFAULT_ADC_RESISTOR           49.99
#define DEFAULT_GVL_CORRECT            0
#define DEFAULT_K_KOEFF                0
#define DEFAULT_MAX_LVL                15
#define DEFAULT_ZERO_LVL               0
#define DEFAULT_GVL_CORRECT_4M         0.004
#define DEFAULT_GVL_CORRECT_20M        0.02

// Коррекция температуры:
#define DEFAULT_CRORRECT_TEMP_A        0
#define DEFAULT_COLIBRATE_KOEFF        10.81

// Параметры select_bar:
#define DEFAULT_MODE                   0  // Режим работы: 0 - текущие показания, 1 - циклический, 2 - выставка
#define DEFAULT_COMMUNICATION          1  // Включен GSM или нет
#define DEFAULT_RS485_PROT             0  // Протокол RS-485
#define DEFAULT_UNITS_MES              1  // Единицы измерения (метры)
#define DEFAULT_SCREEN_SEVER_MODE      1  // Включить заставку при включении
#define DEFAULT_USB_MODE               0  // Режим работы USB
#define DEFAULT_SAVE_IN                0  // Куда сохранять: 0 - FLASH, 1 - SD, 2 - USB, 3 - Сайт
#define DEFAULT_LEN                    0  // Язык меню
#define DEFAULT_MODE_ADC               0  // Режим работы АЦП: 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
#define DEFAULT_BLOCK                  0  // Блокировка: 0 - разблокировано, 1 - заблокировано




#endif // SETTINGS_H