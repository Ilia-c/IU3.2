#include "Display.h"
// #include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 2                            //  отступы верхней линии слева и справа
#define end_line winth_display - 2 * line_indentation //  отступы верхней линии слева и справа
#define line_ind_top 11                               // оттуп линии сверху
#define back_pic_pos_x 0                              // начало иконки предыдущий пункт меню x
#define back_pic_pos_y 4                              // начало иконки предыдущий пункт меню y
#define size_back_pic_x 3                             // размер треугольника по x
#define size_back_pic_y 3                             // размер треугольника по y
#define top_pic_last_munu 1                           // отступ сверху до названия предыдущего пункта меню
#define left_pic_last_munu 7                          // отступ слева до названия предыдущего пункта меню
#define top_GSM_status 2                              // отступ сверху до статуса связи
#define width_GSM_status 15                           //  ширина одного значка статуса связи
#define top_akb_status 1                              // отступ сверху до уровня заряда
#define width_akb_status 7                            //  ширина одного уровня заряда

#define time_led_cursor 500      // Время обновления индикации курсоора при вводе данных
#define time_updateDisplay 20000 // Время обновления экрана вне ввода данных

/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// Редактирование данных
int mode_redact = 0;  // 1 - режим редактирования данных, 0 - режим переключения страниц
int pos_redact = 0;   // позиция для редактирования
extern char c_Time[]; // из settings
extern char c_Date[]; // из settings
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
int right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа

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
#define ADD_SIGNAT_RU ""
#define ADD_SIGNAT_EN ""
#define NULL_ENTRY Null_Menu

#define MAKE_MENU(Id, Name_rus, Name_en, Num_menu, add_signat_ru, add_signat_en, Next, Previous, Parent, Child, action, select_bar, data_in, Data_out) \
    extern menuItem Next;                                                                                                                              \
    extern menuItem Previous;                                                                                                                          \
    extern menuItem Parent;                                                                                                                            \
    extern menuItem Child;                                                                                                                             \
    menuItem Id = {Name_rus, Name_en, 0, add_signat_ru, add_signat_en, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&action, (menuSelect_item *)&select_bar, (char *)&data_in, (char *)&Data_out}

// Выбираемые значения и статус
menuSelect_item GSM_MODE[2] = {&GSM_mode, {"Вкл.", "On"}, {"Выкл", "Off"}};          // Вкл откл GSM
menuSelect_item RS485_MODE[2] = {&RS485_prot, {"Выкл.", "Off"}, "Modbus", "Modbus"}; //
menuSelect_item UNITS_MODE[2] = {&units_mes, {"миллиметры", "millimeters"}, "метры", "meters"};
////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////
const int max_munu_in_page = 5; // максимальное количество пунктов меню на странице
int select_menu_in_page = 0;    // метущий пункт менюc
char len = 'r';                 //  r - русский язык;  e -  английский

#define height_up_menu 14                                            // выста верхнего пункта меню
#define dist_y (int)((64 - height_up_menu) / (max_munu_in_page)) + 2 // расстояние между пунктами меню
#define pos_x_menu 4                                                 // отступ от края для названий пунктов меню
#define pos_x_menu_data 100                                          // отступ от края для вывода значений

#define font my5x7fonts

/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод значения                                       0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
*/
char Time_char[1] = {0x01}; // нужны для записи времени, после окончания редактирования, вставляется в поле DATA_OUT
char Data_char[1] = {0x02}; // нужны для записи даты, после окончания редактирования, вставляется в поле DATA_OUT
extern char char_ADC_in_temp[];
extern char char_ADC_Height[];
extern char char_ADC_Height_correct[];
extern char ID_board[];
extern char ver_board[];
extern char ver_programm[];

MAKE_MENU(Menu_1, "Режимы", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2, PREVISION_MENU, PARENT_MENU, Menu_1_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_1, "Цикл", "Cycle", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_1_2, PREVISION_MENU, Menu_1, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_2, "Тест", "Test", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_1_3, Menu_1_1, Menu_1, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_3, "Показания", "Data", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_1_2, Menu_1, Menu_1_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_3_1, "Тепература", "Data", 0, "°", "°", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, char_ADC_in_temp);
        MAKE_MENU(Menu_1_3_2, "Глубина", "Data", 0, "м.", ADD_SIGNAT_EN, Menu_1_3_3, Menu_1_3_1, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, char_ADC_Height);
        MAKE_MENU(Menu_1_3_3, "Кор. глуб.", "Data", 0, "м.", ADD_SIGNAT_EN, NEXT_MENU, Menu_1_3_2, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, char_ADC_Height_correct);
MAKE_MENU(Menu_2, "Настройки", "Settings", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3, Menu_1, PARENT_MENU, Menu_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_1, "Дата", "Data", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_2, PREVISION_MENU, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, c_Date, Time_char);
    MAKE_MENU(Menu_2_2, "Время", "Time", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_3, Menu_2_1, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, c_Time, Data_char);
    MAKE_MENU(Menu_2_3, "Время сна", "time sleep", 0, "ч", "h", Menu_2_4, Menu_2_2, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, c_time_sleep_h, DATA_OUT);
    MAKE_MENU(Menu_2_4, "Время сна", "time sleep", 0, "м", "m", Menu_2_5, Menu_2_3, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, c_time_sleep_m, DATA_OUT);
    MAKE_MENU(Menu_2_5, "Нул. ур.", "Modes", 0, "м", "m", Menu_2_6, Menu_2_4, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_6, "Уров. дат.", "Modes", 0, "м", "m", Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_7, "Связь", "GSM", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_8, Menu_2_6, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_8, "Ток. петля", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_9, Menu_2_7, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_9, "RS485", "RS485", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10, Menu_2_8, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_10, "Ед. изм.", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11, Menu_2_9, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_11, "Инжен. меню", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_12, Menu_2_10, Menu_2, Menu_2_11_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_1, "Авт. Калиб.", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_2, PREVISION_MENU, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_2, "Ном. платы", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_3, Menu_2_11_1, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_3, "Глубина", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_4, Menu_2_11_2, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_4, "Ток", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_5, Menu_2_11_3, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_5, "Темп. 1", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_6, Menu_2_11_4, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_6, "Темп. 2", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_7, Menu_2_11_5, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_7, "Темп. 3", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_8, Menu_2_11_6, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_8, "Давление", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11_9, Menu_2_11_7, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_11_9, "Влажность", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_2_11_8, Menu_2_11, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_12, "Сброс", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_13, Menu_2_11, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_13, "Формат. SD", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_2_12, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_3, "Сведения", "Info", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_4, Menu_2, PARENT_MENU, Menu_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_3_1, "ID устр.", "ID Device", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3_2, PREVISION_MENU, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ID_board);
    MAKE_MENU(Menu_3_2, "Вер. платы.", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3_3, Menu_3_1, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ver_board);
    MAKE_MENU(Menu_3_3, "Вер. ПО", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_3_2, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ver_programm);
MAKE_MENU(Menu_4, "Инструкция", "Instruction", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_3, PARENT_MENU, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);

////////////////////////////////////////////////////
//                Обработка меню                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

/// Возвращает длину строки без учета точек и двоеточий
int search_len_mass(char *string)
{
    int counter = 0; //
    for (int i = 0; i < 11; i++)
    {
        if ((string[i] != '.') && (string[i] != ':'))
            counter++;
        if (string[i] == 0)
        {
            return counter - 1;
        }
    }
    return -1;
}

/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
*/
void Select_diplay_functions(menuItem *menu, int pos_y)
{
    int len_end_symbol = 0;

    if (len == 'r')
    {
        OLED_DrawStr(menu->add_signat_ru, winth_display - 10, pos_y * dist_y + height_up_menu, 1);
        len_end_symbol = OLED_GetWidthStr(menu->add_signat_ru) + 2;
    }
    else
    {
        OLED_DrawStr(menu->add_signat_en, winth_display - 10, pos_y * dist_y + height_up_menu, 1);
        len_end_symbol = OLED_GetWidthStr(menu->add_signat_en) + 2;
    }

    if (menu->data_in != (void *)&NULL_ENTRY)
    {
        int counter = OLED_GetWidthStr(menu->data_in) + 2 + len_end_symbol;
        OLED_DrawStr(menu->data_in, winth_display - counter, pos_y * dist_y + height_up_menu, 1);

        /// индикация ввода
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            //  Нужно найти ширину символа. pos_redact - текущий символ
            // int len = search_len_mass(selectedMenuItem->data_in); // длина массива

            char string_pos_line[10] = {};
            int pos_x_line = 0;
            int i = 0;
            for (; ((pos_x_line < pos_redact + 1) || (i > 10)); i++)
            {
                string_pos_line[i] = selectedMenuItem->data_in[i];
                if ((selectedMenuItem->data_in[i] != '.') && (selectedMenuItem->data_in[i] != ':'))
                {
                    pos_x_line++;
                }
            }
            int len_to_char = OLED_GetWidthStr(string_pos_line);
            char array[2] = {};
            array[0] = selectedMenuItem->data_in[i - 1];
            int len_select_char = OLED_GetWidthStr(array) - 1;
            OLED_DrawHLine(winth_display - counter + len_to_char - len_select_char - 1, select_menu_in_page * dist_y + height_up_menu + 8, len_select_char, led_cursor = !led_cursor);
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
{
    FontSet(font);
    if (len == 'r')
    {
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }
    else
    {
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }

    if (selectedMenuItem->data_out != (void *)&NULL_ENTRY)
    {
        OLED_DrawStr(menu->data_out, winth_display - 60, pos_y * dist_y + height_up_menu, 1);
    }
}
void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}

extern SPI_HandleTypeDef hspi1;

void Display_TopBar(menuItem *CurrentMenu)
{

    OLED_DrawHLine(line_indentation, line_ind_top, end_line, 1);
    if ((void *)CurrentMenu->Parent != (void *)&NULL_ENTRY)
    {
        OLED_DrawTriangleFill(back_pic_pos_x, back_pic_pos_y, back_pic_pos_x + size_back_pic_x, back_pic_pos_y + size_back_pic_y, back_pic_pos_x + size_back_pic_x, back_pic_pos_y - size_back_pic_y);
        OLED_DrawPixel(back_pic_pos_x + 1, back_pic_pos_y);
        OLED_DrawStr(CurrentMenu->Parent, left_pic_last_munu, top_pic_last_munu, 1);
    }

/*
    W25_Ini();
    unsigned int id = W25_Read_ID();
    sprintf(str, "ID Value: 0x%X\n", id);
    OLED_DrawStr(str, 1, 1, 1);
*/


    sprintf(str, "%d", ADC_AKB_Proc);
    if (ADC_AKB_Proc < 10)
        str[1] = '%';
    else if (ADC_AKB_Proc < 100)
    {
        str[2] = '%';
        right_ot -= 6;
    }
    else
    {
        str[3] = '%';
        right_ot -= 12;
    }


    OLED_DrawStr(str, right_ot, top_akb_status + 1, 1);
    right_ot -= width_akb_status;

    int c = ADC_AKB_Proc * 5 / 100 + 1;
    if (ADC_AKB_Proc == 0)
        c = 0;
    // OLED_DrawRectangle(right_ot+2, top_akb_status+7-c, right_ot+3, top_akb_status+2+c);
    for (c; c > 0; c--)
    {
        OLED_DrawHLine(right_ot + 2, top_akb_status + 8 - c, 2, 1);
    }
    OLED_DrawXBM(right_ot, top_akb_status, akb);
    right_ot -= width_GSM_status;

    if (GSM_Signal_Level < 0)
    {
        right_ot += 3;
        OLED_DrawXBM(right_ot, top_GSM_status, no_signal);
    }
    if (GSM_Signal_Level == 0)
    {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_0);
    }
    if (GSM_Signal_Level == 1)
    {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_1);
    }
    if (GSM_Signal_Level == 2)
    {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_2);
    }
    if (GSM_Signal_Level == 3)
    {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_3);
    }
    right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа
}

void Display_all_menu()
{ // отображение всех пунктов меню на странице

    OLED_Clear(0);

    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);

    Display_punkt_menu(menu_s, select_menu_in_page);
    Select_diplay_functions(menu_s, select_menu_in_page);

    int pos_cursor = select_menu_in_page * dist_y + height_up_menu + 2;
    OLED_DrawTriangleFill(0, pos_cursor - 1, 0, pos_cursor + 3, 2, pos_cursor+1);

    for (int i = select_menu_in_page - 1; i >= 0; i--)
    {
        if (menu_s->Previous ==  (void *)&NULL_ENTRY){
            break;
        }
        menu_s = (menuItem *)(menu_s->Previous);
        Display_punkt_menu(menu_s, i);
        Select_diplay_functions(menu_s, i);
    }

    menu_s = (menuItem *)(selectedMenuItem);

    for (int i = select_menu_in_page + 1; i < max_munu_in_page; i++)
    {
        if (menu_s->Next ==  (void *)&NULL_ENTRY){
            break;
        }
        menu_s = (menuItem *)(menu_s->Next);
        Display_punkt_menu(menu_s, i);
        Select_diplay_functions(menu_s, i);
    }
    OLED_UpdateScreen();
}

// вход в режим редактирования, если есть соответствующее поле
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        // ввод значений
        mode_redact = 1;
        pos_redact = 0;
        time_update_display = time_led_cursor;
        xSemaphoreGive(Display_semaphore);
    }
}
void redact_end()
{
    if ((mode_redact == 1) && (selectedMenuItem->data_in == c_Date))
    {
        RTC_set_date();
    }
    if ((mode_redact == 1) && (selectedMenuItem->data_in == c_Time))
    {
        RTC_set_time();
    }

    pos_redact = len - 1;
    mode_redact = 0;
    led_cursor = 1;
    time_update_display = time_updateDisplay;
}

char searc_pos_in_string(char *string, int position)
{
    int counter = 0;
    for (int i = 0; i < 10; i++)
    {
        if (position == counter)
        {
            return string[i];
        }
        if ((string[i + 1] != '.') && (string[i + 1] != ':'))
            counter++;
    }
    return -1;
}

void up_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        // действия при вводе. Нажатие вверх
        char data = searc_pos_in_string(selectedMenuItem->data_in, pos_redact);
        if (data == '9')
        {
            if (pos_redact - 1 >= 0)
            {
                char data_2 = searc_pos_in_string(selectedMenuItem->data_in, pos_redact - 1);
                if (data_2 + 1 > '9') data_2 = '0';
                else data_2++;
                data_redact_pos(pos_redact - 1, data_2);
            }
            data = '0';
        }
        else
            data++;
        data_redact_pos(pos_redact, data);
        led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
    }
}
void up()
{
    if (!(mode_redact == 0))
    {
        up_redact();
        return;
    }
    if (selectedMenuItem->Previous != (void *)&NULL_ENTRY)
    {
        if (select_menu_in_page > 0)
            select_menu_in_page -= 1;
        menuChange(selectedMenuItem->Previous);
    }
}

void down_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        // действия при вводе. Нажатие вниз
        char data = searc_pos_in_string(selectedMenuItem->data_in, pos_redact);
        if (data == '0')
        {
            data = '9';
            if (pos_redact - 1 >= 0)
            {
                char data_2 = searc_pos_in_string(selectedMenuItem->data_in, pos_redact - 1);
                if (data_2 - 1 < '0')
                    data_2 = '9';
                else
                    data_2--;
                data_redact_pos(pos_redact - 1, data_2);
            }
        }
        else
            data--;
        data_redact_pos(pos_redact, data);
        led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
    }
}
void down()
{
    if (!(mode_redact == 0))
    {
        down_redact();
        return;
    }
    if (selectedMenuItem->Next != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Next);
        if (select_menu_in_page + 1 < max_munu_in_page)
            select_menu_in_page += 1;
    }
}

void left_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        pos_redact--;
        led_cursor = 0;
        if (pos_redact < 0)
        {
            mode_redact = 0;
            led_cursor = 1;
            time_update_display = time_updateDisplay;
        }
        // действия при вводе. Нажатие вправо
    }

    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY)
    {
        // действие при прокрутке
    }
}
void left()
{
    if (!(mode_redact == 0))
    {
        left_redact();
        return;
    }
    if (selectedMenuItem->Parent != (void *)&NULL_ENTRY)
    {
        menuChange(selectedMenuItem->Parent);
        select_menu_in_page = selectedMenuItem->Num_menu;
    }
}

void right_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        int len = search_len_mass(selectedMenuItem->data_in);
        pos_redact++;
        led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
        if (pos_redact >= len)
        {
            redact_end();
        }

        // действия при вводе. Нажатие вправо
    }

    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY)
    {
        // действие при прокрутке
    }
}

void right()
{
    if (mode_redact == 1)
    {
        right_redact();
        return;
    }

    if (selectedMenuItem->action != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->action(); //  ссылка на функцию при действии
        return;
    }
    mode_check(); // выставление режима

    if (selectedMenuItem->Child != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->Num_menu = select_menu_in_page;
        select_menu_in_page = 0;
        menuChange(selectedMenuItem->Child);
    }
}
void ok()
{
    if (mode_redact == 1)
    {
        redact_end();
        return;
    }
    mode_check();
}
void null_fun()
{
}

/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод значения                                       0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
*/

// редактирование позиции с учетом  :  и .
void data_redact_pos(int position, char data)
{
    int counter_pos = 0;
    for (int i = 0; i < 10; i++)
    {
        char Test = selectedMenuItem->data_in[i];
        if (counter_pos == position)
        {
            selectedMenuItem->data_in[i] = data;
            led_cursor = 0;
            return;
        }
        if ((selectedMenuItem->data_in[i + 1] != '.') && (selectedMenuItem->data_in[i + 1] != ':'))
            counter_pos++;
    }
}

void key_press_data_write(char data)
{
    int len = search_len_mass(selectedMenuItem->data_in);
    data_redact_pos(pos_redact, data);
    pos_redact++;
    led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
    if (pos_redact >= len)
        pos_redact = len - 1;
}

// события по нажатию кнопки на клавиатуре
void Keyboard_processing()
{
    if (Keyboard_press_code != 0xFF)
    {
        // Стрелки
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

            // Левая часть панели

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