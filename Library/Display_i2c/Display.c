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

#define height_up_munu 15                                            // ����� �������� ������ ����
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
        FontSet(font);
        Display_TopBar(menu_s);

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

#define line_indentation 2 //  ������� ������� ����� ����� � ������
#define end_line 128-2*line_indentation //  ������� ������� ����� ����� � ������
#define line_ind_top 11 // ����� ����� ������
#define back_pic_pos_x 0    // ������ ������ ���������� ����� ���� x
#define back_pic_pos_y 4    // ������ ������ ���������� ����� ���� y
#define size_back_pic_x 3   // ������ ������������ �� x
#define size_back_pic_y 3   // ������ ������������ �� y
#define top_pic_last_munu 1 // ������ ������ �� �������� ����������� ������ ����
#define left_pic_last_munu 7 // ������ ����� �� �������� ����������� ������ ����
#define top_GSM_status 2    // ������ ������ �� ������� �����
#define width_GSM_status 15  //  ������ ������ ������ ������� �����
#define top_akb_status 1    // ������ ������ �� ������ ������
#define width_akb_status 7  //  ������ ������ ������ ������
extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;
char str[4];
int right_ot = 128-12-2;    // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������

void Display_TopBar(menuItem *CurrentMenu){

    OLED_DrawHLine(line_indentation, line_ind_top, end_line, 1);
    if ((void *)CurrentMenu->Parent != (void *)&NULL_ENTRY){
        OLED_DrawTriangleFill(back_pic_pos_x, back_pic_pos_y, back_pic_pos_x+size_back_pic_x, back_pic_pos_y+size_back_pic_y, back_pic_pos_x+size_back_pic_x, back_pic_pos_y-size_back_pic_y);
        OLED_DrawPixel(back_pic_pos_x+1, back_pic_pos_y);
        OLED_DrawStr(CurrentMenu->Parent, left_pic_last_munu, top_pic_last_munu, 1);
    }

    sprintf(str, "%d", ADC_AKB_Proc);
    if (ADC_AKB_Proc<10) str[1] = '%';
    else if (ADC_AKB_Proc<100){ str[2] = '%'; right_ot-=6;}
    else {str[3] = '%'; right_ot-=12;}

    
    OLED_DrawStr(str, right_ot, top_akb_status+1, 1);
    right_ot -= width_akb_status;

    float c = ADC_AKB_Proc*5/100+1;
    if (ADC_AKB_Proc == 0) c = 0;
    //OLED_DrawRectangle(right_ot+2, top_akb_status+7-c, right_ot+3, top_akb_status+2+c);
    for (c; c>0;c--){
        OLED_DrawHLine(right_ot+2,  top_akb_status+8-c, 2, 1);
    }
    OLED_DrawXBM(right_ot, top_akb_status, akb);
    right_ot -= width_GSM_status;
    

    if (GSM_Signal_Level<0){
        right_ot+=3;
        OLED_DrawXBM(right_ot, top_GSM_status, no_signal);
    }
    if (GSM_Signal_Level==0){
        OLED_DrawXBM(right_ot, top_GSM_status, signal_0);
    }
    if (GSM_Signal_Level==1){
        OLED_DrawXBM(right_ot, top_GSM_status, signal_1);
    }
    if (GSM_Signal_Level==2){
        OLED_DrawXBM(right_ot, top_GSM_status, signal_2);
    }
    if (GSM_Signal_Level==3){
        OLED_DrawXBM(right_ot, top_GSM_status, signal_3);
    }
    right_ot = 128-12-2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������

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
