#include "OLED.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"

typedef struct Menu_item
{
    int *data;              // привязанное значение
    const char Name[2][10]; // Название пункта меню на русском и английском
} menuSelect_item;

typedef struct MAKE_MENU
{
    const char Name_rus[25];  // Название пункта меню на русском
    const char Name_en[25];   // Название пункта меню на английском
    int Num_menu;             // Номер вкладки сверху при переходе
    const char add_signat_ru[5]; // дополнительная надпись справа (еденицы измерения)
    const char add_signat_en[5]; // дополнительная надпись справа (еденицы измерения)

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
    char *data_in;
    char *data_out;
} menuItem;



void Display_all_menu();
void Display_punkt_menu(menuItem *menu, int pos_y);
void Display_TopBar();
void menuChange(menuItem *NewMenu);
void Keyboard_processing();
void convert_string_to_ascii(const char *input, char *output);
char convert_to_ascii(unsigned char c);