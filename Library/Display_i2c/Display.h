#include "OLED.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"



typedef struct  MAKE_MENU{
    const char  Name_rus[20];    // Название пункта меню на русском
    const char  Name_en[20];     // Название пункта меню на английском
    const char  Type_menu[1];       // тип меню 0b543210
    /* 
        5 - вкладка
        4 - ввод значения int
        3 - ввод значения float
        2 - ввод значения дата
        1 - вывод незменяемого значвения char[]
        0 - по нажатию - действие   
    */
    void        *Next;           // Следующий пункт меню (Id)
	void        *Previous;       // Предыдущий пункт меню (Id)

	void        *Parent;         // Родительский пункт меню  
	void        *Child;          // На какой пункт меню ссылается 

    float       *data_out;
    int         *Data_int;       // данные типа int для редактирования
    float       *Data_float;     // данные типа float для редактирования
} menuItem;


void Display_all_menu();
void Display_punkt_menu(menuItem* menu, int pos_y);
void menuChange(menuItem* NewMenu);
void Keyboard_select();