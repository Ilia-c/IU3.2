#include "Display.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
//#include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 2 //  ������� ������� ����� ����� � ������
#define end_line winth_display-2*line_indentation //  ������� ������� ����� ����� � ������
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


// RTC

#define width_symbol 6  //  ������ ������ ���� 01.01.2000

/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// �������������� ������
int mode_redact = 0; // 1 - ����� �������������� ������, 0 - ����� ������������ �������
int pos_redact = 0; // ������� ��� ��������������
#define end_symbol ";"
extern char c_Time[]; // �� settings
extern char c_Date[]; // �� settings
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||


extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;


extern int Mode;
extern int GSM_mode;
extern int RS485_prot;
extern int units_mes;

extern char c_time_sleep_h;
extern char c_time_sleep_m;

char str[4];
int right_ot = winth_display-12-2;    // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������


char trans_str[11];

extern char Keyboard_press_code;
extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;


extern RTC_TimeTypeDef s_Time;


menuItem Null_Menu = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#define NULL_ENTRY Null_Menu

#define MAKE_MENU(Id, Name_rus, Name_en, Type_menu, Num_menu, add_signat_ru, add_signat_en, Next, Previous, Parent, Child, data_in, Data_out) \
    extern menuItem Next;                                                                                          \
    extern menuItem Previous;                                                                                      \
    extern menuItem Parent;                                                                                        \
    extern menuItem Child;                                                                                         \
    extern menuItem Data_out;                                                                                      \
    extern menuItem Data_int;                                                                                      \
    extern menuItem Data_float;                                                                                    \
    menuItem Id = {Name_rus, Name_en, Type_menu, 0, add_signat_ru, add_signat_en, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (char *)&data_in, (void *)&Data_out}


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
    5 - ���� ��������                                       0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    0 - �� ������� - ��������                               0x01
*/


