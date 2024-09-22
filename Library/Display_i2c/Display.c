#include "Display.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
//#include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 2 //  отступы верхней линии слева и справа
#define end_line winth_display-2*line_indentation //  отступы верхней линии слева и справа
#define line_ind_top 11 // оттуп линии сверху
#define back_pic_pos_x 0    // начало иконки предыдущий пункт меню x
#define back_pic_pos_y 4    // начало иконки предыдущий пункт меню y
#define size_back_pic_x 3   // размер треугольника по x
#define size_back_pic_y 3   // размер треугольника по y
#define top_pic_last_munu 1 // отступ сверху до названия предыдущего пункта меню
#define left_pic_last_munu 7 // отступ слева до названия предыдущего пункта меню
#define top_GSM_status 2    // отступ сверху до статуса связи
#define width_GSM_status 15  //  ширина одного значка статуса связи
#define top_akb_status 1    // отступ сверху до уровня заряда
#define width_akb_status 7  //  ширина одного уровня заряда


// RTC

#define width_symbol 6  //  ширина текста даты 01.01.2000



extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;

extern int Mode;
extern int GSM_mode;
extern int RS485_prot;
extern int units_mes;
extern int time_sleep_h;
extern int time_sleep_m;

char str[4];
int right_ot = winth_display-12-2;    // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа


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
    menuItem Id = {Name_rus, Name_en, Type_menu, 0, add_signat_ru, add_signat_en, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&data_in, (void *)&Data_out}


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

#define height_up_munu 15                                            // выста верхнего пункта меню
#define dist_y (int)((64 - height_up_munu) / (max_munu_in_page)) // расстояние между пунктами меню
#define pos_x_menu 4                                                // отступ от края для названий пунктов меню
#define pos_x_menu_data 100                                             // отступ от края для вывода значений

#define font my5x7fonts


/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    2 - Ввод времени                                        0x04
    1 - Ввод даты                                           0x02
    0 - по нажатию - действие                               0x01
*/




MAKE_MENU(Menu_1, "Режимы", "Modes",                    {0x40}, 0, "","", Menu_2,         NULL_ENTRY,     NULL_ENTRY, Menu_1_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_1, "Цикл", "Cycle",                {0x01}, 0, "","", Menu_1_2,       NULL_ENTRY,     Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_2, "Тест", "Test",                 {0x01}, 0, "","", Menu_1_3,       Menu_1_1,       Menu_1, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_1_3, "Показания", "Data",            {0x40}, 0, "","", NULL_ENTRY,     Menu_1_2,       Menu_1, Menu_1_3_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_1_3_1, "Тепература", "Data",     {0x08}, 0, "гр.","", Menu_1_3_2,     NULL_ENTRY,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "Глубина", "Data",        {0x08}, 0, "м.","", Menu_1_3_3,     Menu_1_3_1,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "Кор. глуб.", "Data",     {0x08}, 0, "м.","", NULL_ENTRY,     Menu_1_3_2,     Menu_1_3, NULL_ENTRY, NULL_ENTRY, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "Настройки", "Settings",              {0x40}, 0, "","", Menu_3,         Menu_1,         NULL_ENTRY, Menu_2_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_1, "Дата", "Modes",                {0x02}, 0, "","", Menu_2_2,       NULL_ENTRY,     Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_2, "Время", "Modes",               {0x04}, 0, "","", Menu_2_3,       Menu_2_1,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_3, "Время сна", "time sleep",      {0x20}, 0, "ч","h", Menu_2_4,       Menu_2_2,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_4, "Время сна", "time sleep",      {0x20}, 0, "м","m", Menu_2_5,       Menu_2_3,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_5, "Нул. ур.", "Modes",            {0x20}, 0, "м","", Menu_2_6,     Menu_2_4,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_6, "Уров. дат.", "Modes",          {0x20}, 0, "м","", Menu_2_7,       Menu_2_5,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_7, "Связь", "GSM",                 {0x10}, 0, "","", Menu_2_8,       Menu_2_6,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_8, "Ток. петля", "Modes",          {0x10}, 0, "","", Menu_2_9,       Menu_2_7,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_9, "RS485", "RS485",               {0x10}, 0, "","", Menu_2_10,       Menu_2_8,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_10, "Ед. изм.", "Modes",           {0x10}, 0, "","", Menu_2_11,      Menu_2_9,       Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_11, "Инжен. меню", "Modes",        {0x01}, 0, "","", Menu_2_12,      Menu_2_10,       Menu_2, Menu_2_11_1, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_1, "Авт. Калиб.", "Modes",  {0x01}, 0, "","", Menu_2_11_2,    NULL_ENTRY,     Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_2, "Ном. платы", "Modes",   {0x20}, 0, "","", Menu_2_11_3,    Menu_2_11_1,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_3, "Глубина", "Modes",      {0x08}, 0, "","", Menu_2_11_4,    Menu_2_11_2,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_4, "Ток", "Modes",          {0x08}, 0, "","", Menu_2_11_5,  Menu_2_11_3,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_5, "Темп. 1", "Modes",      {0x08}, 0, "","", Menu_2_11_6,    Menu_2_11_4,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_6, "Темп. 2", "Modes",      {0x08}, 0, "","", Menu_2_11_7,    Menu_2_11_5,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_7, "Темп. 3", "Modes",      {0x08}, 0, "","", Menu_2_11_8,    Menu_2_11_6,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_8, "Давление", "Modes",     {0x08}, 0, "","", Menu_2_11_9,    Menu_2_11_7,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
        MAKE_MENU(Menu_2_11_9, "Влажность", "Modes",    {0x08}, 0, "","", NULL_ENTRY,     Menu_2_11_8,    Menu_2_11, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_12, "Сброс", "Modes",              {0x01}, 0, "","", Menu_2_13,      Menu_2_11,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_2_13, "Формат. SD", "Modes",         {0x01}, 0, "","", NULL_ENTRY,     Menu_2_12,      Menu_2, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);
MAKE_MENU(Menu_3, "Сведения", "Info",                   {0x40}, 0, "","", Menu_4,         Menu_2,         NULL_ENTRY, Menu_3_1, NULL_ENTRY, NULL_ENTRY);
    MAKE_MENU(Menu_3_1, "ID устр.", "ID Device",        {0x08}, 0, "","", Menu_3_2,       NULL_ENTRY,     Menu_3, NULL_ENTRY, NULL_ENTRY, ID_board);    
    MAKE_MENU(Menu_3_2, "Вер. платы.", "Modes",         {0x08}, 0, "","", Menu_3_3,       Menu_3_1,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_board);
    MAKE_MENU(Menu_3_3, "Вер. ПО", "Modes",             {0x08}, 0, "","", NULL_ENTRY,     Menu_3_2,       Menu_3, NULL_ENTRY, NULL_ENTRY, ver_programm);
MAKE_MENU(Menu_4, "Инструкция", "Instruction",          {0x01}, 0, "","", NULL_ENTRY,     Menu_3,         NULL_ENTRY, NULL_ENTRY, NULL_ENTRY, NULL_ENTRY);

////////////////////////////////////////////////////
//                Обработка меню                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

void Display_all_menu()
{ // отображение всех пунктов меню на странице

        OLED_Clear(0);

        menuItem *menu_s = (menuItem *)(selectedMenuItem);
        FontSet(font);
        Display_TopBar(menu_s);

        Display_punkt_menu(menu_s, select_menu_in_page);
        Select_diplay_functions(menu_s, select_menu_in_page);

        int pos_cursor = select_menu_in_page * dist_y + height_up_munu+2;
        OLED_DrawTriangleFill(0, pos_cursor - 2, 0, pos_cursor + 2, 2, pos_cursor);

        

        for (int i = select_menu_in_page-1; i >= 0; i--)
        {
            menu_s = (menuItem *)(menu_s->Previous);
            Display_punkt_menu(menu_s, i);
            Select_diplay_functions(menu_s, i);
        }

        menu_s = (menuItem *)(selectedMenuItem);
        
        for (int i = select_menu_in_page+1; i < max_munu_in_page; i++)
        {
            menu_s = (menuItem *)(menu_s->Next);
            Display_punkt_menu(menu_s, i);
            Select_diplay_functions(menu_s, i);
        }



        OLED_UpdateScreen();
    
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
void Select_diplay_functions(menuItem *menu, int pos_y){
    if (menu->Type_menu == 0x04){
        Display_time(pos_y);
    }
    if (menu->Type_menu == 0x02){
        Display_data(pos_y);
    }    
}


char time_h[4];
char time_m[4];

void Display_data(int pos_y){

    snprintf(trans_str, 11, "%02d", Date.Date);
    OLED_DrawStr(trans_str, winth_display-width_symbol*9+2, pos_y*dist_y+height_up_munu, 1);
    OLED_DrawStr(".", winth_display-width_symbol*7+1, pos_y*dist_y+height_up_munu, 1);

    snprintf(trans_str, 11, "%02d", Date.Month);
    OLED_DrawStr(trans_str, winth_display-width_symbol*7+3, pos_y*dist_y+height_up_munu, 1);
    OLED_DrawStr(".", winth_display-width_symbol*5+2, pos_y*dist_y+height_up_munu, 1);

    snprintf(trans_str, 11, "20%02d", Date.Year);
    OLED_DrawStr(trans_str, winth_display-width_symbol*4-2, pos_y*dist_y+height_up_munu, 1);

}

void Display_time(int pos_y){
    snprintf(time_h, 4, "%02d", Time.Hours);
    
    OLED_DrawStr(time_h, winth_display-width_symbol*5+2, pos_y*dist_y+height_up_munu, 1);
    OLED_DrawStr(":", winth_display-width_symbol*3+2, pos_y*dist_y+height_up_munu, 1);
    
    snprintf(time_m, 4, "%02d", Time.Minutes);
    OLED_DrawStr(time_m, winth_display-width_symbol*3+4, pos_y*dist_y+height_up_munu, 1);

}




void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
{
    FontSet(font);
    if (len == 'r'){
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
        OLED_DrawStr(menu->add_signat_ru, winth_display-10, pos_y*dist_y+height_up_munu, 1);
    }
    else{
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y*dist_y+height_up_munu, 1);
        OLED_DrawStr(menu->add_signat_en,  winth_display-10, pos_y*dist_y+height_up_munu, 1);
    }

    if (menu->Type_menu & 0x08){
        OLED_DrawStr(menu->data_out,  winth_display-60, pos_y*dist_y+height_up_munu, 1);
    }
}

void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}



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
    right_ot = winth_display-12-2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа


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

void Keyboard_processing()
{   
    up_down();
    
    if (selectedMenuItem->Type_menu == 0x40)
        right();    //6 - вкладка                                       0x40
    if (selectedMenuItem->Type_menu == 0x20)
        return;    //5 - ввод числа                                          0x20
    if (selectedMenuItem->Type_menu == 0x10)
        return;    //4 - изменяемые пункты (трубуется ссылка на структуру)   0x10  
    if (selectedMenuItem->Type_menu == 0x08)
        left();    //3 - вывод незменяемого значвения char[]                 0x08
    if (selectedMenuItem->Type_menu == 0x04)
        return;    //2 - Ввод времени                                        0x04
    if (selectedMenuItem->Type_menu == 0x02)
        return;    //1 - Ввод даты                                           0x02  
    if (selectedMenuItem->Type_menu == 0x01)
        left();    //0 - по нажатию - действие                               0x01  

    Keyboard_press_code = 0xFF;
}

void up_down(){
    if (Keyboard_press_code != 0xFF)
    {
        if (Keyboard_press_code == 'D')
        {
            if (selectedMenuItem->Next != (void *)&NULL_ENTRY)
            {
                menuChange(selectedMenuItem->Next);
                if (select_menu_in_page + 1 < max_munu_in_page)
                    select_menu_in_page += 1;
            }
        }
        if (Keyboard_press_code == 'U')
        {
            if (selectedMenuItem->Previous != (void *)&NULL_ENTRY)
            {
                if (select_menu_in_page > 0)
                    select_menu_in_page -= 1;
                menuChange(selectedMenuItem->Previous);
            }
        }
    }
}
void left()
{
    if (Keyboard_press_code != 0xFF)
    {
        if (Keyboard_press_code == 'L')
        {
            if (selectedMenuItem->Parent != (void *)&NULL_ENTRY)
            {
                menuChange(selectedMenuItem->Parent);
                select_menu_in_page = selectedMenuItem->Num_menu;
            }
        }
    }
}

void right()
{
    if (Keyboard_press_code != 0xFF)
    {
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
    }
}
