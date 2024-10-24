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

#define time_led_cursor 500 // ����� ���������� ��������� �������� ��� ����� ������
#define time_updateDisplay 20000 // ����� ���������� ������ ��� ����� ������

/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// �������������� ������
int mode_redact = 0; // 1 - ����� �������������� ������, 0 - ����� ������������ �������
int pos_redact = 0; // ������� ��� ��������������
extern char c_Time[]; // �� settings
extern char c_Date[]; // �� settings
int led_cursor = 1;
extern int time_update_display;
extern xSemaphoreHandle Display_semaphore;
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


menuItem Null_Menu = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
#define NEXT_MENU Null_Menu
#define PREVISION_MENU Null_Menu
#define PARENT_MENU Null_Menu
#define CHILD_MENU Null_Menu
#define ACTION_MENU Null_Menu
#define SELECT_BAR Null_Menu
#define DATA_IN Null_Menu
#define DATA_OUT Null_Menu
#define NULL_ENTRY Null_Menu


#define MAKE_MENU(Id, Name_rus, Name_en, Num_menu, add_signat_ru, add_signat_en, Next, Previous, Parent, Child, action, select_bar, data_in, Data_out) \
    extern menuItem Next;                                                                                          \
    extern menuItem Previous;                                                                                      \
    extern menuItem Parent;                                                                                        \
    extern menuItem Child;                                                                                         \
    menuItem Id = {Name_rus, Name_en, 0, add_signat_ru, add_signat_en, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&action, (menuSelect_item *)&select_bar, (char *)&data_in, (char *)&Data_out}


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

#define height_up_munu 14                                            // ����� �������� ������ ����
#define dist_y (int)((64 - height_up_munu) / (max_munu_in_page))+2 // ���������� ����� �������� ����
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
char Time_char[1] = {0x01}; // ����� ��� ������ �������, ����� ��������� ��������������, ����������� � ���� DATA_OUT
char Data_char[1] = {0x02}; // ����� ��� ������ ����, ����� ��������� ��������������, ����������� � ���� DATA_OUT
extern char char_ADC_in_temp[];
extern char char_ADC_Height[];
extern char char_ADC_Height_correct[];
extern char ID_board[];
extern char ver_board[];
extern char ver_programm[];


MAKE_MENU(Menu_1, "������", "Modes",                     0, "","", Menu_2,         PREVISION_MENU,     PARENT_MENU, Menu_1_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_1, "����", "Cycle",                 0, "","", Menu_1_2,       PREVISION_MENU,     Menu_1, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_2, "����", "Test",                  0, "","", Menu_1_3,       Menu_1_1,       Menu_1, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_3, "���������", "Data",             0, "","", NEXT_MENU,     Menu_1_2,       Menu_1, Menu_1_3_1, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_3_1, "����������", "Data",      0, "�","�", Menu_1_3_2,     PREVISION_MENU,     Menu_1_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "�������", "Data",         0, "�.","", Menu_1_3_3,     Menu_1_3_1,     Menu_1_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "���. ����.", "Data",      0, "�.","", NEXT_MENU,     Menu_1_3_2,     Menu_1_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "���������", "Settings",               0, "","", Menu_3,         Menu_1,         PARENT_MENU, Menu_2_1, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_1, "����", "Data",                 0, "","", Menu_2_2,       PREVISION_MENU,     Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, c_Date, Time_char);
    MAKE_MENU(Menu_2_2, "�����", "Time",                0, "","", Menu_2_3,       Menu_2_1,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, c_Time, Data_char);
    MAKE_MENU(Menu_2_3, "����� ���", "time sleep",       0, "�","h", Menu_2_4,       Menu_2_2,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, c_time_sleep_h, DATA_OUT);
    MAKE_MENU(Menu_2_4, "����� ���", "time sleep",       0, "�","m", Menu_2_5,       Menu_2_3,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, c_time_sleep_m, DATA_OUT);
    MAKE_MENU(Menu_2_5, "���. ��.", "Modes",             0, "�","m", Menu_2_6,     Menu_2_4,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_6, "����. ���.", "Modes",           0, "�","m", Menu_2_7,       Menu_2_5,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_7, "�����", "GSM",                  0, "","", Menu_2_8,       Menu_2_6,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_8, "���. �����", "Modes",           0, "","", Menu_2_9,       Menu_2_7,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_9, "RS485", "RS485",                0, "","", Menu_2_10,       Menu_2_8,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_10, "��. ���.", "Modes",            0, "","", Menu_2_11,      Menu_2_9,       Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_11, "�����. ����", "Modes",         0, "","", Menu_2_12,      Menu_2_10,       Menu_2, Menu_2_11_1, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_1, "���. �����.", "Modes",   0, "","", Menu_2_11_2,    PREVISION_MENU,     Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_2, "���. �����", "Modes",    0, "","", Menu_2_11_3,    Menu_2_11_1,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_3, "�������", "Modes",       0, "","", Menu_2_11_4,    Menu_2_11_2,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_4, "���", "Modes",           0, "","", Menu_2_11_5,  Menu_2_11_3,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_5, "����. 1", "Modes",       0, "","", Menu_2_11_6,    Menu_2_11_4,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_6, "����. 2", "Modes",       0, "","", Menu_2_11_7,    Menu_2_11_5,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_7, "����. 3", "Modes",       0, "","", Menu_2_11_8,    Menu_2_11_6,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_8, "��������", "Modes",      0, "","", Menu_2_11_9,    Menu_2_11_7,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_9, "���������", "Modes",     0, "","", NEXT_MENU,     Menu_2_11_8,    Menu_2_11, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_12, "�����", "Modes",               0, "","", Menu_2_13,      Menu_2_11,      Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_13, "������. SD", "Modes",          0, "","", NEXT_MENU,     Menu_2_12,      Menu_2, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_3, "��������", "Info",                    0, "","", Menu_4,         Menu_2,         PARENT_MENU, Menu_3_1, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_3_1, "ID ����.", "ID Device",         0, "","", Menu_3_2,       PREVISION_MENU,     Menu_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, ID_board);    
    MAKE_MENU(Menu_3_2, "���. �����.", "Modes",          0, "","", Menu_3_3,       Menu_3_1,       Menu_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, ver_board);
    MAKE_MENU(Menu_3_3, "���. ��", "Modes",              0, "","", NEXT_MENU,     Menu_3_2,       Menu_3, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, ver_programm);
MAKE_MENU(Menu_4, "����������", "Instruction",           0, "","", NEXT_MENU,     Menu_3,         PARENT_MENU, CHILD_MENU, ACTION_MENU,SELECT_BAR, DATA_IN, DATA_OUT);

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;


/// ���������� ����� ������ ��� ����� ����� � ��������� 
int search_len_mass(char *string){
    int counter = 0; //
    for (int i = 0; i<11;i++){
        if ((string[i] != '.') && (string[i] != ':')) counter++;
        if (string[i] == 0){
            return counter-1;
        }
    }
    return -1;
}

/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� �����                                          0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
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
        int counter = OLED_GetWidthStr(menu->data_in) + 2 + len_end_symbol;
        OLED_DrawStr(menu->data_in, winth_display - counter, pos_y * dist_y + height_up_munu, 1);

        /// ��������� �����
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            //  ����� ����� ������ �������. pos_redact - ������� ������
            // int len = search_len_mass(selectedMenuItem->data_in); // ����� �������

            char string_pos_line[10] = {};
            int pos_x_line = 0;
            int i = 0;
            for (;((pos_x_line < pos_redact+1) || (i>10)); i++)
            {
                string_pos_line[i] = selectedMenuItem->data_in[i];
                if ((selectedMenuItem->data_in[i] != '.') && (selectedMenuItem->data_in[i] != ':'))
                {
                    pos_x_line++;
                }
            }
            int len_to_char = OLED_GetWidthStr(string_pos_line);
            char array[2] = {};
            array[0] = selectedMenuItem->data_in[i-1];
            int len_select_char = OLED_GetWidthStr(array)-1;
            OLED_DrawHLine(winth_display - counter + len_to_char-len_select_char-1, select_menu_in_page * dist_y + height_up_munu + 8, len_select_char, led_cursor=!led_cursor);
        }   
        ///
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

    if (selectedMenuItem->data_out != (void *)&NULL_ENTRY)
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

    int c = ADC_AKB_Proc*5/100+1;
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

