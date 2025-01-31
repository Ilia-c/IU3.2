#include "OLED.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#include "MS5193T.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RTC_data.h"
#include "AT24C02.h"

/* ==============================
   ОПИСАНИЕ STATUS (ОШИБКИ УСТРОЙСТВА)
   Теперь uint64_t STATUS позволяет хранить до 64 ошибок
   ============================== */

/* ---------- 1. ОШИБКИ ЭКРАНА SPI ---------- */
#define STATUS_SPI_DISPLAY_INIT_ERROR      0x0000000000000001ULL  // Ошибка инициализации экрана SPI
#define STATUS_SPI_DISPLAY_TX_ERROR        0x0000000000000002ULL  // Ошибка передачи данных на экран
#define STATUS_SPI_DISPLAY_NO_RESPONSE     0x0000000000000004ULL  // Экран не отвечает

/* ---------- 2. ОШИБКИ ВНЕШНЕЙ EEPROM ---------- */
#define STATUS_EEPROM_INIT_ERROR           0x0000000000000008ULL  // Ошибка инициализации EEPROM
#define STATUS_EEPROM_WRITE_ERROR          0x0000000000000010ULL  // Ошибка записи в EEPROM
#define STATUS_EEPROM_READ_ERROR           0x0000000000000020ULL  // Ошибка чтения из EEPROM
#define STATUS_EEPROM_CRC_ERROR            0x0000000000000040ULL  // Ошибка проверки CRC данных EEPROM

/* ---------- 3. ОШИБКИ ВНЕШНЕГО АЦП ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR     0x0000000000000080ULL  // Ошибка инициализации внешнего АЦП
#define STATUS_ADC_EXTERNAL_SENSOR_ERROR   0x0000000000000100ULL  // Ошибка считывания данных с внешнего датчика
#define STATUS_ADC_BOARD_TEMP_ERROR        0x0000000000000200ULL  // Ошибка считывания температуры платы (аналоговый датчик)
#define STATUS_ADC_RANGE_ERROR             0x0000000000000400ULL  // Ошибка диапазона данных АЦП

/* ---------- 4. ОШИБКИ ДАТЧИКОВ ТЕМПЕРАТУРЫ ---------- */
#define STATUS_TEMP_SENSOR_COMM_ERROR      0x0000000000000800ULL  // Ошибка связи с цифровым датчиком температуры
#define STATUS_TEMP_SENSOR_RANGE_ERROR     0x0000000000001000ULL  // Ошибка диапазона данных цифрового датчика температуры

/* ---------- 5. ОШИБКИ RS-485 ---------- */
#define STATUS_RS485_INIT_ERROR            0x0000000000002000ULL  // Ошибка инициализации RS-485 (UART)
#define STATUS_RS485_RX_ERROR              0x0000000000004000ULL  // Ошибка приема данных RS-485
#define STATUS_RS485_TX_ERROR              0x0000000000008000ULL  // Ошибка передачи данных RS-485

/* ---------- 6. ОШИБКИ USB ---------- */
#define STATUS_USB_INIT_ERROR              0x0000000000010000ULL  // Ошибка инициализации USB
#define STATUS_USB_UART_CMD_ERROR          0x0000000000020000ULL  // Ошибка работы USB-UART (неверная команда)
#define STATUS_USB_FLASH_WRITE_ERROR       0x0000000000040000ULL  // Ошибка записи на USB-Flash
#define STATUS_USB_RESERVED_ERROR          0x0000000000080000ULL  // Ошибка резервного режима USB

/* ---------- 7. ОШИБКИ ВНУТРЕННЕЙ FLASH ПАМЯТИ ---------- */
#define STATUS_FLASH_INIT_ERROR            0x0000000000100000ULL  // Ошибка инициализации внутренней Flash памяти
#define STATUS_FLASH_WRITE_ERROR           0x0000000000200000ULL  // Ошибка записи во внутреннюю Flash память
#define STATUS_FLASH_READ_ERROR            0x0000000000400000ULL  // Ошибка чтения из внутренней Flash памяти

/* ---------- 8. ОШИБКИ SD-КАРТЫ ---------- */
#define STATUS_SD_INIT_ERROR               0x0000000000800000ULL  // Ошибка инициализации SD-карты
#define STATUS_SD_MOUNT_ERROR              0x0000000001000000ULL  // Ошибка монтирования файловой системы SD-карты
#define STATUS_SD_WRITE_ERROR              0x0000000002000000ULL  // Ошибка записи на SD-карту
#define STATUS_SD_READ_ERROR               0x0000000004000000ULL  // Ошибка чтения с SD-карты

/* ---------- 9. ОШИБКИ МОДУЛЯ СВЯЗИ (UART) ---------- */
#define STATUS_UART_MODULE_INIT_ERROR      0x0000000008000000ULL  // Ошибка инициализации модуля связи (UART)
#define STATUS_UART_MODULE_TX_ERROR        0x0000000010000000ULL  // Ошибка отправки данных через модуль связи
#define STATUS_UART_MODULE_RX_ERROR        0x0000000020000000ULL  // Ошибка приема данных через модуль связи
#define STATUS_UART_SMS_SEND_ERROR         0x0000000040000000ULL  // Ошибка отправки SMS
#define STATUS_UART_SERVER_COMM_ERROR      0x0000000080000000ULL  // Ошибка связи с сервером
#define STATUS_UART_SERVER_UNAVAILABLE     0x0000000100000000ULL  // Сервер недоступен
#define STATUS_UART_NO_RESPONSE            0x0000000200000000ULL  // Нет ответа от сайта
#define STATUS_UART_WRONG_PASSWORD         0x0000000400000000ULL  // Неверный пароль с сайта

