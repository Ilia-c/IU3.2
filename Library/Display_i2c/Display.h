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
    double *data_double;
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
    menuSelect_item *add_signat;           // дополнительная надпись справа (режим) 0 - отключено 1 - select_bar

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
void SAVE_DOUBLE(double *save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed);