#include "Display.h"
#include "Settings.h"
extern int Display_update;
extern char Keyboard_press_code;
menuItem Null_Menu = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#define NULL_ENTRY Null_Menu

#define MAKE_MENU(Id, Name_rus, Name_en, Type_menu, Next, Previous, Parent, Child, Data_out, Data_int, Data_float) \
    extern menuItem Next;                                                                                          \
    extern menuItem Previous;                                                                                      \
    extern menuItem Parent;                                                                                        \
    extern menuItem Child;                                                                                         \
    extern menuItem Data_out;                                                                                      \
    extern menuItem Data_int;                                                                                      \
    extern menuItem Data_float;                                                                                    \
    menuItem Id = {Name_rus, Name_en, Type_menu, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (float *)&Data_out, (int *)&Data_int, (float *)&Data_float}

////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////
const int max_munu_in_page = 5; // максимальное количество пунктов меню на странице
int select_menu_in_page = 0;        // метущий пункт менюc
char len[2] = "ru";                 //  ru - русский язык;  en -  английский

#define height_up_munu 10                                            // выста верхнего пункта меню
#define dist_y (int)((64 - height_up_munu) / (max_munu_in_page)) // расстояние между пунктами меню
#define pos_x_menu 4                                                 // отступ от края для названий пунктов меню

#define fonts_rus Arial_Rus_7
#define fonts_en Segoe_UI_Eng_10

/*  тип меню (char)0b543210
    5 - вкладка
    4 - ввод значения int
    3 - ввод значения float
    2 - ввод значения дата
    1 - вывод незменяемого значвения char[]
    0 - по нажатию - действие
*/
MAKE_MENU(Menu_1, "Ht;bvs", "Modes", {0b000000}, Menu_2, NULL_ENTRY,  NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_1, "Цикл", "Cycle", {0b000001}, Menu_1_2, NULL_ENTRY,  Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_2, "Тест", "Test",  {0b000001}, Menu_1_3, Menu_1_2,  Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_3, "Показания", "Data",  {0b000000}, NULL_ENTRY,  Menu_1_3, Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_1, "Тепература", "Data",  {0b000000}, Menu12, NULL_ENTRY, Menu_1_3, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_2, "Глубина", "Data",  {0b000000}, Menu12, NULL_ENTRY, Menu_1_3, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_3, "Кор. глуб.", "Data",  {0b000000}, Menu12,  NULL_ENTRY, Menu_1_3, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_2, "Yfcnhjqrb", "Settings", {0b000000}, Menu_3, Menu_1,  NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_1, "Дата", "Modes", {0b000000}, Menu12, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_2, "Время", "Modes", {0b000000}, Menu12, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_3, "Время сна", "Modes", {0b000000}, Menu12, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_4, "Нул. ур.", "Modes", {0b000000}, Menu12, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_5, "Уров. дат.", "Modes", {0b000000}, Menu12, NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_6, "GSM", "GSM",  {0b000000}, Menu12,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_7, "Ток. петля", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_8, "RS485", "RS485",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_9, "Ед. изм.", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_10, "Инжен. меню", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_1, "Авт. Калиб.", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_2, "ID платы", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_3, "Глубина", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_4, "Ток", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_5, "Темп. 1", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_6, "Темп. 2", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_7, "Темп. 3", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_8, "Давление", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_10, "Влажность", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_11, "Сброс", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_12, "Формат. SD", "Modes",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_3, "Cdtltybz", "Modes",  {0b000000}, Menu_4,  Menu_2,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_1, "ID устр.", "ID Device",  {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);    
    MAKE_MENU(Menu_3_2, "Вер. платы.", "Modes", {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_3, "Вер. ПО", "Modes", {0b000000}, Menu12,  NULL_ENTRY,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_4, "Bycnherwbz", "Modes",  {0b000000}, NULL_ENTRY,  Menu_3,  NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);


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
        Display_punkt_menu(menu_s, select_menu_in_page);
        int pos_cursor = select_menu_in_page * dist_y + height_up_munu+2;
        OLED_DrawTriangleFill(0, pos_cursor - 2, 0, pos_cursor + 2, 2, pos_cursor);

        for (int i = select_menu_in_page-1; i >= 0; i--)
        {
            menu_s = (menuItem *)(menu_s->Previous);
            Display_punkt_menu(menu_s, i);
        }

        menu_s = (menuItem *)(selectedMenuItem);

        for (int i = select_menu_in_page+1; i < max_munu_in_page+1; i++)
        {
            menu_s = (menuItem *)(menu_s->Next);
            Display_punkt_menu(menu_s, i);
        }

        Display_update = 0;
        OLED_UpdateScreen();
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
{
    FontSet(len[0] == "r" ? fonts_en : fonts_rus);
    OLED_DrawStr(len[0] == "r" ? menu->Name_en : menu->Name_rus, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
}

void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}

void Keyboard_select()
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
            if ((void *)selectedMenuItem->Next != (void *)&NULL_ENTRY){
                menuChange(selectedMenuItem->Next);
                if (select_menu_in_page+1 < max_munu_in_page) select_menu_in_page+=1;
            } 
        }
        if (Keyboard_press_code == 'U')
        {
            if ((void *)selectedMenuItem->Previous != (void *)&NULL_ENTRY){
                if (select_menu_in_page>0) select_menu_in_page-=1;
                menuChange(selectedMenuItem->Previous);
            } 
        }       
        Keyboard_press_code = 0xFF;
        osDelay(200);
    }
}