MAKE_MENU(Menu_1, "������", "Modes",                    {0x40}, 0, "","", Menu_2,         NULL_ENTRY,     NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_1, "����", "Cycle",                {0x01}, 0, "","", Menu_1_2,       NULL_ENTRY,     Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_2, "����", "Test",                 {0x01}, 0, "","", Menu_1_3,       Menu_1_1,       Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_3, "���������", "Data",            {0x40}, 0, "","", NULL_ENTRY,     Menu_1_2,       Menu_1, Menu_1_3_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_1, "����������", "Data",     {0x08}, 0, "��.","", Menu_1_3_2,     NULL_ENTRY,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "�������", "Data",        {0x08}, 0, "�.","", Menu_1_3_3,     Menu_1_3_1,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "���. ����.", "Data",     {0x08}, 0, "�.","", NULL_ENTRY,     Menu_1_3_2,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "���������", "Settings",              {0x40}, 0, "","", Menu_3,         Menu_1,         NULL_ENTRY, Menu_2_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_1, "����", "Modes",                {0x20}, 0, "","", Menu_2_2,       NULL_ENTRY,     Menu_2, NULL_ENTRY, c_Date, NULL_ENTRY);
    MAKE_MENU(Menu_2_2, "�����", "Modes",               {0x20}, 0, "","", Menu_2_3,       Menu_2_1,       Menu_2, NULL_ENTRY, c_Time, NULL_ENTRY);
    MAKE_MENU(Menu_2_3, "����� ���", "time sleep",      {0x20}, 0, "�","h", Menu_2_4,       Menu_2_2,       Menu_2, NULL_ENTRY, c_time_sleep_h, NULL_ENTRY);
    MAKE_MENU(Menu_2_4, "����� ���", "time sleep",      {0x20}, 0, "�","m", Menu_2_5,       Menu_2_3,       Menu_2, NULL_ENTRY, c_time_sleep_m, NULL_ENTRY);
    MAKE_MENU(Menu_2_5, "���. ��.", "Modes",            {0x20}, 0, "�","", Menu_2_6,     Menu_2_4,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_6, "����. ���.", "Modes",          {0x20}, 0, "�","", Menu_2_7,       Menu_2_5,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_7, "�����", "GSM",                 {0x10}, 0, "","", Menu_2_8,       Menu_2_6,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_8, "���. �����", "Modes",          {0x10}, 0, "","", Menu_2_9,       Menu_2_7,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_9, "RS485", "RS485",               {0x10}, 0, "","", Menu_2_10,       Menu_2_8,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_10, "��. ���.", "Modes",           {0x10}, 0, "","", Menu_2_11,      Menu_2_9,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_11, "�����. ����", "Modes",        {0x01}, 0, "","", Menu_2_12,      Menu_2_10,       Menu_2, Menu_2_11_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_1, "���. �����.", "Modes",  {0x01}, 0, "","", Menu_2_11_2,    NULL_ENTRY,     Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_2, "���. �����", "Modes",   {0x20}, 0, "","", Menu_2_11_3,    Menu_2_11_1,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_3, "�������", "Modes",      {0x08}, 0, "","", Menu_2_11_4,    Menu_2_11_2,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_4, "���", "Modes",          {0x08}, 0, "","", Menu_2_11_5,  Menu_2_11_3,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_5, "����. 1", "Modes",      {0x08}, 0, "","", Menu_2_11_6,    Menu_2_11_4,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_6, "����. 2", "Modes",      {0x08}, 0, "","", Menu_2_11_7,    Menu_2_11_5,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_7, "����. 3", "Modes",      {0x08}, 0, "","", Menu_2_11_8,    Menu_2_11_6,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_8, "��������", "Modes",     {0x08}, 0, "","", Menu_2_11_9,    Menu_2_11_7,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_9, "���������", "Modes",    {0x08}, 0, "","", NULL_ENTRY,     Menu_2_11_8,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_12, "�����", "Modes",              {0x01}, 0, "","", Menu_2_13,      Menu_2_11,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_13, "������. SD", "Modes",         {0x01}, 0, "","", NULL_ENTRY,     Menu_2_12,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_3, "��������", "Info",                   {0x40}, 0, "","", Menu_4,         Menu_2,         NULL_ENTRY, Menu_3_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_1, "ID ����.", "ID Device",        {0x08}, 0, "","", Menu_3_2,       NULL_ENTRY,     Menu_3, NULL_ENTRY, NULL_ENTRY, ID_board);    
    MAKE_MENU(Menu_3_2, "���. �����.", "Modes",         {0x08}, 0, "","", Menu_3_3,       Menu_3_1,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_board);
    MAKE_MENU(Menu_3_3, "���. ��", "Modes",             {0x08}, 0, "","", NULL_ENTRY,     Menu_3_2,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_programm);
MAKE_MENU(Menu_4, "����������", "Instruction",          {0x01}, 0, "","", NULL_ENTRY,     Menu_3,         NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� �����                                          0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    2 - ���� �������                                        0x04
    1 - ���� ����                                           0x02
    0 - �� ������� - ��������                               0x01
*/
void Select_diplay_functions(menuItem *menu, int pos_y)
{
    int len_end_symbol = 0;
    if (len == 'r'){
        OLED_DrawStr(menu->add_signat_ru, winth_display-10, pos_y*dist_y+height_up_munu, 1);
        len_end_symbol = OLED_GetWidthStr(menu->add_signat_ru) + 2;
    }
    else{
        OLED_DrawStr(menu->add_signat_en,  winth_display-10, pos_y*dist_y+height_up_munu, 1);
        len_end_symbol = OLED_GetWidthStr(menu->add_signat_en) + 2;
    }


    if (menu->data_in != (void *)&NULL_ENTRY)
    {
        if (menu->Type_menu == 0x20)
        {
            int counter = OLED_GetWidthStr(menu->data_in) + 2 + len_end_symbol;
            OLED_DrawStr(menu->data_in, winth_display - counter, pos_y * dist_y + height_up_munu, 1);
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // ����������� ������ ������ ����
{
    FontSet(font);
    if (len == 'r')
    {
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y * dist_y + height_up_munu, 1);
    }
    else
    {
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y * dist_y + height_up_munu, 1);
    }

    if (menu->Type_menu & 0x08)
    {
        OLED_DrawStr(menu->data_out, winth_display - 60, pos_y * dist_y + height_up_munu, 1);
    }
}
void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}


extern unsigned int id;
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
    
    //sprintf(str,"ID:0x%X\r\n",id);
    //OLED_DrawStr(str, 1, 1, 1);
    
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
    right_ot = winth_display-12-2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������
}

