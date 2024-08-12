#include "Menu_title.h"

#define NULL_ENTRY Null_Menu
menuItem    Null_Menu = {{0x00},{0x00}, {0x00}, 0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0};


#define MAKE_MENU(Id, Name_rus, Name_en, Fonts, Type_menu, Next, Previous, Parent, Child, Data_out, Data_int, Data_float) \
	extern menuItem Next;       \
    extern menuItem Previous;   \
    extern menuItem Parent;     \
    extern menuItem Child;      \
    extern menuItem Data_out;   \
    extern menuItem Data_int;   \
    extern menuItem Data_float; \
	menuItem Id = {Name_rus, Name_en, Fonts, Type_menu, (void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (float*)&Data_out, (int*)&Data_int, (float*)&Data_float}


////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////

// тип меню - вкладка(0)/значение ввод int(1)/значение ввод float(2)/значение вывод - float(требуется преобразование) (3)/действие(4)/инженерное меню(5). На действие проверяется параметр Next.
void Make_munu_fun()
{
    MAKE_MENU(Menu01, "Режимы", "Modes", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu11, "Цикл", "Cycle", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu11, "Тест", "Test", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu11, "Показания", "Data", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu11, "Тепература", "Data", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu11, "Глубина", "Data", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu11, "Кор. глуб.", "Data", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu01, "Режимы", "Modes", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Дата", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Время", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Время сна", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Нул. ур.", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Уров. дат.", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "GSM", "GSM", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Ток. петля", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "RS485", "RS485", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Ед. изм.", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Инжен. меню", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Авт. Калиб.", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "ID платы", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Глубина", "Modes", "font5x7",  3 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Ток", "Modes", "font5x7",  3 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Темп. 1", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Темп. 2", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Темп. 3", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Давление", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
            MAKE_MENU(Menu01, "Влажность", "Modes", "font5x7",  1 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Сброс", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Формат. SD", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu01, "Сведения", "Modes", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "ID устр.", "ID Device", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);    
        MAKE_MENU(Menu01, "Вер. платы.", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu01, "Вер. ПО", "Modes", "font5x7",  4 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu01, "Инструкция", "Modes", "font5x7",  0 /*тип меню*/, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
}
