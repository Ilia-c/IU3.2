#include "Display.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


// ���������� �������� � ������
menuSelect_item GSM_MODE[2] = {&GSM_mode, {"���.", "On"}, {"����", "Off"}};    // ��� ���� GSM
menuSelect_item RS485_MODE[2] = {&RS485_prot, {"����.", "Off"}, "Modbus", "Modbus"};             // 
menuSelect_item UNITS_MODE[2] = {&units_mes, {"����������", "millimeters"}, "�����", "meters"};
////////////////////////////////////////////////////
//                  ������ ����                   //
////////////////////////////////////////////////////
const int max_munu_in_page = 5; // ������������ ���������� ������� ���� �� ��������
int select_menu_in_page = 0;        // ������� ����� ����c
char len = 'r';                 //  r - ������� ����;  e -  ����������

#define height_up_munu 10                                            // ����� �������� ������ ����
#define dist_y (int)((64 - height_up_munu) / (max_munu_in_page)) // ���������� ����� �������� ����
#define pos_x_menu 4                                                // ������ �� ���� ��� �������� ������� ����
#define pos_x_menu_data 100                                             // ������ �� ���� ��� ������ ��������

#define font my5x7fonts


/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� �����                                          0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    2 - ���� �������                                        0x04
    1 - ���� ����                                           0x02
    0 - �� ������� - ��������                               0x01
*/

MAKE_MENU(Menu_1, "������", "Modes",                    {0x40}, 0, "", Menu_2,         NULL_ENTRY,     NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_1, "����", "Cycle",                {0x01}, 0, "", Menu_1_2,       NULL_ENTRY,     Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_2, "����", "Test",                 {0x01}, 0, "", Menu_1_3,       Menu_1_1,       Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_3, "���������", "Data",            {0x40}, 0, "", NULL_ENTRY,     Menu_1_2,       Menu_1, Menu_1_3_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_1, "����������", "Data",     {0x08}, 0, "", Menu_1_3_2,     NULL_ENTRY,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "�������", "Data",        {0x08}, 0, "", Menu_1_3_3,     Menu_1_3_1,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "���. ����.", "Data",     {0x08}, 0, "", NULL_ENTRY,     Menu_1_3_2,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "���������", "Settings",              {0x40}, 0, "", Menu_3,         Menu_1,         NULL_ENTRY, Menu_2_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_1, "����", "Modes",                {0x02}, 0, "", Menu_2_2,       NULL_ENTRY,     Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_2, "�����", "Modes",               {0x04}, 0, "", Menu_2_3,       Menu_2_1,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_3, "����� ���", "Modes",           {0x20}, 0, "", Menu_2_4,       Menu_2_2,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_4, "���. ��.", "Modes",            {0x20}, 0, "��", Menu_2_5,       Menu_2_3,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_5, "����. ���.", "Modes",          {0x20}, 0, "", Menu_2_6,       Menu_2_4,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_6, "�����", "GSM",                 {0x10}, 0, "", Menu_2_7,       Menu_2_5,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_7, "���. �����", "Modes",          {0x10}, 0, "", Menu_2_8,       Menu_2_6,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_8, "RS485", "RS485",               {0x10}, 0, "", Menu_2_9,       Menu_2_7,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_9, "��. ���.", "Modes",            {0x10}, 0, "", Menu_2_10,      Menu_2_8,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_10, "�����. ����", "Modes",        {0x01}, 0, "", Menu_2_11,      Menu_2_9,       Menu_2, Menu_2_10_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_1, "���. �����.", "Modes",  {0x01}, 0, "", Menu_2_10_2,    NULL_ENTRY,     Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_2, "���. �����", "Modes",   {0x20}, 0, "", Menu_2_10_3,    Menu_2_10_1,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_3, "�������", "Modes",      {0x08}, 0, "", Menu_2_10_4,    Menu_2_10_2,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_4, "���", "Modes",          {0x08}, 0, "��", Menu_2_10_5,    Menu_2_10_3,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_5, "����. 1", "Modes",      {0x08}, 0, "", Menu_2_10_6,    Menu_2_10_4,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_6, "����. 2", "Modes",      {0x08}, 0, "", Menu_2_10_7,    Menu_2_10_5,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_7, "����. 3", "Modes",      {0x08}, 0, "", Menu_2_10_8,    Menu_2_10_6,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_8, "��������", "Modes",     {0x08}, 0, "", Menu_2_10_9,    Menu_2_10_7,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_10_9, "���������", "Modes",    {0x08}, 0, "", NULL_ENTRY,     Menu_2_10_8,    Menu_2_10, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_11, "�����", "Modes",              {0x01}, 0, "", Menu_2_12,      Menu_2_10,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_12, "������. SD", "Modes",         {0x01}, 0, "", NULL_ENTRY,     Menu_2_11,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_3, "��������", "Info",                   {0x40}, 0, "", Menu_4,         Menu_2,         NULL_ENTRY, Menu_3_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_1, "ID ����.", "ID Device",        {0x08}, 0, "", Menu_3_2,       NULL_ENTRY,     Menu_3, NULL_ENTRY, NULL_ENTRY, ID_board);    
    MAKE_MENU(Menu_3_2, "���. �����.", "Modes",         {0x08}, 0, "", Menu_3_3,       Menu_3_1,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_board);
    MAKE_MENU(Menu_3_3, "���. ��", "Modes",             {0x08}, 0, "", NULL_ENTRY,     Menu_3_2,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_programm);
MAKE_MENU(Menu_4, "����������", "Instruction",          {0x01}, 0, "", NULL_ENTRY,     Menu_3,         NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

void Display_all_menu()
{ // ����������� ���� ������� ���� �� ��������

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

        OLED_UpdateScreen();
    
}


void Display_punkt_menu(menuItem *menu, int pos_y) // ����������� ������ ������ ����
{
    FontSet(font);
    if (len == 'r'){
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
    }
    else{
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
    }

/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� �����                                          0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    2 - ���� �������                                        0x04
    1 - ���� ����                                           0x02
    0 - �� ������� - ��������                               0x01
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
extern int a;
void Display_TopBar(){
    OLED_DrawXBM(1, 1, signal_low_1);
}

void Display_Keyboard_select()
{
    if (Keyboard_press_code != 0xFF)
    {
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
        //osDelay(200);
    }
}
