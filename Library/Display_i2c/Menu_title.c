#include "Menu_title.h"

#define NULL_ENTRY Null_Menu
menuItem    Null_Menu = {{0x00}, {0x00}, 0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0, (void*)0};


#define MAKE_MENU(Id, Name, Fonts, Type_menu, Next, Previous, Parent, Child, Data_int, Data_float) \
	extern menuItem Next;       \
    extern menuItem Previous;   \
    extern menuItem Parent;     \
    extern menuItem Child;      \
    extern menuItem Data_int;   \
    extern menuItem Data_float; \
	menuItem Id = {Name, Fonts, Type_menu, (void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (int*)&Data_int, (float*)&Data_float}


////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////

MAKE_MENU(Menu11, "Цикл", "font5x7",  0, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu12, "Настройки", "font5x7",  0, Menu13,  Menu11,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu13, "Сведения", "font5x7",  0, Menu14,  Menu12,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu14, "Сброс", "font5x7",  0, NULL_ENTRY,  Menu13,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);