void Display_all_menu()
{ // ����������� ���� ������� ���� �� ��������

    OLED_Clear(0);

    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);

    Display_punkt_menu(menu_s, select_menu_in_page);
    Select_diplay_functions(menu_s, select_menu_in_page);

    int pos_cursor = select_menu_in_page * dist_y + height_up_munu + 2;
    OLED_DrawTriangleFill(0, pos_cursor - 2, 0, pos_cursor + 2, 2, pos_cursor);

    for (int i = select_menu_in_page - 1; i >= 0; i--)
    {
        menu_s = (menuItem *)(menu_s->Previous);
        Display_punkt_menu(menu_s, i);
        Select_diplay_functions(menu_s, i);
    }

    menu_s = (menuItem *)(selectedMenuItem);

    for (int i = select_menu_in_page + 1; i < max_munu_in_page; i++)
    {
        menu_s = (menuItem *)(menu_s->Next);
        Display_punkt_menu(menu_s, i);
        Select_diplay_functions(menu_s, i);
    }
    OLED_UpdateScreen();
}

void up()
{
    if (selectedMenuItem->Previous != (void *)&NULL_ENTRY)
    {
        if (select_menu_in_page > 0)
            select_menu_in_page -= 1;
        menuChange(selectedMenuItem->Previous);
    }
}

void down()
{
    if (selectedMenuItem->Next != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Next);
        if (select_menu_in_page + 1 < max_munu_in_page)
            select_menu_in_page += 1;
    }
}
void left()
{
    if (selectedMenuItem->Parent != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Parent);
        select_menu_in_page = selectedMenuItem->Num_menu;
    }
}

void right()
{
    if (selectedMenuItem->Child != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->Num_menu = select_menu_in_page;
        select_menu_in_page = 0;
        menuChange(selectedMenuItem->Child);
    }
}
void null_fun(){

}

/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� ��������                                       0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    0 - �� ������� - ��������                               0x01
*/
void select_code_procces(void (*action)(void))
{
    // up_down();
    if (selectedMenuItem->Type_menu == 0x40)
        action(); // 6 - �������                                             0x40
    if (selectedMenuItem->Type_menu == 0x20)
        action(); // 5 - ���� �����                                          0x20
    if (selectedMenuItem->Type_menu == 0x10)
        action(); // 4 - ���������� ������ (��������� ������ �� ���������)   0x10
    if (selectedMenuItem->Type_menu == 0x08)
        action(); // 3 - ����� ������������ ��������� char[]                 0x08
    if (selectedMenuItem->Type_menu == 0x04)
        action(); // 2 - ���� �������                                        0x04
    if (selectedMenuItem->Type_menu == 0x02)
        action(); // 1 - ���� ����                                           0x02
    if (selectedMenuItem->Type_menu == 0x01)
        action(); // 0 - �� ������� - ��������                               0x01
}

void Keyboard_processing(){
    if (Keyboard_press_code != 0xFF)
    {
        // �������
        if (Keyboard_press_code == 'L')
        {
            select_code_procces(left);
            return;
        }
        if (Keyboard_press_code == 'R')
        {
            select_code_procces(right);
            return;
        }
        if (Keyboard_press_code == 'U')
        {
            select_code_procces(up);
            return;
        }
        if (Keyboard_press_code == 'D')
        {
            select_code_procces(down);
            return;
        }

        // ����� ����� ������
        if (Keyboard_press_code == '0')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '1')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '2')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '3')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '4')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '5')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '6')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '7')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '8')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == '9')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == 'O')
        {
            select_code_procces(null_fun);
            return;
        }
        if (Keyboard_press_code == 'P')
        {
            select_code_procces(null_fun);
            return;
        }
    }
}