// ���� � ����� ��������������, ���� ���� ��������������� ����
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        // ���� ��������
        mode_redact = 1;
        pos_redact = 0;
        time_update_display = time_led_cursor;
        xSemaphoreGive(Display_semaphore);
    }
}
void redact_end(){
	if ((mode_redact == 1) && (selectedMenuItem->data_in==c_Date)){
		RTC_set_date();
	}
	if ((mode_redact == 1) && (selectedMenuItem->data_in==c_Time)){
		RTC_set_time();
	}

    pos_redact = len-1;
    mode_redact = 0;
    led_cursor = 1;
    time_update_display = time_updateDisplay;
}

void up_redact(){
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){
        // �������� ��� �����. ������� �����
    }
}
void up()
{
    if (!(mode_redact == 0)){ up_redact(); return;}
    if (selectedMenuItem->Previous != (void *)&NULL_ENTRY)
    {
        if (select_menu_in_page > 0)
            select_menu_in_page -= 1;
        menuChange(selectedMenuItem->Previous);
    }
}


void down_redact(){
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){
        // �������� ��� �����. ������� ����
    }
}
void down()
{
    if (!(mode_redact == 0)){ down_redact(); return;}
    if (selectedMenuItem->Next != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Next);
        if (select_menu_in_page + 1 < max_munu_in_page)
            select_menu_in_page += 1;
    }
}


void left_redact(){
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){
        pos_redact--;
        led_cursor = 0;
        if (pos_redact<0) {
            mode_redact = 0;
            led_cursor = 1;
            time_update_display = time_updateDisplay;
        }
        // �������� ��� �����. ������� ������
    }
    
    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY){
        // �������� ��� ���������
    }
}
void left()
{
    if (!(mode_redact == 0)){ left_redact(); return;}
    if (selectedMenuItem->Parent != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Parent);
        select_menu_in_page = selectedMenuItem->Num_menu;
    }
}


void right_redact(){
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){
        int len = search_len_mass(selectedMenuItem->data_in);
        pos_redact++;
        led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������ 
        if (pos_redact>=len){
            redact_end();
        }
        
        // �������� ��� �����. ������� ������
    }
    
    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY){
        // �������� ��� ���������
    }
}

void right()
{
    if (mode_redact == 1){ right_redact(); return;}

    if (selectedMenuItem->action != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->action(); //  ������ �� ������� ��� ��������
        return;
    }
    mode_check(); // ����������� ������

    if (selectedMenuItem->Child != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->Num_menu = select_menu_in_page;
        select_menu_in_page = 0;
        menuChange(selectedMenuItem->Child);
    }
}
void ok(){
    if (mode_redact == 1)
    {
        redact_end();
        return;
    }
    mode_check();
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

// �������������� ������� � ������  :  � .
void data_redact_pos(int position, char data, int len){
    int counter_pos = 0;
    for (int i = 0; i<10; i++){
        char Test = selectedMenuItem->data_in[i];
        if (counter_pos == position){
            selectedMenuItem->data_in[i] = data;
            led_cursor = 0;
            return;
        }
        if ((selectedMenuItem->data_in[i+1] != '.') && (selectedMenuItem->data_in[i+1] != ':'))
            counter_pos++;
    }

}


void key_press_data_write(char data)
{
    int len = search_len_mass(selectedMenuItem->data_in);
    data_redact_pos(pos_redact, data, len);
    pos_redact++;
    led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������ 
    if (pos_redact>=len)
        pos_redact = len-1;
}


// ������� �� ������� ������ �� ����������
void Keyboard_processing()
{
    if (Keyboard_press_code != 0xFF)
    {
        // �������
        if (Keyboard_press_code == 'L')
        {
            left();
        }
        if (Keyboard_press_code == 'R')
        {
            right();
        }
        if (Keyboard_press_code == 'U')
        {
            up();
        }
        if (Keyboard_press_code == 'D')
        {
            down();
        }
        if (Keyboard_press_code == 'O')
        {
            ok();
        }
        if (mode_redact == 1)
        {   
            if (Keyboard_press_code == 'P')
            {
                null_fun();
            }
            
            // ����� ����� ������
            
            if (Keyboard_press_code == '0')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '1')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '2')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '3')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '4')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '5')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '6')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '7')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '8')
            {
                key_press_data_write(Keyboard_press_code);
            }
            if (Keyboard_press_code == '9')
            {
                key_press_data_write(Keyboard_press_code);
            }
            
        }
        Keyboard_press_code = 0xFF;
    }
}