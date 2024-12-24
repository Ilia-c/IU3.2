#include "Display.h"
// #include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 2                            //  ������� ������� ����� ����� � ������
#define end_line winth_display - 2 * line_indentation //  ������� ������� ����� ����� � ������
#define line_ind_top 11                               // ����� ����� ������
#define back_pic_pos_x 0                              // ������ ������ ���������� ����� ���� x
#define back_pic_pos_y 4                              // ������ ������ ���������� ����� ���� y
#define size_back_pic_x 3                             // ������ ������������ �� x
#define size_back_pic_y 3                             // ������ ������������ �� y
#define top_pic_last_munu 1                           // ������ ������ �� �������� ����������� ������ ����
#define left_pic_last_munu 7                          // ������ ����� �� �������� ����������� ������ ����
#define top_GSM_status 2                              // ������ ������ �� ������� �����
#define width_GSM_status 15                           //  ������ ������ ������ ������� �����
#define top_akb_status 1                              // ������ ������ �� ������ ������
#define width_akb_status 7                            //  ������ ������ ������ ������

#define time_led_cursor 500      // ����� ���������� ��������� �������� ��� ����� ������
#define time_updateDisplay 20000 // ����� ���������� ������ ��� ����� ������

/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// �������������� ������
int mode_redact = 0;  // 1 - ����� �������������� ������, 0 - ����� ������������ �������
int pos_redact = 0;   // ������� ��� ��������������


int led_cursor = 1;
extern int time_update_display;
extern xSemaphoreHandle Display_semaphore;
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;

extern int Mode;
extern int Communication;
extern int RS485_prot;
extern int units_mes;
extern int screen_sever_mode;


char str[4];
int right_ot = winth_display - 12 - 2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������

char trans_str[11];

extern char Keyboard_press_code;
extern int GSM_Signal_Level;
extern int ADC_AKB_Proc;


const int max_munu_in_page = 5; // ������������ ���������� ������� ���� �� ��������
int select_menu_in_page = 0;    // ������� ����� ����c
extern char len;                //  0 - ������� ����;  1 -  ����������
uint8_t Intermediate = 0;           // ������������� ����������, ���� ����������� ��������� �� ����������



#define height_up_menu 14                                            // ����� �������� ������ ����
#define dist_y (int)((64 - height_up_menu) / (max_munu_in_page)) + 2 // ���������� ����� �������� ����
#define pos_x_menu 4                                                 // ������ �� ���� ��� �������� ������� ����
#define pos_x_menu_data 100                                          // ������ �� ���� ��� ������ ��������

#define font my5x7fonts

/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� ��������                                       0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    0 - �� ������� - ��������                               0x01
*/

extern char ver_board[];
extern char VERSION_PROGRAMM[];
extern char time_work_char[];

menuItem Null_Menu = {{0}, {0}, 0, {0}, {0}, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
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

// ���������� �������� � ������
menuSelect_item Communication_DATA = { 
    (uint8_t *)&Communication,
    {
        {"GSM/NB-IoT", "GSM/NB-IoT"},
        {"����", "Off"}
    }
}; 
menuSelect_item RS485_MODE_DATA = {
    (uint8_t *)&RS485_prot,
    {
        {"����.", "Off"},
        {"Modbus", "Modbus"}
    }
};
menuSelect_item UNITS_MODE_DATA = {
    (uint8_t *)&units_mes,
    {
        {"��", "mm"},
        {"�", "m"}
    }
};
menuSelect_item SCREENSAVER = {
    (uint8_t *)&screen_sever_mode ,
    {
        {"���.", "on"},
        {"����.", "off"}
    }
};

menuSelect_item LANGUAGE = {
    (uint8_t *)&len,
    {
        {"�������", "�������"},
        {"English", "English"}
    }
};

extern int Current_mode;
menuSelect_item CURRENT_LOOP = {
    (uint8_t *)&Current_mode,
    {
        {"4-20��", "4-20mA"},
        {"0-20��", "0-20mA"},
        {"����.", "off"}
    }
};

// �������������� ��� uint8_t
void format_uint8_t(char *buffer, size_t size, void *data) {
    snprintf(buffer, size, "%u", *(uint8_t *)data);
}
// �������������� ��� uint16_t
void format_uint16_t(char *buffer, size_t size, void *data) {
    snprintf(buffer, size, "%u", *(uint16_t *)data);
}
// �������������� ��� int32_t
void format_int32_t(char *buffer, size_t size, void *data) {
    snprintf(buffer, size, "%ld", *(int32_t *)data);
}
void format_char(char *buffer __attribute__((unused)), size_t size __attribute__((unused)), void *data __attribute__((unused))) {
    return;
}

DataFormatter formatters[] = {
    format_uint8_t,
    format_uint16_t,
    format_int32_t,
    format_char
};
void Null_func(){}

// ���������� ���������

// �������������� ���� RTC
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
menuSelect_item_char Date_redact = {
    {'.', '.', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    3,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&Date.Date, &Date.Month, &Date.Year},  // �������� ��������
    {0, 0, 0},                  // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                  // �������� ������ ��� �����������
    {2, 2, 2},                  // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                  // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},         // ������������� ��������
    {31, 12, 99},               // ������������ ��������
    {0, 0, 0},                  // ����������� ��������
    Save_date_format                   // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// �������������� ������� RTC
menuSelect_item_char Time_redact = {
    {':', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&Time.Hours, &Time.Minutes, '\0'},  // �������� ��������
    {0, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                   // �������� ������ ��� �����������
    {2, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {23, 59, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Save_time_format                    // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// ����� ���, ����������
extern uint16_t time_sleep_h;
extern uint16_t time_sleep_m;
menuSelect_item_char Time_sleep_redact = {
    {'�', '�', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&time_sleep_h, &time_sleep_m, '\0'},  // �������� ��������
    {1, 1, 1},                  // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                  // �������� ������ ��� �����������
    {3, 2, 0},                  // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {1, 1, 1},                  // ������ ����� ��� �������������� (�����������) (001 - ������ 3, 23 - ������ 2) 
    {"\0", "\0", "\0"},         // ������������� ��������
    {999, 59, '\0'},            // ������������ ��������
    {0, 5, '\0'},               // ����������� ��������
    Save_general_format         // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)


// ������� �������
menuSelect_item_char Zero_level = {
    {'\0', '\0', '\0'},     // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    1,                      // ����������� ����� ������
    0,                      // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&Time.Hours, &Time.Minutes, '\0'},  // �������� ��������
    {2, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                   // �������� ������ ��� �����������
    {2, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {23, 59, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Save_time_format                    // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)



// ������������ ������� ���������� �������
menuSelect_item_char Max_Level_Mesurment = {
    {'\0', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    1,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&Time.Hours, &Time.Minutes, '\0'},  // �������� ��������
    {2, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                   // �������� ������ ��� �����������
    {2, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {23, 59, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Null_func                    // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// ����������� ������� ���������� �������
menuSelect_item_char Min_Level_Mesurment = {
    {'\0', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    1,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&Time.Hours, &Time.Minutes, '\0'},  // �������� ��������
    {2, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                   // �������� ������ ��� �����������
    {2, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {23, 59, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Null_func                    // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)


extern uint8_t VER_PCB_IDEOLOGY = 0;
extern uint8_t VER_PCB_VERSION = 0;
extern char VER_PCB_INDEX[3];
menuSelect_item_char Serial_number = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    3,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    //  ������
    {&VER_PCB_IDEOLOGY, &VER_PCB_VERSION, &VER_PCB_INDEX},  // �������� ��������
    {0, 0, 3},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0, 0},                   // �������� ������ ��� �����������
    {2, 2, 3},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 0},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99, 99, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Null_func                    // ������ �� ������� ���������� ������
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

////////////////////////////////////////////////////
//                 ������� ����                   //
////////////////////////////////////////////////////




////////////////////////////////////////////////////
//                  ������ ����                   //
////////////////////////////////////////////////////

extern char data_add_unit[3];
extern char error_code[4];
extern char ADC_AKB_volts[4];
extern char ADC_AKB_Proc[4];
extern char ADC_in_temp_char[5];      // ���������� ����������� ����������������
extern char ADC_ADS1115_temp_char[5]; // ����������� �� ���������� ������� �����������
extern char OneWire_temp_char[5];     // ����������� �� ������� OneWire

extern char ADC_value_char[11];         // �������� ���
extern char ADC_status_char[3];         // ������ ���, �������� �� (ERR, WAR, OK)
extern char ADC_Volts_char[5];          // ���������� �� ������� �����
extern char ADC_Current_char[5];        // ��� �� ������� �����
extern char ADC_Height_char[5];         // ������� � ������ ��� ���������
extern char ADC_Height_correct_char[5]; // ������� � ������ � ��������������

extern char GSM_status_char[4];       // ������ GSM
extern char GSM_SIMCARD_char[4];      // ����� �� GSM SIM?
extern char GSM_status_ready_char[4]; // ����� �� GSM?
extern char GSM_status_reg_char[4];   // ����������������� �� GSM � ����
extern char GSM_operator_char[10];    // �������� ��������� MTS, bilene � �.�.
extern char GSM_signal_lvl_char[3];   // ������� ������� 0-99
extern char GSM_gprs_on_char[3];      // ������� �� gprs?

extern char EEPROM_status_char[3]; // ������ ����������� EEPROM
extern char FLASH_status_char[3];  // ������ ����������� FLASH
extern char SD_status_char[3];     // ������ ����������� SD

// ������ ���� ��� ������� ������
void full_test(){}

// ���� �������� ���
void GSM_sms_test(){}

// ���� 
void GSM_internet_test(){}



MAKE_MENU(Menu_1, "������", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2, PREVISION_MENU, PARENT_MENU, Menu_1_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_1, "����", "Cycle", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_1_2, PREVISION_MENU, Menu_1, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_1_2, "����", "Test", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_1_3, Menu_1_1, Menu_1, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_2_1, "��� ������", "Error code", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, error_code);
        MAKE_MENU(Menu_1_2_1, "������ ����", "Full test", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, full_test, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_2_1, "���", "BAT", 0, "�", "v", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_AKB_volts);
        MAKE_MENU(Menu_1_2_1, "���", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_status_char);
            MAKE_MENU(Menu_1_2_1_1, "���", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_value_char);
            MAKE_MENU(Menu_1_2_1_1, "���������", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_Height_char);
            MAKE_MENU(Menu_1_2_1_1, "���. ����", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_Height_correct_char);
            MAKE_MENU(Menu_1_2_1_1, "���������� �.", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_Volts_char);
            MAKE_MENU(Menu_1_2_1_1, "���", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_Current_char);
        MAKE_MENU(Menu_1_2_1, "GSM", "GSM", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_status_char);
            MAKE_MENU(Menu_1_2_1_1, "���� ���", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, GSM_sms_test, SELECT_BAR, DATA_IN, DATA_OUT);
            MAKE_MENU(Menu_1_2_1_1, "���� ����", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, GSM_internet_test, SELECT_BAR, DATA_IN, DATA_OUT);
            MAKE_MENU(Menu_1_2_1_1, "��� �����", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_SIMCARD_char);
            MAKE_MENU(Menu_1_2_1_1, "����������", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_status_ready_char);
            MAKE_MENU(Menu_1_2_1_1, "�����������", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_status_reg_char);
            MAKE_MENU(Menu_1_2_1_1, "��������", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_operator_char);
            MAKE_MENU(Menu_1_2_1_1, "������� ����.", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_signal_lvl_char);
            MAKE_MENU(Menu_1_2_1_1, "������� GPRS?", "ADC", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, GSM_gprs_on_char);
        MAKE_MENU(Menu_1_2_1, "EEPROM", "EEPROM", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM_status_char);
        MAKE_MENU(Menu_1_2_1, "SD", "SD", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, SD_status_char);
        MAKE_MENU(Menu_1_2_1, "FLASH", "FLASH", 0, "", "", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, FLASH_status_char);
    MAKE_MENU(Menu_1_3, "���������", "Data", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_1_2, Menu_1, Menu_1_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_3_1, "����������", "Data", 0, "�", "�", Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_3_2, "�������", "Data", 0, "�.", ADD_SIGNAT_EN, Menu_1_3_3, Menu_1_3_1, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_1_3_3, "���. ����.", "Data", 0, "�.", ADD_SIGNAT_EN, NEXT_MENU, Menu_1_3_2, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_2, "���������", "Settings", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3, Menu_1, PARENT_MENU, Menu_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_1, "����", "Data", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_2, PREVISION_MENU, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Date_redact, DATA_OUT);
    MAKE_MENU(Menu_2_2, "�����", "Time", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_3, Menu_2_1, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_redact, DATA_OUT);
    MAKE_MENU(Menu_2_3, "����� ���", "time sleep", 0, "�", "h", Menu_2_4, Menu_2_2, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_sleep_redact, DATA_OUT);
    MAKE_MENU(Menu_2_4, "���. ��.", "Modes", 0, "�", "m", Menu_2_5, Menu_2_3, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_5, "���", "Modes", 0, "�", "m", Menu_2_6, Menu_2_4, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Max_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_5, "���", "Modes", 0, "�", "m", Menu_2_6, Menu_2_4, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Min_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_6, "�����", "GSM", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, Communication_DATA, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_6, "����. �� ����", "GSM", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_6, "������ �� USB", "GSM", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_7, "���. �����", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_8, Menu_2_6, Menu_2, CHILD_MENU, ACTION_MENU, CURRENT_LOOP, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_8, "RS485", "RS485", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_9, Menu_2_7, Menu_2, CHILD_MENU, ACTION_MENU, RS485_MODE_DATA, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_9, "��. ���.", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10, Menu_2_8, Menu_2, CHILD_MENU, ACTION_MENU, UNITS_MODE_DATA, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_10, "�����. ����", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_11, Menu_2_9, Menu_2, Menu_2_10_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_10, "��������", "Wallpaper", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_2_10_9, Menu_2_10, CHILD_MENU, ACTION_MENU, SCREENSAVER, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_1, "�������� �����", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_2, PREVISION_MENU, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, Serial_number, DATA_OUT);
        MAKE_MENU(Menu_2_10_1, "���������� ��", "GSM", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, !, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_2, "������� �� USB", "Num. of PCB", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_3, Menu_2_10_1, Menu_2_10, CHILD_MENU, ACTION_MENU, !, DATA_IN, !);
        MAKE_MENU(Menu_2_10_2, "���������� ����", "Num. of PCB", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_3, Menu_2_10_1, Menu_2_10, CHILD_MENU, !, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_3, "���������� ���", "Depth", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_4, Menu_2_10_2, Menu_2_10, CHILD_MENU, !, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_4, "�������������", "Current", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_5, Menu_2_10_3, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_5, "����. ����", "Temp. 1", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_6, Menu_2_10_4, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, !, DATA_OUT);
        MAKE_MENU(Menu_2_10_6, "��������", "Temp. 2", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_7, Menu_2_10_5, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, !, DATA_OUT);
        MAKE_MENU(Menu_2_10_7, "����. ��", "Temp. 3", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_8, Menu_2_10_6, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_8, "����. ����.", "Pressure", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_9, Menu_2_10_7, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_9, "����. ����.", "Humidity", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_10, Menu_2_10_8, Menu_2_10, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, !);
        MAKE_MENU(Menu_2_10_9, "���� Flash", "Humidity", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_10, Menu_2_10_8, Menu_2_10, CHILD_MENU, !, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_9, "���� EEPROM", "Humidity", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_10, Menu_2_10_8, Menu_2_10, CHILD_MENU, !, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_9, "���� SD", "Humidity", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_10, Menu_2_10_8, Menu_2_10, CHILD_MENU, !, SELECT_BAR, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_10_9, "����������", "Humidity", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_10_10, Menu_2_10_8, Menu_2_10, CHILD_MENU, ACTION_MENU, !, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_11, "����", "Language", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_12, Menu_2_10, Menu_2, CHILD_MENU, ACTION_MENU, LANGUAGE, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_12, "�����", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_2_13, Menu_2_11, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_13, "������. SD", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_2_12, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_3, "��������", "Info", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_4, Menu_2, PARENT_MENU, Menu_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_3_1, "ID ����.", "ID Device", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3_2, PREVISION_MENU, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ver_board);
    MAKE_MENU(Menu_3_2, "���. ��", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, Menu_3_3, Menu_3_1, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, VERSION_PROGRAMM);
    MAKE_MENU(Menu_3_3, "����� ������", "Modes", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_3_2, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, time_work_char);
MAKE_MENU(Menu_4, "����������", "Instruction", 0, ADD_SIGNAT_RU, ADD_SIGNAT_EN, NEXT_MENU, Menu_3, PARENT_MENU, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

/// ���������� ����� ������ ��� ����� ����� � ���������
int search_len_mass(char *string, int len_str, char separotor[])
{
    int counter = 0; 
    for (int i = 0; i < len_str; i++)
    {
        if (string[i] == '\0') return counter;
        if ((string[i] != separotor[0]) && (string[i] != separotor[1]) && (string[i] != separotor[2]))
            counter++;
    }
    return -1;
}



void Data_in_no_redact(menuItem *menu, int pos_y){
    char string[10] = {'\0'};
    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        char buffer[11] = {'\0'};
        char separat[11] = {'\0'}; // ������ ������������, � ������ ������� �����, ������������ ����
        formatters[menu->data_in->data_type[i]](buffer, sizeof(buffer), menu->data_in->data[i]);
        // �������� ���� ����� ��������
        if (menu->data_in->data_type[i] != 3)
        {
            uint8_t len_dat = search_len_mass(buffer, 11, separat);
            for (int j = 0; j < (uint8_t)menu->data_in->len_data_zero_unredact[i] - len_dat; j++)
                separat[j] = '0';
            strcat(string, separat);
        }
        if (menu->data_in->data_type[i] == 3)
            strcat(string, menu->data_in->data[i]);
        strcat(string, buffer);
        char temp[2] = {menu->data_in->separators[i], '\0'};
        strcat(string, temp);
    }
    uint8_t len_string = OLED_GetWidthStr(string) + 4;
    OLED_DrawStr(string, winth_display - len_string, pos_y * dist_y + height_up_menu, 1);
}


void Data_in_redact(menuItem *menu, int pos_y){
    
    char string[11] = {'\0'};
    uint8_t separators_count = 0;

    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        menu->data_in->data_temp[i][menu->data_in->len_data_zero[i]] = '\0'; // �� ������ ������
        strcat(string, menu->data_in->data_temp[i]);
        char temp[2] = {menu->data_in->separators[i], '\0'};
        if (menu->data_in->separators[i] != '\0') separators_count++;
        strcat(string, temp);
    }
    uint8_t len_string = OLED_GetWidthStr(string) + 4;
    OLED_DrawStr(string, winth_display - len_string, pos_y * dist_y + height_up_menu, 1);
    
    
    uint8_t len_witout_separator = menu->data_in->len_data_zero[0] + menu->data_in->len_data_zero[1] + menu->data_in->len_data_zero[2];
    int8_t add_pos = 0;
    int8_t position = pos_redact+1;
    for (; add_pos<separators_count; add_pos++){
        position -= menu->data_in->len_data_zero[add_pos];
        if (position<=0) break;
    }
    char string_temp[11] = {'\0'};
    strncpy(string_temp, string, pos_redact+add_pos);
    uint8_t start_pos = OLED_GetWidthStr(string_temp);
    OLED_DrawHLine(winth_display - len_string + start_pos, pos_y * dist_y + 8 + height_up_menu, 5, led_cursor=!led_cursor);

}

/*  ��� ���� (char)0b543210 �������������� �������� 
    6 - �������                                             0x40
    5 - ���� �����                                          0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    0 - �� ������� - ��������                               0x01
*/
void Select_diplay_functions(menuItem *menu, int pos_y)
{
    //int len_end_symbol = 0;
    int leng_font = 0;
    if (len == 0x00) OLED_DrawStr(menu->add_signat_ru, winth_display - 10, pos_y * dist_y + height_up_menu, 1);
    else 
    {
        leng_font = 1;
        OLED_DrawStr(menu->add_signat_en, winth_display - 10, pos_y * dist_y + height_up_menu, 1);
    }
    
    // ����� ������ (���������)
    if (menu->select_bar != (void *)&NULL_ENTRY)
    {   
        uint8_t num_menu = *menu->select_bar->data;
        if ((selectedMenuItem == menu) && (mode_redact == 1)) num_menu = Intermediate;

        int len_dat = OLED_GetWidthStr(menu->select_bar->Name[num_menu][leng_font]);
        
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            OLED_DrawStr(menu->select_bar->Name[num_menu][leng_font], winth_display - len_dat - 8, pos_y * dist_y + height_up_menu, 1);  // ����� ���� ������� ����� (��������������)
            int pos_cursor = select_menu_in_page * dist_y + height_up_menu + 2;
            int x_left = winth_display-len_dat-11;
            int x_right = winth_display-7;
            if (menu->select_bar->Name[Intermediate+1][0][0] != '\0') OLED_DrawTriangleFill(x_right, pos_cursor - 1, x_right, pos_cursor + 3, x_right+2, pos_cursor+1);
            if (Intermediate != 0) OLED_DrawTriangleFill(x_left, pos_cursor - 1, x_left, pos_cursor + 3, x_left-2, pos_cursor+1);
        }
        else{
            OLED_DrawStr(menu->select_bar->Name[num_menu][leng_font], winth_display - len_dat - 4, pos_y * dist_y + height_up_menu, 1); // ����� ���� ��� ������ 
        }
    }


    if (menu->data_in != (void *)&NULL_ENTRY)
    {
        // ����� �� ��������������
        if ((mode_redact == 0) || (selectedMenuItem != menu)){
            Data_in_no_redact(menu, pos_y);
        }

        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {   
            Data_in_redact(menu, pos_y);
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // ����������� ������ ������ ����
{
    FontSet(font);
    //uint8_t leng_font = 0;
    if (len == 0x00)
    {   
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }
    else
    {
        //leng_font = 1;
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }

    if (selectedMenuItem->data_out != (void *)&NULL_ENTRY)
    {
        OLED_DrawStr(menu->data_out, winth_display - (OLED_GetWidthStr(menu->data_out) + 4), pos_y * dist_y + height_up_menu, 1);
    }
    
}
void menuChange(menuItem *NewMenu)
{
    if ((void *)NewMenu == (void *)&NULL_ENTRY)
        return;
    selectedMenuItem = (menuItem *)(NewMenu);
}

void Display_TopBar(menuItem *CurrentMenu)
{
    OLED_DrawHLine(line_indentation, line_ind_top, end_line, 1);
    if ((void *)CurrentMenu->Parent != (void *)&NULL_ENTRY)
    {
        OLED_DrawTriangleFill(back_pic_pos_x, back_pic_pos_y, back_pic_pos_x + size_back_pic_x, back_pic_pos_y + size_back_pic_y, back_pic_pos_x + size_back_pic_x, back_pic_pos_y - size_back_pic_y);
        OLED_DrawPixel(back_pic_pos_x + 1, back_pic_pos_y);
        OLED_DrawStr(CurrentMenu->Parent, left_pic_last_munu, top_pic_last_munu, 1);
    }
    //OLED_DrawStr("005�08�", 1, 1, 1);
/*
    W25_Ini();
    unsigned int id = W25_Read_ID();
    sprintf(str, "ID Value: 0x%X\n", id);
    OLED_DrawStr(str, 1, 1, 1);
*/
/*
    uint32_t data = Read_MS5193T_Data();
    double koeff = 0.0000000697; 
    koeff = data*koeff-0.4;
    koeff *= 9.375; 
    gcvt(koeff, 4, str);
    OLED_DrawStr(str, 1, 1, 1);
*/
/*
    uint32_t data = Read_MS5193T_Data();
    double koeff = 0.0000000697; 
    koeff = data*koeff-0.5;
    koeff /= 0.01; 
    gcvt(koeff, 4, str);
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
    for (; c > 0; c--)
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

    const uint8_t* signal_icons[] = {signal_0, signal_1, signal_2, signal_3};
    //GSM_Signal_Level = 3;
    if (GSM_Signal_Level >= 0 && GSM_Signal_Level <= 3) {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_icons[GSM_Signal_Level]);
    }
    right_ot = winth_display - 12 - 2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������
    //������� �����������
}

// ����������� ���� ������� ���� �� ��������
void Display_all_menu()
{
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

// ���� � ����� ��������������, ���� ���� ��������������� ����
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        // ���� ��������
        if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
        {   
            for (int i = 0; i<3; i++)
            {
                char separat[11] = {'\0'};
                for (int j = 0; j<11; j++) selectedMenuItem->data_in->data_temp[i][j] = '\0';
                formatters[selectedMenuItem->data_in->data_type[i]](selectedMenuItem->data_in->data_temp[i], sizeof(selectedMenuItem->data_in->data_temp[i]), selectedMenuItem->data_in->data[i]);
                uint8_t len_dat = search_len_mass(selectedMenuItem->data_in->data_temp[i], 11, separat);
                for (int j = 0; j < (uint8_t)selectedMenuItem->data_in->len_data_zero[i] - len_dat; j++){
                    if (selectedMenuItem->data_in->data_type[i] != 3) separat[j] = '0';
                    if (selectedMenuItem->data_in->data_type[i] == 3) separat[j] = 0x99;
                }
                strcat(separat, selectedMenuItem->data_in->data_temp[i]);
                strcpy(selectedMenuItem->data_in->data_temp[i], separat);
            }
        }
        if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY) Intermediate = *selectedMenuItem->select_bar->data;

        mode_redact = 1;
        pos_redact = 0;
        time_update_display = time_led_cursor;
        xSemaphoreGive(Display_semaphore);
    }
}

void Save_general_format(){
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10);
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        if (selectedMenuItem->data_in->data_type[i] == 0) *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 1) *((uint16_t *)selectedMenuItem->data_in->data[i]) = (uint16_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 2) *((int32_t *)selectedMenuItem->data_in->data[i]) = result;
        if (selectedMenuItem->data_in->data_type[i] == 3) selectedMenuItem->data_in->data[i] = selectedMenuItem->data_in->data_temp[i];
    }
}

void Save_date_format(){
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10);
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
    }
    RTC_set_date();
    RTC_read();
}

void Save_time_format(){
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10);
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
    }
    RTC_set_time();
    RTC_read();
}


void redact_end()
{
    // ������ ������ � ����������� �������� � ����� ����������� �������

    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){ 
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        pos_redact = len_witout_separator - 1;
        selectedMenuItem->data_in->end_redact_func(); // ����� ������� ��� ���������� ���������
    }
    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY) *selectedMenuItem->select_bar->data = Intermediate;
    
    //pos_redact = len_dat - 1;
    mode_redact = 0;
    led_cursor = 1;
    time_update_display = time_updateDisplay;
}


void position_calculate(int8_t *_add_pos, int *_position)
{
    int separators_count = 0;
    for (int i = 0; i < 3; i++) if (selectedMenuItem->data_in->separators[i] != '\0') separators_count++;
    for (; *_add_pos < separators_count; (*_add_pos)++)
    {
        *_position -= selectedMenuItem->data_in->len_data_zero[*_add_pos];
        if (*_position < 0)
        {
            *_position += selectedMenuItem->data_in->len_data_zero[*_add_pos];
            return;
        }
    }
}

// �������� �� ������������ ������ �������� � ���������

void up_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        // �������� ��� �����. ������� ����
        int8_t add_pos = 0;
        int position = pos_redact;
        position_calculate(&add_pos, &position);
        // ���� ��� ������ char
        if (selectedMenuItem->data_in->data_type[add_pos] == 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position]+1 != 0) selectedMenuItem->data_in->data_temp[add_pos][position]++;
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == 0x98 || 0xA0 || 0xAD) selectedMenuItem->data_in->data_temp[add_pos][position]++;
        // �������� ���������� ������� �� ������������ ������ �� ���������
        1
        }
        // ���� ��� ������ �������������
        if (selectedMenuItem->data_in->data_type[add_pos] != 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == '9') selectedMenuItem->data_in->data_temp[add_pos][position] = '0';
            else selectedMenuItem->data_in->data_temp[add_pos][position]++;
        }
        //data_correct(add_pos);
        // �������� �������� min-max

        led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������
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
        // �������� ��� �����. ������� ����
        int8_t add_pos = 0;
        int position = pos_redact;
        position_calculate(&add_pos, &position);
        // ���� ��� ������ char
        if (selectedMenuItem->data_in->data_type[add_pos] == 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position]-1 > 32) selectedMenuItem->data_in->data_temp[add_pos][position]--;
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == 0x98 || 0xA0 || 0xAD) selectedMenuItem->data_in->data_temp[add_pos][position]--;
        }
        // ���� ��� ������ �������������
        if (selectedMenuItem->data_in->data_type[add_pos] != 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == '0') selectedMenuItem->data_in->data_temp[add_pos][position] = '9';
            else selectedMenuItem->data_in->data_temp[add_pos][position]--;
        }
        //data_correct(add_pos);
        // �������� �������� min-max

        led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������
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
        // �������� ��� �����. ������� ������
    }

    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY)
    {
        if (selectedMenuItem->select_bar->data == 0){
            mode_redact = 0;
            led_cursor = 1;
        }
        else{
            time_update_display = time_updateDisplay;
            if (selectedMenuItem->select_bar->Name[Intermediate-1][0][0] != '\0'){
                //*selectedMenuItem->select_bar->data-=1;
                Intermediate-=1;
            }
        }
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
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        pos_redact++;
        led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������
        if (pos_redact >= len_witout_separator)
        {
            redact_end();
        }

        // �������� ��� �����. ������� ������
    }

    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY)
    {
        if (selectedMenuItem->select_bar->Name[Intermediate+1][0][0] != '\0'){
            //*selectedMenuItem->select_bar->data+=1;
            Intermediate+=1;
        }
        // �������� ��� ��������� ������
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
void ok()
{
    if (mode_redact == 1)
    {
        redact_end();
        return;
    }
    mode_check();
}
/*  ��� ���� (char)0b543210
    6 - �������                                             0x40
    5 - ���� ��������                                       0x20
    4 - ���������� ������ (��������� ������ �� ���������)   0x10
    3 - ����� ������������ ��������� char[]                 0x08
    0 - �� ������� - ��������                               0x01
*/

// �������������� ������� � ������  ������������
void data_redact_pos(char data)
{
    int8_t add_pos = 0;
    int position = pos_redact;
    position_calculate(&add_pos, &position);
    selectedMenuItem->data_in->data_temp[add_pos][position] = data;
}

void key_press_data_write(char data)
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        data_redact_pos(data);
        pos_redact++;
        led_cursor = 0; // ����� ��� �� ������ ����� ��������� ��� ������������
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        if (pos_redact >= len_witout_separator)
            pos_redact -=  1;
    }
}
void null_fun(){}
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
            if (Keyboard_press_code >= '0' && Keyboard_press_code <= '9')
            {
                key_press_data_write(Keyboard_press_code);
            }
        }
        Keyboard_press_code = 0xFF;
    }
}

///
/// ����� �����
///


extern const uint8_t frames[23][1026];
extern const uint16_t frame_delays[];

void UpdateFrameDiff(const uint8_t *new_frame) {
    static uint8_t old_frame[1024] = {0}; // ������ ����
    uint8_t temp_buffer[1024] = {0}; // ������������� ����� ��� ������ �����

    uint8_t width = new_frame[0];
    uint8_t height = new_frame[1];
    const uint8_t* data_ptr = &new_frame[2];
    int widthInBytes = (width + 7) / 8;

    // ���������� ����� ���� � temp_buffer ��� ������ ������ � oled_buffer
    // � ������ ������ �� ������ ����������� XBM � ������ ������ � oled_buffer
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t data = data_ptr[(y * widthInBytes) + (x / 8)];
            uint8_t bit = 1 << (x & 7);
            if (data & bit) {
                temp_buffer[x + (y/8)*OLED_WIDTH] |= (1 << (y & 7));
            }
        }
    }

    // ���������� temp_buffer � old_frame, ����� ����� ������������ ����
    uint8_t changed_pages[8] = {0}; // ����������� ����� �������� ����������
    for (int i = 0; i < 1024; i++) {
        uint8_t diff = temp_buffer[i] ^ old_frame[i];
        if (diff != 0) {
            int page = i / OLED_WIDTH;
            int x_pos = i % OLED_WIDTH;
            for (int bit = 0; bit < 8; bit++) {
                if (diff & (1 << bit)) {
                    // ������� ���������
                    uint8_t new_pixel = (temp_buffer[i] & (1 << bit)) ? 1 : 0;
                    OLED_DrawPixelStatus(x_pos, page*8 + bit, new_pixel);
                    changed_pages[page] = 1;
                }
            }
        }
    }

    // ��������� ������ �� ��������, ������� ����������
    for (int p = 0; p < 8; p++) {
        if (changed_pages[p]) {
            OLED_UpdateOnePage(p);
        }
    }

    // ���������� ������� ���� ��� ������ ��� ���������� �������
    memcpy(old_frame, temp_buffer, 1024);
}

void Start_video() {
    int contrast = 0xFF;
    int charge_period = 0xF1;
    for (int i = 0; i < 23; i++) {
        UpdateFrameDiff(frames[i]);
        HAL_Delay(frame_delays[i]);
        if (i>15){
            OLED_SetContrast(contrast);
            OLED_SendCommand(0xD9);  //  ��������� ���
            OLED_SendCommand(charge_period);  // ��������� ���
            contrast -= 31;
            charge_period-=28;
        }
    }
    
    OLED_SendCommand(0xD9);  //  ��������� ��� �� ������� �����
    OLED_SendCommand(0xF1);  //  ��������� ��� �� ������� �����
    OLED_SetContrast(0xFF);
}