/* ---------- 10. ОШИБКИ ВНЕШНЕГО ПИТАНИЯ (ЗАРЯД АКБ) ---------- */
#define STATUS_POWER_DETECTION_ERROR       0x0000000800000000ULL  // Ошибка определения внешнего питания
#define STATUS_BATTERY_LOW_ERROR           0x0000001000000000ULL  // Низкий уровень заряда АКБ

/* ---------- 11. РАЗДЕЛ ЗАГЛУШЕК ДЛЯ БУДУЩИХ ОШИБОК ---------- */
#define STATUS_RESERVED_1                  0x0000002000000000ULL  // Зарезервировано для будущих ошибок
#define STATUS_RESERVED_2                  0x0000004000000000ULL  // Зарезервировано для будущих ошибок
#define STATUS_RESERVED_3                  0x0000008000000000ULL  // Зарезервировано для будущих ошибок
#define STATUS_RESERVED_4                  0x0000010000000000ULL  // Зарезервировано для будущих ошибок
#define STATUS_RESERVED_5                  0x0000020000000000ULL  // Зарезервировано для будущих ошибок

/* ---------- 12. ОБЩАЯ КРИТИЧЕСКАЯ ОШИБКА ---------- */
#define STATUS_CRITICAL_ERROR              0x0000040000000000ULL  // Критическая ошибка системы (неизвестное состояние)
#define STATUS_UNKNOWN_ERROR               0x0000080000000000ULL  // Неизвестная ошибка системы

/* ==============================
   КОНЕЦ ОПИСАНИЯ STATUS
   ============================== */


typedef struct Menu_item
{
    uint8_t *data;              // привязанное значение
    const char Name[10][2][15]; // Название пункта меню на русском и английском
} menuSelect_item;

typedef void (*DataFormatter)(char *buffer, size_t size, void *data);

typedef struct Menu_item_char
{
    char separators[3];     // разделители, 3 штуки, если нету, то '\0'. последний может использоваться в качестве приписки
    uint8_t Number_of_cells; // количетво ячеек данных
    uint8_t redact_right_end; // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено

    //  ЯЧЕЙКИ
    double **data_double;
    void *data[3];  // ссылки на исходные значения 
    uint8_t data_type[3]; // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов) 4 - float
    uint8_t unsigned_signed[2]; // есть минус или нет 1-да, 0 - нет.  Первое - текущее, второе - промежуточное
    uint8_t len_data_zero[3]; // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    uint8_t len_data_zero_unredact[3]; // ширина ячеек в режиме не редактирования (001 - ширина 3, 23 - ширина 2)
    char data_temp[3][11];   // промежуточные данные (для редактирования)
    int32_t UP_data[3];         // минимальные значения
    int32_t DOWN_data[3];       // максимальные значения
    void (*end_redact_func)(void);
    void (*end_redact_func_2)(double *save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed); // функция вызываемая при окончании редактирования (сохранение значений)
} menuSelect_item_char;



typedef struct MAKE_MENU
{
    const char Name_rus[26];  // Название пункта меню на русском
    const char Name_en[26];   // Название пункта меню на английском
    int Num_menu;             // Номер вкладки сверху при переходе
    const int data_update;             // Обновлять или нет этот пункт меню каждые 100мс (1/0 вкл/выкл)
    menuSelect_item *add_signat;       // дополнительная надпись справа (режим) 0 - отключено 1 - select_bar

    /*  тип меню (char)0b543210
        6 - вкладка                                             0x40
        5 - ввод числа                                          0x20
        4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
        3 - вывод незменяемого значвения char[]                 0x08
        2 - Ввод времени                                        0x04
        1 - Ввод даты                                           0x02
        0 - по нажатию - действие                               0x01
    */
    void *Next;     // Следующий пункт меню (Id)
    void *Previous; // Предыдущий пункт меню (Id)

    void *Parent; // Родительский пункт меню
    void *Child;  // На какой пункт меню ссылается

    void (*action)(void); // действие при нажатии
    menuSelect_item *select_bar;
    menuSelect_item_char *data_in;
    char *data_out;
} menuItem;



void Display_all_menu();
void Display_punkt_menu(menuItem *menu, int pos_y);
void Display_TopBar();
void menuChange(menuItem *NewMenu);
void Keyboard_processing();
void convert_string_to_ascii(const char *input, char *output);
char convert_to_ascii(unsigned char c);
void ADC_Init();
void data_redact_pos(char data);
void Start_video();
void Save_general_format();
void Save_time_format();
void Save_date_format();
void Programm_GVL_CORRECT();
void SAVE_DOUBLE(double **save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed);