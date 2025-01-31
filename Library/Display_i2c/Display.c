#include "Display.h"
// #include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 0                            //  ������� ������� ����� ����� � ������
#define end_line 125                                  //  ������� ������� ����� ����� � ������
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
extern xSemaphoreHandle Display_semaphore;
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

extern GSM_STATUS_item GSM_data;
extern ADC_MS5193T_item ADC_data;
extern EEPROM_Settings_item EEPROM;


char str[4];
int right_ot = winth_display - 12 - 2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������

char trans_str[11];

extern char Keyboard_press_code;
extern int ADC_AKB_Proc;


const int max_munu_in_page = 4; // ������������ ���������� ������� ���� �� �������� ������� (+1)
int select_menu_in_page = 0;    // ������� ����� ����c
uint8_t Intermediate = 0;           // ������������� ����������, ���� ����������� ��������� �� ���������� ��� ��������� (����� ������)



#define height_up_menu 14                                            // ����� �������� ������ ����
#define dist_y 12 // ���������� ����� �������� ����
#define pos_x_menu 4                                                 // ������ �� ���� ��� �������� ������� ����
#define pos_x_menu_data 100                                          // ������ �� ���� ��� ������ ��������

#define font my5x7fonts

extern char ver_board[];
extern const char VERSION_PROGRAMM[10];
extern char time_work_char[];

menuItem Null_Menu = {{0}, {0}, 0, "", '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
#define NEXT_MENU Null_Menu
#define PREVISION_MENU Null_Menu
#define PARENT_MENU Null_Menu
#define CHILD_MENU Null_Menu
#define ACTION_MENU Null_Menu
#define SELECT_BAR Null_Menu
#define DATA_IN Null_Menu
#define DATA_OUT Null_Menu
uint8_t dat = 0;
#define NULL_ENTRY Null_Menu
#define UPTADE_ON 1
#define UPTADE_OFF 0

extern TIM_HandleTypeDef htim6;
void NULL_F(){}
#define MAKE_MENU(Id, Name_rus, Name_en, Num_menu, data_uptade, _add_signat, Next, Previous, Parent, Child, action, select_bar, data_in, Data_out) \
    extern menuItem Next;                                                                                                                              \
    extern menuItem Previous;                                                                                                                          \
    extern menuItem Parent;                                                                                                                            \
    extern menuItem Child;                                                                                                                             \
    menuItem Id = {Name_rus, Name_en, 0, data_uptade, (menuSelect_item *)&_add_signat, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&action, (menuSelect_item *)&select_bar, (menuSelect_item_char *)&data_in, (char *)&Data_out}


// ���������� �������� � ������
menuSelect_item Communication_DATA = { 
    .data = (uint8_t *)&EEPROM.Communication,
    .Name = {
        {"GSM/NB-IoT", "GSM/NB-IoT"},
        {"����", "Off"}
    },
}; 
menuSelect_item RS485_MODE_DATA = {
    (uint8_t *)&EEPROM.RS485_prot,
    {
        {"����.", "Off"},
        {"Modbus", "Modbus"}
    }
};
menuSelect_item UNITS_MODE_DATA = {
    (uint8_t *)&EEPROM.units_mes,
    {
        {"��", "mm"},
        {"�", "m"},
        {"��", "Pa"}
    }
};
menuSelect_item SCREENSAVER = {
    (uint8_t *)&EEPROM.screen_sever_mode ,
    {
        {"����.", "off"},
        {"���.", "on"}
    }
};

menuSelect_item LANGUAGE = {
    (uint8_t *)&EEPROM.len,
    {
        {"�������", "�������"},
        {"English", "English"}
    }
};


menuSelect_item CURRENT_LOOP = {
    (uint8_t *)&ADC_data.mode,
    {
        {"4-20��", "4-20mA"},
        {"0-20��", "0-20mA"},
        {"����.", "Off"}
    }
};

// ������� �� USB
extern uint8_t USB_mode;
// ����� USB, - ������ � ������� flash (0), ������ � ���������� ������ (1), ������ USB � ������ ������ ���������� flash (2), ������ � ������ ������ SD ����� (3), ������ � ������ ������ USB (4)
menuSelect_item USB_MODE_STRUCT = {
    (uint8_t *)&EEPROM.USB_mode,
    {
        {"������ USB", "FLASH USB"},           // �������� ��������
        {"�������", "Comand"},
        {"����. FLASH", "Int. FLASH"},
        {"����. SD", "Int. SD"},
        {"USB DEBUG", "USB DEBUG"}
    }
};

menuSelect_item Unit_voltage = {
    (uint8_t *)&dat, { {"�", "v"} }
};
menuSelect_item Unit_hours = {
    (uint8_t *)&dat, { {"�", "h"} }
};
menuSelect_item Unit_minuts = {
    (uint8_t *)&dat, { {"�", "m"} }
};
menuSelect_item Unit_degree = {
    (uint8_t *)&dat, { {"�", "�"} }
};
menuSelect_item Unit_resistance = {
    (uint8_t *)&dat, { {"��", "Ohms"} }
};
menuSelect_item Unit_current = {
    (uint8_t *)&dat, { {"�", "Amps"} }
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
    if ((*(int32_t *)data) < 0)
    {
        *(int32_t *)data *=-1;
        snprintf(buffer, size, "%ld", *(int32_t *)data);
        *(int32_t *)data *=-1;
    }
    else snprintf(buffer, size, "%ld", *(int32_t *)data);
    
}
void format_char(char *buffer, size_t size, void *data) {
    char *str_data = (char *)data;
    strncpy(buffer, str_data, size - 1);
    buffer[size - 1] = '\0';
}

DataFormatter formatters[] = {
    format_uint8_t,
    format_uint16_t,
    format_int32_t,
    format_char,
    format_int32_t
};
void Null_func(){}




// ���������� ���������
extern GSM_STATUS_item GSM_data;
// �������������� ���� RTC
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
menuSelect_item_char Date_redact = {
    {'.', '.', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    3,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    '\0',
    //  ������
    {&Date.Date, &Date.Month, &Date.Year},  // �������� ��������
    {0, 0, 0},                  // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                     // ���� (�������������)
    {2, 2, 2},                  // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                  // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},         // ������������� ��������
    {31, 12, 99},               // ������������ ��������
    {0, 0, 0},                  // ����������� ��������
    Save_date_format,           // ������ �� ������� ���������� ������
    '\0'
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// �������������� ������� RTC
menuSelect_item_char Time_redact = {
    {':', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    '\0',
    //  ������
    {&Time.Hours, &Time.Minutes, '\0'},  // �������� ��������
    {0, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                   // ���� (�������������)
    {2, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {23, 59, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Save_time_format,                    // ������ �� ������� ���������� ������
    '\0',
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// ����� ���, ����������

menuSelect_item_char Time_sleep_redact = {
    {'�', '�', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    '\0',
    //  ������
    {&EEPROM.time_sleep_h, &EEPROM.time_sleep_m, '\0'},  // �������� ��������
    {1, 1, 1},                  // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                  // ���� (�������������)
    {3, 2, 0},                  // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {1, 1, 0},                  // ������ ����� ��� �������������� (�����������) (001 - ������ 3, 23 - ������ 2) 
    {"\0", "\0", "\0"},         // ������������� ��������
    {999, 59, '\0'},            // ������������ ��������
    {0, 5, '\0'},               // ����������� ��������
    Save_general_format,         // ������ �� ������� ���������� ������
    '\0',
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)


// ���, ������� ������ ���������
menuSelect_item_char Max_Level_Mesurment = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    &ADC_data.MAX_LVL,
    //  ������
    {&ADC_data.MAX_LVL_char[0], &ADC_data.MAX_LVL_char[1], '\0'},  // �������� ��������
    {2, 2, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������), 4 - float (��������)
    {0, 0},                     // ���� (�������������)
    {6, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99999, 999, '\0'},              // ������������ ��������
    {-99999, 0, '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    SAVE_DOUBLE
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// ���, ������ ������ ���������
menuSelect_item_char Min_Level_Mesurment = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    &ADC_data.ZERO_LVL,
    //  ������
    {&ADC_data.ZERO_LVL_char[0], &ADC_data.ZERO_LVL_char[1], '\0'},  // �������� ��������
    {2, 2, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������) 4 - int32 �����������
    {0, 0},                   // ���� (�������������)
    {6, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99999, 999, '\0'},              // ������������ ��������
    {-99999, 0, '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    SAVE_DOUBLE
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

// ���������� ��� 
void SAVE_DOUBLE(double **save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed){
    **save_data = 0;
    **save_data+=*value_int;
    double temp = (double)*value_float;
    for (int i=0; i<size_float; i++) temp/=10;
    **save_data+=temp;
    if (_signed == 1) **save_data*=-1;
}

menuSelect_item_char GVL_Correct = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    &ADC_data.GVL_correct,
    //  ������
    {&ADC_data.GVL_correct_char[0], &ADC_data.GVL_correct_char[1], '\0'},  // �������� ��������
    {2, 2, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                      // ���� (�������������)
    {5, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 2},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99999, 999, '\0'},              // ������������ ��������
    {-99999, 0, '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    SAVE_DOUBLE
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

menuSelect_item_char Serial_number = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    3,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-��������� 
    '\0',
    //  ������
    {&EEPROM.version.VER_PCB_IDEOLOGY, &EEPROM.version.VER_PCB_VERSION, &EEPROM.version.VER_PCB_INDEX},  // �������� ��������
    {0, 0, 3},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                      // �������� ������ ��� �����������
    {2, 2, 4},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 0},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99, 99, '\0'},              // ������������ ��������
    {0, 0, '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    '\0'
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

////////////////////////////////////////////////////
//                 ������� ����                   //
////////////////////////////////////////////////////

////////////////////////////////////////////////////
//                  ������ ����                   //
////////////////////////////////////////////////////

extern char EEPROM_status_char[3]; // ������ ����������� EEPROM
extern char FLASH_status_char[3];  // ������ ����������� FLASH
extern char SD_status_char[3];     // ������ ����������� SD
extern char  ADC_in_temp_char[5];

// ������ ���� ��� ������� ������
void full_test(){}

// ���� �������� ���
void GSM_sms_test(){}

// ���� 
void GSM_internet_test(){}

// ���������� �� 
void Programm_Update_USB(){}

void sleep(){
    Enter_StandbyMode(0, 30);
}

//uint8_t dat = 0;
menuSelect_item NO_SIGNED = {
    (uint8_t *)&dat, { {"", ""} }
};

  MAKE_MENU(Menu_1    , "������"       , "Modes"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2         , PREVISION_MENU , PARENT_MENU    , Menu_1_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_1_1  , "����"         , "Cycle"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_1_2       , PREVISION_MENU , Menu_1         , CHILD_MENU     , sleep    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_1_2  , "�����������"         , "Test check"         , 0 , UPTADE_OFF               ,  NO_SIGNED  , Menu_1_3       , Menu_1_1       , Menu_1         , Menu_1_2_1     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_1, "��� ������"   , "Error code"   , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_1_2_2     , PREVISION_MENU , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_2, "������ ����"  , "Full test"    , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_1_2_3     , Menu_1_2_1     , Menu_1_2       , CHILD_MENU     , full_test      , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_3, "���"          , "BAT"          , 0 , UPTADE_OFF               ,   Unit_voltage          ,  Menu_1_2_4     , Menu_1_2_2     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_AKB_volts_char);
      MAKE_MENU(Menu_1_2_4, "���"          , "ADC"          , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_1_2_5     , Menu_1_2_3     , Menu_1_2       , Menu_1_2_4_1   , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_status_char);
        MAKE_MENU(Menu_1_2_4_1, "���"          , "ADC"          , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_1_2_4_2   , PREVISION_MENU , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_value_char);
        MAKE_MENU(Menu_1_2_4_2, "���������"    , "Metering values"          , 0 , UPTADE_ON               ,  UNITS_MODE_DATA ,  Menu_1_2_4_3   , Menu_1_2_4_1   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_char);
        MAKE_MENU(Menu_1_2_4_3, "������������� �����"    , "Shunt resistance"          , 0 , UPTADE_ON               , Unit_resistance  ,  Menu_1_2_4_4   , Menu_1_2_4_2   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_correct_char);
        MAKE_MENU(Menu_1_2_4_4, "���������� �����", "Shunt voltage"          , 0 , UPTADE_ON               ,  Unit_voltage ,  Menu_1_2_4_5   , Menu_1_2_4_3   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN           , ADC_data.ADC_Volts_char);
        MAKE_MENU(Menu_1_2_4_5, "��� �����"          , "Shunt current"          , 0 , UPTADE_ON               ,  Unit_voltage ,  NEXT_MENU      , Menu_1_2_4_4   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_Current_char);
      MAKE_MENU(Menu_1_2_5, "�����"          , "Mobile network"          , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_1_2_6     , Menu_1_2_4     , Menu_1_2       , Menu_1_2_5_1   , ACTION_MENU    , SELECT_BAR     , DATA_IN            ,    GSM_data.GSM_status_char);
        MAKE_MENU(Menu_1_2_5_1, "���� ���"     , "SMS test"          , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_1_2_5_2   , PREVISION_MENU , Menu_1_2_5     , CHILD_MENU     , GSM_sms_test   , SELECT_BAR     , DATA_IN        , DATA_OUT);
        MAKE_MENU(Menu_1_2_5_2, "���� ����"    , "SITE test"          , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_1_2_5_3   , Menu_1_2_5_1   , Menu_1_2_5     , CHILD_MENU     , GSM_internet_test, SELECT_BAR   , DATA_IN        , DATA_OUT);
        MAKE_MENU(Menu_1_2_5_3, "SIM-�����"    , "SIM-card"          , 0 , UPTADE_ON               , NO_SIGNED  ,  Menu_1_2_5_4   , Menu_1_2_5_2   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_SIMCARD_char);
        MAKE_MENU(Menu_1_2_5_4, "������"   , "Status"          , 0 , UPTADE_ON               , NO_SIGNED  ,  Menu_1_2_5_5   , Menu_1_2_5_3   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_status_ready_char);
        MAKE_MENU(Menu_1_2_5_5, "�����������"  , "Logging"          , 0 , UPTADE_ON               , NO_SIGNED  ,  Menu_1_2_5_6   , Menu_1_2_5_4   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_status_reg_char);
        MAKE_MENU(Menu_1_2_5_6, "��������"     , "Mobile operator"          , 0 , UPTADE_ON               , NO_SIGNED  ,  Menu_1_2_5_7   , Menu_1_2_5_5   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_operator_char);
        MAKE_MENU(Menu_1_2_5_7, "������� ����.", "Signal level"          , 0 , UPTADE_ON               , NO_SIGNED  , Menu_1_2_5_8   , Menu_1_2_5_6   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_signal_lvl_char);
        MAKE_MENU(Menu_1_2_5_8, "������ GPRS", "GPRS status"          , 0 , UPTADE_ON               , NO_SIGNED  ,  NEXT_MENU      , Menu_1_2_5_7   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_gprs_on_char);
      MAKE_MENU(Menu_1_2_6, "EEPROM"       , "EEPROM"       , 0 , UPTADE_ON               , NO_SIGNED  ,  Menu_1_2_7     , Menu_1_2_5     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , EEPROM_status_char);
      MAKE_MENU(Menu_1_2_7, "SD-�����"           , "SD-card"           , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_1_2_8     , Menu_1_2_6     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , FLASH_status_char);
      MAKE_MENU(Menu_1_2_8, "FLASH"        , "FLASH"        , 0 , UPTADE_OFF               , NO_SIGNED  ,  NEXT_MENU      , Menu_1_2_7     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , SD_status_char);
    MAKE_MENU(Menu_1_3  , "���������"    , "Sensor reading"         , 0 , UPTADE_OFF               , NO_SIGNED  , NEXT_MENU      , Menu_1_2       , Menu_1         , Menu_1_3_1     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_3_1, "����������"   , "Temperature "         , 0 , UPTADE_ON               ,      Unit_degree       , Menu_1_3_2     , PREVISION_MENU , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_MS5193T_temp_char);
      MAKE_MENU(Menu_1_3_2, "�������"      , "Data"         , 0 , UPTADE_ON               ,       UNITS_MODE_DATA     , Menu_1_3_3     , Menu_1_3_1     , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_value_char);
      MAKE_MENU(Menu_1_3_3, "���"   , "GWL"         , 0 , UPTADE_ON               ,      UNITS_MODE_DATA     , NEXT_MENU      , Menu_1_3_2     , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_correct_char);
  MAKE_MENU(Menu_2    , "���������"    , "Settings"     , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_3         , Menu_1         , PARENT_MENU    , Menu_2_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_1  , "����"         , "Date"         , 0 , UPTADE_OFF               , NO_SIGNED  ,Menu_2_2       , PREVISION_MENU , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Date_redact    , DATA_OUT);
    MAKE_MENU(Menu_2_2  , "�����"        , "Time"         , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_3       , Menu_2_1       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Time_redact    , DATA_OUT);
    MAKE_MENU(Menu_2_3  , "����� ���"    , "Seep time"   , 0 , UPTADE_OFF               , NO_SIGNED            ,  Menu_2_4       , Menu_2_2       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Time_sleep_redact, DATA_OUT);
    MAKE_MENU(Menu_2_4  , "����. ���"     , "GWL correction"        , 0 , UPTADE_OFF               ,      UNITS_MODE_DATA        , Menu_2_5       , Menu_2_3       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , GVL_Correct        , DATA_OUT);
    MAKE_MENU(Menu_2_5  , "���"          , "U-range"        , 0 , UPTADE_OFF               ,          UNITS_MODE_DATA     , Menu_2_6       , Menu_2_4       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Max_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_6  , "���"          , "L-range"        , 0 , UPTADE_OFF               ,          UNITS_MODE_DATA      , Menu_2_7       , Menu_2_5       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Min_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_7  , "�����"        , "Network"          , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_8       , Menu_2_6       , Menu_2         , CHILD_MENU     , ACTION_MENU    , Communication_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_8  , "����. �� ����", "Data uploading"          , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_9       , Menu_2_7       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_9  , "������ �� USB", "USB write"          , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_10       , Menu_2_8       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_10 , "������� �����"   , "Current loop "        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_11       , Menu_2_9       , Menu_2         , CHILD_MENU     , ACTION_MENU    , CURRENT_LOOP   , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_11 , "RS-485"        , "RS-485"        , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_2_12       , Menu_2_10       , Menu_2         , CHILD_MENU     , ACTION_MENU    , RS485_MODE_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_12 , "��. ���."     , "Unit measure"        , 0 , UPTADE_OFF               , NO_SIGNED    , Menu_2_13      , Menu_2_11       , Menu_2         , CHILD_MENU     , ACTION_MENU    , UNITS_MODE_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_13 , "�����. ����"  , "Administration menu"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_14      , Menu_2_12       , Menu_2         , Menu_2_13_1    , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_1, "��������"     , "Startup screen"    , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_13_2      , PREVISION_MENU    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SCREENSAVER    , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_2, "ID ����.", "ID Device"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_13_3    , Menu_2_13_1 , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Serial_number  , DATA_OUT);
      MAKE_MENU(Menu_2_13_3, "���������� ��", "Update of software"          , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_13_4       , Menu_2_13_2       , Menu_2_13      , CHILD_MENU     , Programm_Update_USB, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_4, "����� USB", "USB mode"  , 0 , UPTADE_OFF               , NO_SIGNED    , Menu_2_13_5    , Menu_2_13_3    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , USB_MODE_STRUCT, DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_5, "���������� �����(����)", "Shunt calibration(up)"  , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_13_6    , Menu_2_13_4    , Menu_2_13      , CHILD_MENU     , ACTION_MENU, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_6, "���������� �����(���)", "Shunt calibration(low)"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_13_7    , Menu_2_13_5    , Menu_2_13      , CHILD_MENU     , ACTION_MENU, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_7, "����. �����", "Shunt resistance"      , 0 , UPTADE_OFF               ,  Unit_resistance , Menu_2_13_8    , Menu_2_13_6    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_8, "����. ����."   , "Offset temperature"      , 0 , UPTADE_OFF               ,  Unit_degree  , Menu_2_13_9    , Menu_2_13_7    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_9, "!!!"     , "!!!"      , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_13_10    , Menu_2_13_8    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_10, "����. ��"     , "MCU temp."      , 0 , UPTADE_OFF               ,  Unit_degree  , Menu_2_13_11    , Menu_2_13_9    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_11, "����. ����."  , "Analog temp."     , 0 , UPTADE_OFF               ,  Unit_degree , Menu_2_13_12    , Menu_2_13_10    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_12, "����. ����."  , "Digital sensor temp."     , 0 , UPTADE_OFF               , Unit_degree , Menu_2_13_13   , Menu_2_13_11    , Menu_2_13      , CHILD_MENU     , SELECT_BAR    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_13, "���� FLASH"   , "FLASH test"     , 0 , UPTADE_OFF               , NO_SIGNED   , Menu_2_13_14   , Menu_2_13_12    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_14, "���� EEPROM"  , "EEPROM test"     , 0, 0              , NO_SIGNED   , Menu_2_13_15   , Menu_2_13_13    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_15, "���� SD-�����"      , "SD-card test"     , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_2_13_16   , Menu_2_13_14    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_16, "����� ����������"   , "BLOCK mode"     , 0 , UPTADE_OFF               , NO_SIGNED  ,  NEXT_MENU   , Menu_2_13_15    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , DATA_IN        , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_14 , "����"         , "Language"     , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_2_15      , Menu_2_13      , Menu_2         , CHILD_MENU     , ACTION_MENU    , LANGUAGE       , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_15 , "����� ��������"        , "Factory reset"        , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_2_16      , Menu_2_14      , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_16 , "������. SD"   , "SD formatting"        , 0 , UPTADE_OFF              , NO_SIGNED  ,  NEXT_MENU      , Menu_2_15      , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
  MAKE_MENU(Menu_3    , "��������"     , "Info"         , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_4         , Menu_2         , PARENT_MENU    , Menu_3_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_3_1  , "ID ����."     , "ID Device"    , 0 , UPTADE_OFF               , NO_SIGNED  ,  Menu_3_2       , PREVISION_MENU , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , EEPROM.version.VERSION_PCB);
    MAKE_MENU(Menu_3_2  , "���. ��"      , "Software version"        , 0 , UPTADE_OFF               , NO_SIGNED  , Menu_3_3       , Menu_3_1       , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , EEPROM.version.VERSION_PROGRAMM);
    MAKE_MENU(Menu_3_3  , "����� ������" , "Elapsed time"        , 0 , UPTADE_OFF               ,NO_SIGNED  , NEXT_MENU      , Menu_3_2       , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , EEPROM.version.time_work_char);
  MAKE_MENU(Menu_4    , "����������"   , "Instruction"  , 0 , UPTADE_OFF               , NO_SIGNED  ,NEXT_MENU      , Menu_3         , PARENT_MENU    , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

/// ���������� ����� ������ ��� ����� ����� � ���������
int search_len_mass(char *string, int len_str, char separator[])
{
    int counter = 0; 
    for (int i = 0; i < len_str; i++)
    {
        if (string[i] == '\0') return counter;
        if ((string[i] != separator[0]) && (string[i] != separator[1]) && (string[i] != separator[2]))
            counter++;
    }
    return -1;
}


void split_double(double *number, int32_t *int_part, int32_t *frac_part, uint8_t precision) {
    if (number == NULL || int_part == NULL || frac_part == NULL) {
        return; // ������ �� �������� NULL-����������
    }

    // ��������� ����� �����
    *int_part = (int32_t)(*number); // ��������� ���� � ����� �����
    // ��������� ������� �����
    double fractional = *number - (double)(*int_part);
    // �������� ������� ����� �� 10^precision � ���������
    double scaling_factor = pow(10, precision);
    *frac_part = (int32_t)(fractional * scaling_factor + 0.5); // ���������� ������� �����
}


// ����� �������� data_in ��� ������ ��������������
void Data_in_no_redact(menuItem *menu, int pos_y){
    char string[20] = {'\0'};
    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        char buffer[20] = {'\0'};
        char separat[20] = {'\0'}; // ������ ������������, � ������ ������� �����, ������������ ����
        if (menu->data_in->data_double != 0x00)
        {
            if (**menu->data_in->data_double < 0)
            {
                menu->data_in->unsigned_signed[0] = 1;
            }
            else
            {
                menu->data_in->unsigned_signed[0] = 0;
            }
            split_double(*menu->data_in->data_double, menu->data_in->data[0], menu->data_in->data[1], menu->data_in->len_data_zero[1]);
        }

        if (menu->data_in->data_type[i] == 2) if (menu->data_in->unsigned_signed[0] == 1) string[0] = '-';
        formatters[menu->data_in->data_type[i]](buffer, sizeof(buffer), menu->data_in->data[i]);
        // �������� ���� ����� ��������
        if (menu->data_in->data_type[i] != 3) 
        {
            uint8_t len_dat = search_len_mass(buffer, 11, separat);
            for (int j = 0; j < (uint8_t)menu->data_in->len_data_zero_unredact[i] - len_dat; j++)
                separat[j] = '0';
            strcat(string, separat);
        }

        strcat(string, buffer);
        char temp[2] = {menu->data_in->separators[i], '\0'};
        strcat(string, temp);

    }

    uint8_t len_add_signa = 0;
    len_add_signa = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
    uint8_t len_string = OLED_GetWidthStr(string);
    OLED_DrawStr(string, winth_display - (len_string + len_add_signa + 4), pos_y * dist_y + height_up_menu, 1);
}

// ����� �������� data_in � ������ ��������������
void Data_in_redact(menuItem *menu, int pos_y){
    
    char string[11] = {'\0'};
    uint8_t len_sig = 0;
    uint8_t separator_count = 0;
    if (menu->data_in->data_type[0] == 2) if (menu->data_in->unsigned_signed[1] == 1) string[0] = '-'; else string[0] = '+';
    len_sig = OLED_GetWidthStr(string);

    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        menu->data_in->data_temp[i][menu->data_in->len_data_zero[i]] = '\0'; // �� ������ ������
        strcat(string, menu->data_in->data_temp[i]);
        if (menu->data_in->separators[i] != '\0') separator_count++;
        char temp[2] = {menu->data_in->separators[i], '\0'};
        strcat(string, temp);
    }
    uint8_t len_add_signa = 0;
    len_add_signa = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
    uint8_t len_string = OLED_GetWidthStr(string) + 4;
    OLED_DrawStr(string, winth_display - (len_string + len_add_signa), pos_y * dist_y + height_up_menu, 1);
    
    /*
    int8_t add_pos = 0;
    int position = pos_redact;
    for (; add_pos < separator_count; add_pos++)
    {
        position -= selectedMenuItem->data_in->len_data_zero[add_pos];
        if (position < 0)
        {
            position+= selectedMenuItem->data_in->len_data_zero[add_pos];
            break;
        }
    }    

    char string_temp[11] = {'\0'};
    strncpy(string_temp, string, pos_redact + add_pos);
    uint8_t start_pos = OLED_GetWidthStr(string_temp);
    OLED_DrawHLine(winth_display - (len_string + len_add_signa - len_sig) + start_pos, pos_y * dist_y + 8 + height_up_menu, 5, led_cursor=!led_cursor);
*/

    int8_t add_pos = 0;
    int8_t position = pos_redact+1;
    for (; add_pos<separator_count; add_pos++){
        position -= menu->data_in->len_data_zero[add_pos];
        if (position<=0) break;
    }
    char string_temp[11] = {'\0'};
    strncpy(string_temp, string+1, pos_redact+add_pos);
    uint8_t start_pos = OLED_GetWidthStr(string_temp);
    OLED_DrawHLine(winth_display - (len_string + len_add_signa - len_sig) + start_pos, pos_y * dist_y + 8 + height_up_menu, 5, led_cursor=!led_cursor);
    

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
    uint8_t leng_font = 0;
    uint8_t len_signat = 0;
    len_signat = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
    if (len_signat > 0) len_signat += 4;
    OLED_DrawStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len], winth_display-len_signat, pos_y * dist_y + height_up_menu, 1);
    
    
    // ����� ������ (���������)
    if (menu->select_bar != (void *)&NULL_ENTRY)
    {   
        uint8_t num_menu = *menu->select_bar->data;
        if ((selectedMenuItem == menu) && (mode_redact == 1)) num_menu = Intermediate;

        int len_dat = OLED_GetWidthStr(menu->select_bar->Name[num_menu][EEPROM.len]);
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            OLED_DrawStr(menu->select_bar->Name[num_menu][EEPROM.len], winth_display - len_dat - 8, pos_y * dist_y + height_up_menu, 1);  // ����� ���� ������� ����� (��������������)
            int pos_cursor = select_menu_in_page * dist_y + height_up_menu + 2;
            int x_left = winth_display-len_dat-11;
            int x_right = winth_display-7;
            if (menu->select_bar->Name[Intermediate+1][0][0] != '\0') OLED_DrawTriangleFill(x_right, pos_cursor - 1, x_right, pos_cursor + 3, x_right+2, pos_cursor+1);
            if (Intermediate != 0) OLED_DrawTriangleFill(x_left, pos_cursor - 1, x_left, pos_cursor + 3, x_left-2, pos_cursor+1);
        }
        else{
            OLED_DrawStr(menu->select_bar->Name[num_menu][EEPROM.len], winth_display - len_dat - 4, pos_y * dist_y + height_up_menu, 1); // ����� ���� ��� ������ 
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // ����������� ������ ������ ����
{
    FontSet(font);
    
    if (EEPROM.len == 0x00) OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    else OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y * dist_y + height_up_menu, 1);

    if (menu->data_out != (void *)&NULL_ENTRY)
    {
        uint8_t len_add_signa = 0;
        len_add_signa = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
        OLED_DrawStr(menu->data_out, winth_display - (OLED_GetWidthStr(menu->data_out)) - (len_add_signa+4), pos_y * dist_y + height_up_menu, 1);
    }

    if (menu->data_in != (void *)&NULL_ENTRY)
    {
        // ����� �� ��������������
        if ((mode_redact == 0) || (selectedMenuItem != menu)) Data_in_no_redact(menu, pos_y);
        if ((mode_redact == 1) && (selectedMenuItem == menu)) Data_in_redact(menu, pos_y);
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


    if (GSM_data.GSM_Signal_Level == -1)
    {
        right_ot += 3;
        OLED_DrawXBM(right_ot, top_GSM_status, no_signal);
    }

    const uint8_t* signal_icons[] = {signal_0, signal_1, signal_2, signal_3};
    //GSM_data.GSM_Signal_Level = 3;
    if (GSM_data.GSM_Signal_Level >= 0 && GSM_data.GSM_Signal_Level <= 3) {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_icons[GSM_data.GSM_Signal_Level]);
    }
    right_ot = winth_display - 12 - 2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������
    //������� �����������
}

uint8_t speed_update = 0; // ���� �� �� �������� ������ ���� ��������� �������� ����������? (100��)
// ����������� ���� ������� ���� �� ��������
void Display_all_menu()
{
    OLED_Clear(0);
    speed_update = 0;

    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);
    if (menu_s->data_update == 1) speed_update = 1;
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
        if (menu_s->data_update == 1) speed_update = 1;
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
        if (menu_s->data_update == 1) speed_update = 1;
    }
    OLED_UpdateScreen();
    if ((speed_update == 1) || (mode_redact == 1)) SetTimerPeriod(time_led_cursor);
    else SetTimerPeriod(time_updateDisplay);
    
}

// ���� � ����� ��������������, ���� ���� ��������������� ����
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        pos_redact = 0;
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

                selectedMenuItem->data_in->unsigned_signed[1] = selectedMenuItem->data_in->unsigned_signed[0];
                //if (selectedMenuItem->data_in->data_type[i] == 2) pos_redact = 1;
            }
        }
        if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY) Intermediate = *selectedMenuItem->select_bar->data;

        mode_redact = 1;
        led_cursor = 0;
        //xSemaphoreGive(Display_semaphore);

    }
}


// ��� �������� ������� �� ������ ����� � ����� ��������������
void Save_general_format(){
    uint8_t presence_of_number = 0; // �������� �� �� 0
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10); 
        if ((selectedMenuItem->data_in->data_type == 4) && (result<0)) result*=-1; 
        if (selectedMenuItem->data_in->data_type[i] == 3){ strcpy(selectedMenuItem->data_in->data[i], selectedMenuItem->data_in->data_temp[i]); return;}
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        if (selectedMenuItem->data_in->data_type[i] == 0) *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 1) *((uint16_t *)selectedMenuItem->data_in->data[i]) = (uint16_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 2) *((int32_t *)selectedMenuItem->data_in->data[i]) = result;
        if (selectedMenuItem->data_in->data_type[i] == 4) *((int32_t *)selectedMenuItem->data_in->data[i]) = (uint32_t)result;
        if (result!=0) presence_of_number = 1;
    }
    if (presence_of_number == 0) selectedMenuItem->data_in->unsigned_signed[1] = 0;
    selectedMenuItem->data_in->unsigned_signed[0] = selectedMenuItem->data_in->unsigned_signed[1];
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

void Programm_GVL_CORRECT(){}

void redact_end()
{
    // ������ ������ � ����������� �������� � ����� ����������� �������

    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){ 
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        pos_redact = len_witout_separator - 1;

        if (selectedMenuItem->data_in->end_redact_func != '\0') selectedMenuItem->data_in->end_redact_func(); // ����� ������� ��� ���������� ���������
        if (selectedMenuItem->data_in->end_redact_func_2 != NULL)
        {
            selectedMenuItem->data_in->end_redact_func_2(
                selectedMenuItem->data_in->data_double,
                (int32_t *)selectedMenuItem->data_in->data[0],
                (int32_t *)selectedMenuItem->data_in->data[1],
                selectedMenuItem->data_in->len_data_zero[1],
                selectedMenuItem->data_in->unsigned_signed[0]);
        }
    }
    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY) *selectedMenuItem->select_bar->data = Intermediate;
    
    //pos_redact = len_dat - 1;
    HAL_TIM_Base_Stop_IT(&htim6);
    TIM6->SR &= ~TIM_SR_UIF;
    mode_redact = 0;
    led_cursor = 1; 
    EEPROM_SaveSettings(&EEPROM);
    if (!EEPROM_CheckDataValidity()){
        STATUS+=0x08;
    }
}


void position_calculate(int8_t *_add_pos, int *_position)
{
    for (; *_add_pos < selectedMenuItem->data_in->Number_of_cells; (*_add_pos)++)
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
            if (selectedMenuItem->data_in->data_temp[add_pos][position] < 255) selectedMenuItem->data_in->data_temp[add_pos][position]++;
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 122) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 192;
        // �������� ���������� ������� �� ������������ ������ �� ���������
        
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
    if ((selectedMenuItem->select_bar != (void *)&NULL_ENTRY) && (mode_redact == 1)){
        redact_end();
    }

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
            if (selectedMenuItem->data_in->data_temp[add_pos][position] < 31) selectedMenuItem->data_in->data_temp[add_pos][position] = 33;
            if (selectedMenuItem->data_in->data_temp[add_pos][position]-1 > 32) selectedMenuItem->data_in->data_temp[add_pos][position]--;
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 122) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 122;
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
    if ((selectedMenuItem->select_bar != (void *)&NULL_ENTRY) && (mode_redact == 1)){
        redact_end();
    }
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
        int8_t add_pos = 0;
        int position = pos_redact;
        position_calculate(&add_pos, &position);
        uint8_t pos_search = 0; // ������� �� ������� ����� �� ��������� �����������

        pos_redact--;
        led_cursor = 0;
        if (pos_redact < pos_search)
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            TIM6->SR &= ~TIM_SR_UIF;
            mode_redact = 0;
            led_cursor = 1;
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
            if (selectedMenuItem->select_bar->Name[Intermediate-1][0][0] != '\0'){
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
            if (selectedMenuItem->data_in->redact_right_end == 1) redact_end();
            else pos_redact--;
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

/**
 * ����������� ������ c �� �������� � ��������� � ������� (Windows-1251).
 * D -> �, d -> �, � �.�.
 * "������" ������� ����� (����� 26-�) ���������� � 'Z'/'z'.
 */
static unsigned char switch_char(unsigned char c)
{
    // ��������� ��������� [A..Z] = [65..90]
    if (c >= 65 && c <= 90)
    {
        unsigned char index = c - 65;     // 0..25
        // ���������� � '�'(192). ���� ����� ���-�� ����� �� ������ 25, 
        // ������������� ������ ��������� ������������ ����� (�) ��� ������ ����
        if (index < 26) 
            c = 192 + index; // 192..217
        else
            c = 223;         // '�'
    }
    // ��������� ������� [�..�] = [192..223]
    else if (c >= 192 && c <= 223)
    {
        unsigned char index = c - 192;    // 0..31
        // ���� ������ ������ 25 (�.�. �, �, � �), ������������ � 'Z' (90)
        if (index < 26) 
            c = 65 + index;  // 65..90
        else
            c = 90;          // 'Z'
    }
    // �������� ��������� [a..z] = [97..122]
    else if (c >= 97 && c <= 122)
    {
        unsigned char index = c - 97;     // 0..25
        if (index < 26) 
            c = 224 + index; // 224..249
        else
            c = 255;         // '�'
    }
    // �������� ������� [�..�] = [224..255]
    else if (c >= 224 && c <= 255)
    {
        unsigned char index = c - 224;    // 0..31
        // ���� ������ > 25 (�, �, � �) => 'z'
        if (index < 26)
            c = 97 + index;  // 97..122
        else
            c = 122;         // 'z'
    }

    return c;
}


void press_plus_minus(){
    int8_t add_pos = 0;
    int position = pos_redact;
    position_calculate(&add_pos, &position);
    
    // ���� ��� ������ char
    if (selectedMenuItem->data_in->data_type[add_pos] == 2 || 4)
    {
        selectedMenuItem->data_in->unsigned_signed[1] ^= 1;
    }
    if (selectedMenuItem->data_in->data_type[add_pos] == 3)
    {
        unsigned char oldChar = selectedMenuItem->data_in->data_temp[add_pos][position];
        unsigned char newChar = switch_char(oldChar);
        selectedMenuItem->data_in->data_temp[add_pos][position] = newChar;
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
                press_plus_minus();
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