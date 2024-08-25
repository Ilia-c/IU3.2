#include "Display.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int Display_update;
extern char Keyboard_press_code;

extern int Mode;
extern int GSM_mode;
extern int RS485_prot;
extern int units_mes;


menuItem Null_Menu = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#define NULL_ENTRY Null_Menu

#define MAKE_MENU(Id, Name_rus, Name_en, Type_menu, Num_menu, add_signat, Next, Previous, Parent, Child, data_in, Data_out) \
    extern menuItem Next;                                                                                          \
    extern menuItem Previous;                                                                                      \
    extern menuItem Parent;                                                                                        \
    extern menuItem Child;                                                                                         \
    extern menuItem Data_out;                                                                                      \
    extern menuItem Data_int;                                                                                      \
    extern menuItem Data_float;                                                                                    \
    menuItem Id = {Name_rus, Name_en, Type_menu, 0, add_signat, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&data_in, (void *)&Data_out}


// Выбираемые значения и статус
menuSelect_item GSM_MODE[2] = {&GSM_mode, {"Вкл.", "On"}, {"Выкл", "Off"}};    // Вкл откл GSM
menuSelect_item RS485_MODE[2] = {&RS485_prot, {"Выкл.", "Off"}, "Modbus", "Modbus"};             // 
menuSelect_item UNITS_MODE[2] = {&units_mes, {"миллиметры", "millimeters"}, "метры", "meters"};
////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////
const int max_munu_in_page = 5; // максимальное количество пунктов меню на странице
int select_menu_in_page = 0;        // метущий пункт менюc
char len = 'r';                 //  r - русский язык;  e -  английский

#define height_up_munu 10                                            // выста верхнего пункта меню
#define dist_y (int)((64 - height_up_munu) / (max_munu_in_page)) // расстояние между пунктами меню
#define pos_x_menu 4                                                // отступ от края для названий пунктов меню
#define pos_x_menu_data 100                                             // отступ от края для вывода значений

#define fonts_rus my5x7rus
#define fonts_en my5x7en

/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    2 - Ввод времени                                        0x04
    1 - Ввод даты                                           0x02
    0 - по нажатию - действие                               0x01
*/

MAKE_MENU(Menu_1, "Режимы", "Modes",                    {0x40}, 0, "", Menu_2,         NULL_ENTRY,     NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_1, "Цикл", "Cycle",                {0x01}, 0, "", Menu_1_2,       NULL_ENTRY,     Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_2, "Тест", "Test",                 {0x01}, 0, "", Menu_1_3,       Menu_1_1,       Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_3, "Показания", "Data",            {0x40}, 0, "", NULL_ENTRY,     Menu_1_2,       Menu_1, Menu_1_3_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_1, "Тепература", "Data",     {0x08}, 0, "", Menu_1_3_2,     NULL_ENTRY,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "Глубина", "Data",        {0x08}, 0, "", Menu_1_3_3,     Menu_1_3_1,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "Кор. глуб.", "Data",     {0x08}, 0, "", NULL_ENTRY,     Menu_1_3_2,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "Настройки", "Settings",              {0x40}, 0, "", Menu_3,         Menu_1,         NULL_ENTRY, Menu_2_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_1, "Дата", "Modes",                {0x02}, 0, "", Menu_2_2,       NULL_ENTRY,     Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_2, "Время", "Modes",               {0x04}, 0, "", Menu_2_3,       Menu_2_1,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_3, "Время сна", "Modes",           {0x20}, 0, "", Menu_2_4,       Menu_2_2,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_4, "Нул. ур.", "Modes",            {0x20}, 0, "", Menu_2_5,       Menu_2_3,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_5, "Уров. дат.", "Modes",          {0x20}, 0, "", Menu_2_6,       Menu_2_4,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_6, "Связь", "GSM",                 {0x10}, 0, "", Menu_2_7,       Menu_2_5,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_7, "Ток. петля", "Modes",          {0x10}, 0, "", Menu_2_8,       Menu_2_6,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_8, "RS485", "RS485",               {0x10}, 0, "", Menu_2_9,       Menu_2_7,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_9, "Ед. изм.", "Modes",            {0x10}, 0, "", Menu_2_10,      Menu_2_8,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_10, "Инжен. меню", "Modes",        {0x01}, 0, "", Menu_2_11,      Menu_2_9,       Menu_2, Menu_2_10_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_1, "Авт. Калиб.", "Modes",  {0x01}, 0, "", Menu_2_10_2,    NULL_ENTRY,     Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_2, "Ном. платы", "Modes",   {0x20}, 0, "", Menu_2_10_3,    Menu_2_10_1,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_3, "Глубина", "Modes",      {0x08}, 0, "", Menu_2_10_4,    Menu_2_10_2,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_4, "Ток", "Modes",          {0x08}, 0, "", Menu_2_10_5,    Menu_2_10_3,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_5, "Темп. 1", "Modes",      {0x08}, 0, "", Menu_2_10_6,    Menu_2_10_4,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_6, "Темп. 2", "Modes",      {0x08}, 0, "", Menu_2_10_7,    Menu_2_10_5,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_7, "Темп. 3", "Modes",      {0x08}, 0, "", Menu_2_10_8,    Menu_2_10_6,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_8, "Давление", "Modes",     {0x08}, 0, "", Menu_2_10_9,    Menu_2_10_7,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_9, "Влажность", "Modes",    {0x08}, 0, "", NULL_ENTRY,     Menu_2_10_8,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_11, "Сброс", "Modes",              {0x01}, 0, "", Menu_2_12,      Menu_2_10,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_12, "Формат. SD", "Modes",         {0x01}, 0, "", NULL_ENTRY,     Menu_2_11,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_3, "Сведения", "Info",                   {0x40}, 0, "", Menu_4,         Menu_2,         NULL_ENTRY, Menu_3_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_1, "ID устр.", "ID Device",        {0x08}, 0, "", Menu_3_2,       NULL_ENTRY,     Menu_3, NULL_ENTRY, NULL_ENTRY, ID_board);    
    MAKE_MENU(Menu_3_2, "Вер. платы.", "Modes",         {0x08}, 0, "", Menu_3_3,       Menu_3_1,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_board);
    MAKE_MENU(Menu_3_3, "Вер. ПО", "Modes",             {0x08}, 0, "", NULL_ENTRY,     Menu_3_2,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_programm);
MAKE_MENU(Menu_4, "Инструкция", "Instruction",          {0x01}, 0, "", NULL_ENTRY,     Menu_3,         NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);

////////////////////////////////////////////////////
//                Обработка меню                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

void Display_all_menu()
{ // отображение всех пунктов меню на странице
    if (Display_update == 1)
    {
        OLED_Clear(0);

        menuItem *menu_s = (menuItem *)(selectedMenuItem);
        Display_TopBar();


        Display_punkt_menu(menu_s, select_menu_in_page);
        int pos_cursor = select_menu_in_page * dist_y + height_up_munu+2;
        OLED_DrawTriangleFill(0, pos_cursor - 2, 0, pos_cursor + 2, 2, pos_cursor);

        for (int i = select_menu_in_page-1; i >= 0; i--)
        {
            menu_s = (menuItem *)(menu_s->Previous);
            Display_punkt_menu(menu_s, i);
        }

        menu_s = (menuItem *)(selectedMenuItem);
        
        for (int i = select_menu_in_page+1; i < max_munu_in_page; i++)
        {
            menu_s = (menuItem *)(menu_s->Next);
            Display_punkt_menu(menu_s, i);
        }

        Display_update = 0;
        OLED_UpdateScreen();
    }
}

char convert_to_ascii(unsigned char c) {
    switch (c) {
        case 0xD0: case 0xD1:
            return c - 0x20;
        case 0x90: return 'F'; // А
        case 0x91: return ','; // Б
        case 0x92: return 'D'; // В
        case 0x93: return 'U'; // Г
        case 0x94: return 'L'; // Д
        case 0x95: return 'T'; // Е
        case 0x96: return ':'; // Ж
        case 0x97: return 'P'; // З
        case 0x98: return 'B'; // И
        case 0x99: return 'Q'; // Й
        case 0x9A: return 'R'; // К
        case 0x9B: return 'K'; // Л
        case 0x9C: return 'V'; // М
        case 0x9D: return 'Y'; // Н
        case 0x9E: return 'J'; // О
        case 0x9F: return 'G'; // П
        case 0xA0: return 'H'; // Р
        case 0xA1: return 'C'; // С
        case 0xA2: return 'N'; // Т
        case 0xA3: return 'E'; // У
        case 0xA4: return 'A'; // Ф
        case 0xA5: return '{'; // Х
        case 0xA6: return 'W'; // Ц
        case 0xA7: return 'X'; // Ч
        case 0xA8: return 'I'; // Ш
        case 0xA9: return 'O'; // Щ
        case 0xAA: return ']'; // Ъ
        case 0xAB: return 'S'; // Ы
        case 0xAC: return 'M'; // Ь
        case 0xAD: return '\"'; // Э
        case 0xAE: return '>'; // Ю
        case 0xAF: return 'Z'; // Я
        case 0xB0: return 'f'; // а
        case 0xB1: return ','; // б
        case 0xB2: return 'd'; // в
        case 0xB3: return 'u'; // г
        case 0xB4: return 'l'; // д
        case 0xB5: return 't'; // е
        case 0xB6: return ';'; // ж
        case 0xB7: return 'p'; // з
        case 0xB8: return 'b'; // и
        case 0xB9: return 'q'; // й
        case 0xBA: return 'r'; // к
        case 0xBB: return 'k'; // л
        case 0xBC: return 'v'; // м
        case 0xBD: return 'y'; // н
        case 0xBE: return 'j'; // о
        case 0xBF: return 'g'; // п
        case 0x80: return 'h'; // р
        case 0x81: return 'c'; // с
        case 0x82: return 'n'; // т
        case 0x83: return 'e'; // у
        case 0x84: return 'a'; // ф
        case 0x85: return '['; // х
        case 0x86: return 'w'; // ц
        case 0x87: return 'x'; // ч
        case 0x88: return 'i'; // ш
        case 0x89: return 'o'; // щ
        case 0x8A: return ']'; // ъ
        case 0x8B: return 's'; // ы
        case 0x8C: return 'm'; // ь
        case 0x8D: return '\''; // э
        case 0x8E: return '.'; // ю
        case 0x8F: return 'z'; // я
        default: return c;
    }
}

void convert_string_to_ascii(const char *input, char *output) {
    int i = 0, j = 0;

    while (input[i] != '\0') {
        if ((unsigned char)input[i] == 0xD0 || (unsigned char)input[i] == 0xD1) {
            output[j] = convert_to_ascii((unsigned char)input[i + 1]);
            i += 2;
        } else {
            output[j] = input[i];
            i++;
        }
        j++;
    }
    output[j] = '\0';
}

void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
{
    FontSet(len == 'r' ? fonts_rus : fonts_en);
    if (len == 'r'){
        char output[20];
        convert_string_to_ascii(menu->Name_rus, output);
        OLED_DrawStr(output, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
    }
    else{
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
    }

/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    2 - Ввод времени                                        0x04
    1 - Ввод даты                                           0x02
    0 - по нажатию - действие                               0x01
*/
    if (menu->Type_menu & 0x08){
        OLED_DrawStr(menu->data_out, pos_x_menu_data, pos_y*dist_y+height_up_munu, 1);
    }


}

void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}

void Display_TopBar(){
    OLED_DrawXBM(1, 1, signal_low_1);
}

void Display_Keyboard_select()
{
    if (Keyboard_press_code != 0xFF)
    {
        Display_update = 1;
        /*
        FontSet(fonts_en);
        char t[3] = "   ";
        t[0] = Keyboard_press_code;
        OLED_DrawStr(t, 0, 5, 1);
        OLED_UpdateScreen();
        */
        if (Keyboard_press_code == 'D')
        {
            if (selectedMenuItem->Next != (void *)&NULL_ENTRY){
                menuChange(selectedMenuItem->Next);
                if (select_menu_in_page+1 < max_munu_in_page) select_menu_in_page+=1;
            } 
        }
        if (Keyboard_press_code == 'U')
        {
            if (selectedMenuItem->Previous != (void *)&NULL_ENTRY){
                if (select_menu_in_page>0) select_menu_in_page-=1;
                menuChange(selectedMenuItem->Previous);
            }
        }

        if (Keyboard_press_code == 'R')
        {
            if (selectedMenuItem->Type_menu & 0x40)
            {
                if (selectedMenuItem->Child != (void *)&NULL_ENTRY)
                {
                    selectedMenuItem->Num_menu = select_menu_in_page;
                    select_menu_in_page = 0;
                    menuChange(selectedMenuItem->Child);
                }
            }
        }

        if (Keyboard_press_code == 'L')
        {
            if (selectedMenuItem->Parent != (void *)&NULL_ENTRY)
            {
                menuChange(selectedMenuItem->Parent);
                select_menu_in_page = selectedMenuItem->Num_menu;
            }
        }

        Keyboard_press_code = 0xFF;
        osDelay(200);
    }
}
