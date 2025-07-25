#include "Display.h"
// #include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 0                            // ������� ������� ����� ����� � ������
#define end_line 125                                  // ������� ������� ����� ����� � ������
#define line_ind_top 11                               // ����� ����� ������
#define back_pic_pos_x 0                              // ������ ������ ���������� ����� ���� x
#define back_pic_pos_y 4                              // ������ ������ ���������� ����� ���� y
#define size_back_pic_x 3                             // ������ ������������ �� x
#define size_back_pic_y 3                             // ������ ������������ �� y
#define top_pic_last_munu 1                           // ������ ������ �� �������� ����������� ������ ����
#define left_pic_last_munu 7                          // ������ ����� �� �������� ����������� ������ ����
#define top_GSM_status 2                              // ������ ������ �� ������� �����
#define width_GSM_status 15                           // ������ ������ ������ ������� �����
#define top_akb_status 1                              // ������ ������ �� ������ ������
#define width_akb_status 7                            // ������ ������ ������ ������

#define time_led_cursor 500      // ����� ���������� ��������� �������� ��� ����� ������
#define time_updateDisplay 20000 // ����� ���������� ������ ��� ����� ������

/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// �������������� ������
int mode_redact = 0;  // 1 - ����� �������������� ������, 0 - ����� ������������ �������, 2 - ����� ����������� (����������� ����) 3 - �� �� ��� � 2, �� ��� ����� ������, 4 - �������� ������, ���� �������� �����������, 5 -����� �� ��� ���
int pos_redact = 0;   // ������� ��� ��������������
char pass[MAX_PASSWPRD_LEN] = "";
uint8_t index_pass = 0;

int led_cursor = 1;
extern xSemaphoreHandle Display_semaphore;
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

extern GSM_STATUS_item GSM_data;
extern ADC_MS5193T_item ADC_data;
extern EEPROM_Settings_item EEPROM;
extern ERRCODE_item ERRCODE;
extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;

char str[5];
int right_ot = winth_display - 12 - 2; // ������ ������ ����� 2 ������� - ������� ������ (0-9%) � - 2 ������ ������

char trans_str[11];

extern char Keyboard_press_code;
extern int ADC_AKB_Proc;
extern const char VERSION_PROGRAMM[20];
extern char time_work_char[15]; // ����� ������ � ���� ������ � �����
extern uint32_t time_work;         // ����� ������ ���������� � ��������

const int max_munu_in_page = 4; // ������������ ���������� ������� ���� �� �������� ������� (+1)
int select_menu_in_page = 0;    // ������� ����� ����c
uint8_t Intermediate = 0;           // ������������� ����������, ���� ����������� ��������� �� ���������� ��� ��������� (����� ������)


#define height_up_menu 14                                            // ����� �������� ������ ����
#define dist_y 12 // ���������� ����� �������� ����
#define pos_x_menu 4                                                 // ������ �� ���� ��� �������� ������� ����
#define pos_x_menu_data 100                                          // ������ �� ���� ��� ������ ��������

#define font my5x7fonts

extern char ver_board[];
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

// ����� �� ����� ��� ��������
const char Clear[2][40] = {"������� ������",  "Memory clear"};
const char Divece[2][40] = {"����������",  "device"};
const char POWER_NOT[2][40] = {"�� ���������� �������",  "DO NOT POWER OFF"};
const char READY[2][40] = {"������",  "Ready"};
const char PASSWORD_IN[2][40] = {"������� ������",  "Enter password"};
const char ERROR_TEXT[2][40] = {"������",  "Error"};
const char DIAPASON_ERR[2][40] = {"��� ���������",  "Out of range"};
const char CALIBRATE_4ma_CORRECT[2][40] = {"�����. 4�� �������",  "4m �alib. compl."};
const char CALIBRATE_20ma_CORRECT[2][40] = {"�����. 20�� �������",  "20mA calib. compl."};
const char ERROR_EEROM[2][40] = {"������ EEPROM",  "EEPROM Error"};
const char SAVE_DATA_IZM[2][40] = {"����������...",  "SAVING..."};
const char ZAPISEY[2][40] = {"������� ",  "Log "};
const char SAVE_USB_DATA[2][40] = {"������ � ����... ",  "Filing..."};
const char USB_RES_ERR[2][40] = {"USB �� ������",  "USB not found"};
const char SEND_ONE_REQ[2][40] = {"�������� �� ����...",  "Uploading"};
const char CALIBRATE_24V_CORRECT[2][40] = {"�����. 24� �������",  "24V �alib. compl."};
const char YES[2][40] = {"��",  "YES"};
const char NO[2][40] = {"���",  "NO"};
const char FORMAT[2][40] = {"������������� ������?",  "Format memory?"};
const char CYCLE[2][40] = {"������� � ����� ����?",  "Format Flash?"};
const char ALL_RESET[2][40] = {"������ �����?",  "All reset?"};
const char RESET_ST[2][40] = {"��������?",  "Reset?"};
const char UPDATE[2][40] = {"����������..",  "Update.."};

// ���������� �������� � ������
menuSelect_item Communication_DATA = {
    .data = (uint8_t *)&EEPROM.Communication,
    .Name = {
        {"����.", "Off"},
        {"GSM", "GSM"},
        {"NB-IoT", "NB-IoT"}
    },
};
menuSelect_item RS485_MODE_DATA = {
    (uint8_t *)&EEPROM.RS485_prot,
    {
        {"����.", "Off"},
        {"���.", "On"}
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
    (uint8_t *)&EEPROM.mode_ADC,
    {
        {"4-20��", "4-20mA"},
        {"0-20��", "0-20mA"}
        //{"����.", "Off"}
    }
};

// ����� USB, - ������ � ������� flash (0), ������ � ���������� ������ (1), ������ USB � ������ ������ ���������� flash (2), ������ � ������ ������ SD ����� (3), ������ � ������ ������ USB (4)
menuSelect_item USB_MODE_STRUCT = {
    (uint8_t *)&EEPROM.USB_mode,
    {
        {"FLASH", "FLASH"},
        {"DEBUG", "DEBUG"},
        {"����.", "Off"}
    }
};

// ���� ��������� ������ �� ���� ���������
menuSelect_item SAVE_IN_STRUCT = {
    (uint8_t *)&EEPROM.Save_in,
    {
        {"������", "Device"},
        {"USB", "USB"}
        //{"����", "Website"}
    }
};

// ���������� ����������
menuSelect_item Block = {
    (uint8_t *)&EEPROM.block,
    {
        {"����������", "Unlocked"},
        {"��������", "Demo"},
        {"����������", "Locked"}
    }
};

// ���������� ����������
menuSelect_item Mode_select = {
    (uint8_t *)&EEPROM.Mode,
    {
        {"���������", "Current"},
        {"�����������", "Cyclic"},
        {"��������", "Exhibition"}
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
    (uint8_t *)&dat, { {"��", "mAmps"} }
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
    {255, 59, '\0'},            // ������������ ��������
    {0, 5, '\0'},               // ����������� ��������
    Save_time_sleep_format,         // ������ �� ������� ���������� ������
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
    {999999, 999, '\0'},              // ������������ ��������
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
    {'\0', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    1,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-���������
    '\0',
    //  ������
    {&EEPROM.version.VERSION_PCB, '\0', '\0'},  // �������� ��������
    {3, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                      // �������� ������ ��� �����������
    {10, 0, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {0, 0, 0},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {'\0', '\0', '\0'},              // ������������ ��������
    {'\0', '\0', '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    '\0'
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)

menuSelect_item_char Password = {
    {'\0', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    1,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-���������
    '\0',
    //  ������
    {&EEPROM.version.password, '\0', '\0'},  // �������� ��������
    {3, 0, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                      // �������� ������ ��� �����������
    {5, 0, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {0, 0, 0},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {'\0', '\0', '\0'},              // ������������ ��������
    {'\0', '\0', '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    '\0'
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)


menuSelect_item_char Temp_correct = {
    {'.', '\0', '\0'},   // �����������, ���� ����, �� '\0', ��������� ����� �������������� � �������� ��������
    2,                  // ����������� ����� ������
    0,                  // ���������� �������������� ��� ������� ������ �� ���� ������ 1-��������, 0-���������
    &ADC_data.Temp_correct_A,
    //  ������
    {&ADC_data.Temp_correct[0], &ADC_data.Temp_correct[1], '\0'},  // �������� ��������
    {2, 2, 0},                   // ��� ������ 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (��� ����� ������ - ������ ����� ��� �������������, � ������ ����� ����� �������� �������� ��������)
    {0, 0},                      // ���� (�������������)
    {5, 2, 0},                   // ������ ����� (001 - ������ 3, 23 - ������ 2)
    {2, 2, 0},                   // ������ ����� ��� �������������� (001 - ������ 3, 23 - ������ 2)
    {"\0", "\0", "\0"},          // ������������� ��������
    {99999, 999, '\0'},              // ������������ ��������
    {-99999, 0, '\0'},                // ����������� ��������
    Save_general_format,                    // ������ �� ������� ���������� ������
    SAVE_DOUBLE
}; // ������������� ����������, ���� ����������� ��������� �� ���������� (char)



// uint8_t dat = 0;
menuSelect_item NO_SIGNED = {
    (uint8_t *)&dat,
    {{"", ""}}
};

MAKE_MENU(Menu_1, "������", "Modes", 0, UPTADE_OFF, NO_SIGNED, Menu_2, PREVISION_MENU, PARENT_MENU, Menu_1_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_1_1, "����", "Cycle", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2, PREVISION_MENU, Menu_1, CHILD_MENU, sleep, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_1_2, "���������", "Sensor reading", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3, Menu_1_1, Menu_1, Menu_1_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_1_2_1, "�������", "Metering", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_2_2, PREVISION_MENU, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_char);
		MAKE_MENU(Menu_1_2_2, "���", "GWL", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_2_3, Menu_1_2_1, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_correct_char);
		MAKE_MENU(Menu_1_2_3, "���������", "Save", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_1_2_2, Menu_1_2, CHILD_MENU, SAVE_IZM, SELECT_BAR, DATA_IN, DATA_OUT); 
    MAKE_MENU(Menu_1_3, "�����������", "Test check", 0, UPTADE_ON, NO_SIGNED, NEXT_MENU, Menu_1_2, Menu_1, Menu_1_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, ERRCODE.Diagnostics_char); //! �������� ������
		MAKE_MENU(Menu_1_3_1, "������", "Status", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ERRCODE.STATUSCHAR);
        MAKE_MENU(Menu_1_3_2, "����.", "State", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3_3, Menu_1_3_1, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT); //! �������� ������
		MAKE_MENU(Menu_1_3_3, "���", "BAT", 0, UPTADE_OFF, Unit_voltage, Menu_1_3_4, Menu_1_3_2, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, IntADC.ADC_AKB_volts_char);
		MAKE_MENU(Menu_1_3_4, "�����", "Mobile network", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3_5, Menu_1_3_3, Menu_1_3, Menu_1_3_4_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_1, "���� ���", "SMS test", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3_4_2, PREVISION_MENU, Menu_1_3_4, CHILD_MENU, GSM_sms_test, SELECT_BAR, DATA_IN, GSM_data.GSM_sms_status);
			MAKE_MENU(Menu_1_3_4_2, "���� ����", "SITE test", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3_4_3, Menu_1_3_4_1, Menu_1_3_4, CHILD_MENU, GSM_internet_test, SELECT_BAR, DATA_IN, GSM_data.GSM_site_status);
			MAKE_MENU(Menu_1_3_4_3, "SIM-�����", "SIM-card", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_4, Menu_1_3_4_2, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_4, "�����", "Mode", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_5, Menu_1_3_4_3, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_5, "�����������", "Logging", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_6, Menu_1_3_4_4, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_6, "������", "Country", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_7, Menu_1_3_4_5, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_7, "��������", "Mobile operator", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_8, Menu_1_3_4_6, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_8, "������� ����.", "Signal level", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_4_9, Menu_1_3_4_7, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_3_4_9, "������", "Signal err", 0, UPTADE_ON, NO_SIGNED, NEXT_MENU, Menu_1_3_4_8, Menu_1_3_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_1_3_5, "���", "ADC", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_6, Menu_1_3_4, Menu_1_3, Menu_1_3_5_1, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_status_char);
			MAKE_MENU(Menu_1_3_5_1, "���", "ADC", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_5_2, PREVISION_MENU, Menu_1_3_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_value_char);
			MAKE_MENU(Menu_1_3_5_2, "�������", "Metering", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_3_5_3, Menu_1_3_5_1, Menu_1_3_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_char);
			MAKE_MENU(Menu_1_3_5_3, "���", "GWL", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_3_5_4, Menu_1_3_5_2, Menu_1_3_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_correct_char);
			MAKE_MENU(Menu_1_3_5_4, "��� �������", "Current", 0, UPTADE_ON, Unit_current, NEXT_MENU, Menu_1_3_5_3, Menu_1_3_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_Current_char);
		MAKE_MENU(Menu_1_3_6, "EEPROM", "EEPROM", 0, UPTADE_ON, NO_SIGNED, Menu_1_3_7, Menu_1_3_5, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM_status_char); 
		MAKE_MENU(Menu_1_3_7, "FLASH", "FLASH", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_1_3_6, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, FLASH_status_char); 
MAKE_MENU(Menu_2, "���������", "Settings", 0, UPTADE_OFF, NO_SIGNED, Menu_3, Menu_1, PARENT_MENU, Menu_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_1, "�������� �� USB", "USB write", 0, UPTADE_OFF, NO_SIGNED, Menu_2_2, PREVISION_MENU, Menu_2, CHILD_MENU, SAVE_USB, SELECT_BAR, DATA_IN, DATA_OUT); 
    MAKE_MENU(Menu_2_2, "�������������", "Sin", 0, UPTADE_OFF, NO_SIGNED, Menu_2_3, Menu_2_1, Menu_2, CHILD_MENU, GSM_HTTP_SYNC, SELECT_BAR, DATA_IN, GSM_data.GSM_site_read_status);
    MAKE_MENU(Menu_2_3, "����", "Date", 0, UPTADE_OFF, NO_SIGNED, Menu_2_4, Menu_2_2, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Date_redact, DATA_OUT);
	MAKE_MENU(Menu_2_4, "�����", "Time", 0, UPTADE_OFF, NO_SIGNED, Menu_2_5, Menu_2_3, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_redact, DATA_OUT);
	MAKE_MENU(Menu_2_5, "����� ���", "Seep time", 0, UPTADE_OFF, NO_SIGNED, Menu_2_6, Menu_2_4, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_sleep_redact, DATA_OUT);
	MAKE_MENU(Menu_2_6, "����. ���", "GWL correction", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, GVL_Correct, DATA_OUT);
	MAKE_MENU(Menu_2_7, "���", "U-range", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_8, Menu_2_6, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Max_Level_Mesurment, DATA_OUT);
	MAKE_MENU(Menu_2_8, "���", "L-range", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_9, Menu_2_7, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Min_Level_Mesurment, DATA_OUT);
	MAKE_MENU(Menu_2_9, "��. ���.", "Unit measure", 0, UPTADE_OFF, NO_SIGNED, Menu_2_10, Menu_2_8, Menu_2, CHILD_MENU, ACTION_MENU, UNITS_MODE_DATA, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_10, "������� �����", "Current loop ", 0, UPTADE_OFF, NO_SIGNED, Menu_2_11, Menu_2_9, Menu_2, CHILD_MENU, ACTION_MENU, CURRENT_LOOP, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_11, "RS-485", "RS-485", 0, UPTADE_OFF, NO_SIGNED, Menu_2_12, Menu_2_10, Menu_2, CHILD_MENU, ACTION_MENU, RS485_MODE_DATA, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_12, "�����", "Network", 0, UPTADE_OFF, NO_SIGNED, Menu_2_13, Menu_2_11, Menu_2, CHILD_MENU, ACTION_MENU, Communication_DATA, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_13, "��������� �", "Save in", 0, UPTADE_OFF, NO_SIGNED, Menu_2_14, Menu_2_12, Menu_2, CHILD_MENU, ACTION_MENU, SAVE_IN_STRUCT, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_14, "����", "Language", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15, Menu_2_13, Menu_2, CHILD_MENU, ACTION_MENU, LANGUAGE, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_15, "���. ����", "Administration menu", 0, UPTADE_OFF, NO_SIGNED, Menu_2_16, Menu_2_14, Menu_2, Menu_2_15_1, PASSWORD, SELECT_BAR, DATA_IN, DATA_OUT); 
		MAKE_MENU(Menu_2_15_1, "��������", "Startup screen", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_2, PREVISION_MENU, Menu_2_15, CHILD_MENU, ACTION_MENU, SCREENSAVER, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_2, "����� USB", "USB mode", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_3, Menu_2_15_1, Menu_2_15, CHILD_MENU, ACTION_MENU, USB_MODE_STRUCT, DATA_IN, DATA_OUT);
        MAKE_MENU(Menu_2_15_3, "���. ���.", "Ser. Number", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_4, Menu_2_15_2, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Serial_number, DATA_OUT);
		MAKE_MENU(Menu_2_15_4, "������", "Password", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_5, Menu_2_15_3, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Password, DATA_OUT);
		MAKE_MENU(Menu_2_15_5, "����. CR", "Plug", 0, UPTADE_ON, NO_SIGNED, Menu_2_15_6, Menu_2_15_4, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, IntADC.MK_vbat_char);
		MAKE_MENU(Menu_2_15_6, "���������� 24�", "Calibration 24V", 0, UPTADE_ON, NO_SIGNED, Menu_2_15_7, Menu_2_15_5, Menu_2_15, CHILD_MENU, colibrate_24v, SELECT_BAR, DATA_IN, IntADC.ADC_AKB_volts_char);
		MAKE_MENU(Menu_2_15_7, "���������� 20��", "Calibration(up)", 0, UPTADE_ON, NO_SIGNED, Menu_2_15_8, Menu_2_15_6, Menu_2_15, CHILD_MENU, colibrate_20ma, SELECT_BAR, DATA_IN, ADC_data.ADC_Current_char);
		MAKE_MENU(Menu_2_15_8, "���������� 4��", "Calibration(low)", 0, UPTADE_ON, NO_SIGNED, Menu_2_15_9, Menu_2_15_7, Menu_2_15, CHILD_MENU, colibrate_4ma, SELECT_BAR, DATA_IN, ADC_data.ADC_Current_char);
		MAKE_MENU(Menu_2_15_9, "����. ����.", "Offset temperature", 0, UPTADE_OFF, Unit_degree, Menu_2_15_10, Menu_2_15_8, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Temp_correct, DATA_OUT);
		MAKE_MENU(Menu_2_15_10, "����. ����.", "Analog temp.", 0, UPTADE_ON, Unit_degree, Menu_2_15_11, Menu_2_15_9, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_MS5193T_temp_char);
		MAKE_MENU(Menu_2_15_11, "����. ���.", "Digital temp.", 0, UPTADE_ON, Unit_degree, Menu_2_15_12, Menu_2_15_10, Menu_2_15, CHILD_MENU, SELECT_BAR, SELECT_BAR, DATA_IN, IntADC.MK_temp_char);
		MAKE_MENU(Menu_2_15_12, "���� FLASH", "FLASH test", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_13, Menu_2_15_11, Menu_2_15, CHILD_MENU, flash_test, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_13, "���� EEPROM", "EEPROM test", 0, 0, NO_SIGNED, Menu_2_15_14, Menu_2_15_12, Menu_2_15, CHILD_MENU, EEPROM_test, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_14, "�����", "Mode", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_15, Menu_2_15_13, Menu_2_15, CHILD_MENU, SELECT_BAR, Block, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_15, "������ �����", "FULL RESET", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_2_15_14, Menu_2_15, CHILD_MENU, ALL_Reset_settings, SELECT_BAR, DATA_IN, DATA_OUT);
    MAKE_MENU(Menu_2_16, "���������� ��", "Update", 0, UPTADE_OFF, NO_SIGNED, Menu_2_17, Menu_2_15, Menu_2, CHILD_MENU, Update_programm, SELECT_BAR, DATA_IN, DATA_OUT); 
    MAKE_MENU(Menu_2_17, "������. ������", "Format device", 0, UPTADE_OFF, NO_SIGNED, Menu_2_18, Menu_2_16, Menu_2, CHILD_MENU, Flash_Format, SELECT_BAR, DATA_IN, DATA_OUT); 
	MAKE_MENU(Menu_2_18, "����� ��������", "Factory reset", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_2_17, Menu_2, CHILD_MENU, Reset_settings, SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_3, "��������", "Info", 0, UPTADE_OFF, NO_SIGNED, Menu_4, Menu_2, PARENT_MENU, Menu_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_3_1, "���. ���.", "Ser. Number", 0, UPTADE_OFF, NO_SIGNED, Menu_3_2, PREVISION_MENU, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM.version.VERSION_PCB);
	MAKE_MENU(Menu_3_2, "���. ��", "Software version", 0, UPTADE_OFF, NO_SIGNED, Menu_3_3, Menu_3_1, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, VERSION_PROGRAMM);
	MAKE_MENU(Menu_3_3, "���������", "Operating time", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_3_2, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, time_work_char);
MAKE_MENU(Menu_4, "����������", "Instruction", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_3, PARENT_MENU, CHILD_MENU, Instruction, SELECT_BAR, DATA_IN, DATA_OUT);

void Add_units(void)
{
    // ������ ����������� ��������� �� ������, ���������� �� GSM_data
    Menu_1_2_1.add_signat   = &UNITS_MODE_DATA;       // ������� �����
    Menu_1_2_2.add_signat = &UNITS_MODE_DATA;
    Menu_1_3_5_2.add_signat = &UNITS_MODE_DATA;
    Menu_1_3_5_3.add_signat = &UNITS_MODE_DATA;
}

void Remove_units(void)
{
    // ����������� ��������� �� ������, ���������� �� GSM_data
    Menu_1_2_1.add_signat   = &NO_SIGNED;       // ������� �����
    Menu_1_2_2.add_signat = &NO_SIGNED;
    Menu_1_3_5_2.add_signat = &NO_SIGNED;
    Menu_1_3_5_3.add_signat = &NO_SIGNED;
}

void InitMenus(void)
{
    // ���������� ������
    Menu_1_3_4.data_out   = GSM_data.GSM_status_char;       // ������� �����
    Menu_1_3_4_3.data_out = GSM_data.GSM_SIMCARD_char;
    Menu_1_3_4_4.data_out = GSM_data.Modem_mode;
    Menu_1_3_4_5.data_out = GSM_data.GSM_status_reg_char;
    Menu_1_3_4_6.data_out = GSM_data.GSM_region_char;
    Menu_1_3_4_7.data_out = GSM_data.GSM_operator_char;
    Menu_1_3_4_8.data_out = GSM_data.GSM_signal_lvl_char;
    Menu_1_3_4_9.data_out = GSM_data.GSM_err_lvl_char;

}

void State_update(){
    uint64_t state = 0;
    state =  (uint64_t)(EEPROM.Mode & 0x0F) << 4*0;
    state |= (uint64_t)(EEPROM.Communication & 0x0F) << 4*1;
    state |= (uint64_t)(EEPROM.RS485_prot & 0x0F) << 4*2;
    state |= (uint64_t)(EEPROM.units_mes & 0x0F) << 4*3;
    state |= (uint64_t)(EEPROM.screen_sever_mode & 0x0F) << 4*4;
    state |= (uint64_t)(EEPROM.USB_mode & 0x0F) << 4*5;
    state |= (uint64_t)(EEPROM.Save_in & 0x0F) << 4*6;
    state |= (uint64_t)(EEPROM.len & 0x0F) << 4*7;
    state |= (uint64_t)(EEPROM.mode_ADC & 0x0F) << 4*8;
    state |= (uint64_t)(EEPROM.block & 0x0F) << 4*9;
    base62_encode(state, ERRCODE.STATE_CAHAR, sizeof(ERRCODE.STATE_CAHAR));
    Menu_1_3_2.data_out = ERRCODE.STATE_CAHAR;        // ������� ��������� (������ � �.�.)
}

////////////////////////////////////////////////////
//                ��������� ����                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

////////////////////////////////////////////////////
//                  ������ ����                   //
////////////////////////////////////////////////////

#define SCROLLBAR_X      (winth_display - 7)  // ������ �� ������� ���� ������
#define SCROLLBAR_Y      14                   // Y-���������� �������� ���� ������� ������
#define SCROLLBAR_HEIGHT 45                   // ������ ������� ������ ���������
#define SCROLLBAR_WIDTH  3                    // ������ ������ ���������
#define MIN_THUMB_HEIGHT 3                    // ����������� ������ ��������

void DrawScrollBarForSection(void) {
    // ���������� ������ �������� ������ (0, ���� ��� ������ �������)
    uint8_t currentIndex = 0;
    menuItem *p = selectedMenuItem;
    for (uint8_t i = 0; i < 32; i++) {  // ����������� � 32 ��������
        if (p->Previous == (void *)&NULL_ENTRY)
            break;
        currentIndex++;
        p = (menuItem *)p->Previous;
    }
    // p ������ ��������� �� ������ ������� �������
    menuItem *firstItem = p;
    
    // ���������� ����� ���������� ������� � �������
    uint8_t totalItems = 0;
    p = firstItem;
    for (uint8_t i = 0; i < 32; i++) {  // ����������� � 32 ��������
        if (p == (menuItem *)&NULL_ENTRY)
            break;
        totalItems++;
        p = (menuItem *)p->Next;
    }
    
    // ������ ����� ������ ���������
    OLED_DrawRectangle(SCROLLBAR_X, SCROLLBAR_Y, SCROLLBAR_X + SCROLLBAR_WIDTH, SCROLLBAR_Y + SCROLLBAR_HEIGHT);
    
    // ���� � ������� ������ ���� �����, ��������� ��� ������� ������
    if (totalItems <= 1) {
        OLED_DrawRectangleFill(SCROLLBAR_X + 1, SCROLLBAR_Y + 1, SCROLLBAR_X + SCROLLBAR_WIDTH - 1, SCROLLBAR_Y + SCROLLBAR_HEIGHT - 1, 1);
    } else {
        // ������ �������� �������������� ��� (SCROLLBAR_HEIGHT / totalItems)
        uint8_t thumbHeight = SCROLLBAR_HEIGHT / totalItems;
        if (thumbHeight < MIN_THUMB_HEIGHT)
            thumbHeight = MIN_THUMB_HEIGHT;
        
        // ������������ �������� �������� �� ���������
        uint8_t maxThumbPos = SCROLLBAR_HEIGHT - thumbHeight;
        // ������������ ��������: ����� currentIndex = 0, thumbY = SCROLLBAR_Y; ����� currentIndex = totalItems-1, thumbY = SCROLLBAR_Y + maxThumbPos.
        uint8_t thumbY = SCROLLBAR_Y + (currentIndex * maxThumbPos) / (totalItems - 1);
        
        // ������ ����������� ��������
        OLED_DrawRectangleFill(SCROLLBAR_X + 1, thumbY, SCROLLBAR_X + SCROLLBAR_WIDTH - 1, thumbY + thumbHeight, 1);
    }
}


void PROGRESS_BAR(uint8_t procent){
    #define Y 25
    #define leight 80
    #define first_point (winth_display-leight)/2
    uint8_t procent_len = (float)((float)leight/100)*procent;
    OLED_DrawRectangle(first_point, Y+15, leight+first_point, Y+25);
    OLED_DrawRectangleFill(first_point, Y+15, procent_len+first_point, Y+25, 1);
}
void OLED_DrawCenteredString(const char strArray[][40], uint16_t Y_pos) {
    uint8_t len = OLED_GetWidthStr(strArray[EEPROM.len]);
    uint16_t x = (winth_display - len) / 2;
    OLED_DrawStr(strArray[EEPROM.len], x, Y_pos, 1);
}
// ������ ���� ��� ������� ������
void full_test(){}

void USB_FLASH_SAVE(){}

void flash_test(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    osThreadSuspend(ADC_readHandle);
    osThreadSuspend(ERROR_INDICATE_taskHandle);
    osDelay(200);

    __HAL_SPI_DISABLE(&hspi2);  // ��������� ���������
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();  // ��� ���� ��������� ������
    }
    __HAL_SPI_ENABLE(&hspi2);   // �������� �������

    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO("����������� FLASH?") == 0)
    {
        mode_redact = 0;
        return;
    }

    OLED_Clear(0);
    OLED_DrawCenteredString(Clear, 10);
    OLED_DrawCenteredString(Divece, 20);
    OLED_DrawCenteredString(POWER_NOT, Y + 20);
    OLED_UpdateScreen();
    W25_Chip_Erase();

    OLED_Clear(0);
    OLED_DrawCenteredString("������������ FLASH", 10);
    OLED_DrawCenteredString(POWER_NOT, Y);
    PROGRESS_BAR(0);
    OLED_UpdateScreen();

    uint32_t errors = W25_Chip_test();
    OLED_Clear(0);
    OLED_DrawCenteredString(Clear, 10);
    OLED_DrawCenteredString(Divece, 20);
    OLED_DrawCenteredString(POWER_NOT, Y + 20);
    OLED_UpdateScreen();
    W25_Chip_Erase();

    if (errors == 0xFFFFFFFF){
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ��� ��������", Y+10);
        OLED_UpdateScreen();
    }
    if (errors == 0)
    {
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ��������", Y+10);
        OLED_UpdateScreen();
    }
    if ((errors != 0) && (errors != 0xFFFFFFFF))
    {
        OLED_Clear(0);
        char error_str[30] = {0};
        snprintf(error_str, sizeof(error_str), "������: %lu", errors);
        OLED_DrawCenteredString(error_str, Y);
        OLED_UpdateScreen();
    }
    __HAL_SPI_DISABLE(&hspi2);  // ��������� ���������
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();  // ��� ���� ��������� ������
    }
    __HAL_SPI_ENABLE(&hspi2);   // �������� �������

    osThreadResume(ADC_readHandle);
    osThreadResume(ERROR_INDICATE_taskHandle);
    osDelay(200);
}
void EEPROM_test(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);

    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO("����������� EEPROM?") == 0)
    {
        mode_redact = 0;
        return;
    }
    OLED_Clear(0);
    OLED_DrawCenteredString("���� ������������", 10);
    OLED_UpdateScreen();
    uint32_t errors = EEPROM_Test();
    if (errors == 0xFFFFFFFF){
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ��� ��������", Y+10);
        OLED_UpdateScreen();
    }
    if (errors == 0)
    {
        OLED_Clear(0);
        OLED_DrawCenteredString("������ ��������", Y+10);
        OLED_UpdateScreen();
    }
    if ((errors != 0) && (errors != 0xFFFFFFFF))
    {
        OLED_Clear(0);
        char error_str[30] = {0};
        snprintf(error_str, sizeof(error_str), "������: %lu", errors);
        OLED_DrawCenteredString(error_str, Y);
        OLED_UpdateScreen();
    }
    osDelay(200);
}


void Update_programm(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 30


    osThreadSuspend(ADC_readHandle);
    osThreadSuspend(ERROR_INDICATE_taskHandle);
    OLED_Clear(0);
    Display_TopBar(selectedMenuItem);
    Update_PO();

    osThreadResume(ADC_readHandle);
    osThreadResume(ERROR_INDICATE_taskHandle);

    osDelay(200);
}

// ��������� ���������, ������ ���� ������� ���������� ������ 
void SAVE_IZM(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define Y 25

    //Collect_DATA();

    if (EEPROM.Save_in == 0)
    {
        Collect_DATA();
        OLED_DrawCenteredString(SAVE_DATA_IZM, Y);
        OLED_UpdateScreen();
        int res = flash_append_record(save_data, 1);
        if (res == -1)
        {
            OLED_DrawCenteredString(ERROR_TEXT, Y);
            OLED_UpdateScreen();
            return;
        }
        g_total_records_count++;
        char flash_end_ptr_char[30] = {0};
        snprintf(flash_end_ptr_char, 30, "%s %ld", ZAPISEY[EEPROM.len], g_total_records_count);
        OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
        OLED_DrawCenteredString(READY, Y);
    
        uint8_t len = OLED_GetWidthStr(flash_end_ptr_char);
        uint16_t x = (winth_display - len) / 2;
        OLED_DrawStr(flash_end_ptr_char, x, Y+10, 1);
        //OLED_DrawStr(save_data, X-15, Y+30, 1);
        OLED_UpdateScreen();
        osDelay(200);
    }
    if (EEPROM.Save_in == 1){
        OLED_DrawCenteredString(SAVE_DATA_IZM, Y);
        OLED_UpdateScreen();
        int res = Save_one_to_USB();
        OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
        if (res == -1){
            OLED_DrawCenteredString(ERROR_TEXT, Y);
            OLED_UpdateScreen();
            return;
        }
        OLED_DrawCenteredString(READY, Y);
    }
    if (EEPROM.Save_in == 2){
        OLED_DrawCenteredString(SEND_ONE_REQ, Y);
        OLED_UpdateScreen();
        // ���������� �� ����
        GSM_data.Status |= HTTP_SEND;
        OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
        TickType_t start_tick = xTaskGetTickCount();
        const TickType_t timeout = pdMS_TO_TICKS(20000); // 20 ������
        const TickType_t delay_interval = pdMS_TO_TICKS(200); // ��������� ������ 200 ��
        while ((xTaskGetTickCount() - start_tick) < timeout)
        {
            // ���� �������� ���� ������ UART ��� ���� �������� �������� HTTP, ������� �� �����
            if (ERRCODE.STATUS & STATUS_HTTP_SERVER_COMM_ERROR){
                GSM_data.Status &= ~STATUS_HTTP_SERVER_COMM_ERROR;
                OLED_DrawCenteredString(ERROR_TEXT, Y);
                OLED_UpdateScreen();
                return;
            }
            if (GSM_data.Status & HTTP_SEND_Successfully)
            {
                GSM_data.Status &= ~HTTP_SEND_Successfully;
                OLED_DrawCenteredString(READY, Y);
                OLED_UpdateScreen();
                return;
            }
            vTaskDelay(delay_interval);
        }
        OLED_DrawCenteredString(ERROR_TEXT, Y);
        OLED_UpdateScreen();
        return;
    }
}

void SAVE_USB(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 25

    OLED_DrawCenteredString(SAVE_USB_DATA, Y);
    OLED_UpdateScreen();

    osThreadSuspend(ADC_readHandle);
    osThreadSuspend(ERROR_INDICATE_taskHandle);
    HAL_SPI_DeInit(&hspi2);
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; // ����� �������� ��������
    HAL_SPI_Init(&hspi2);

    int res = backup_records_to_external();
    OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
    if (res != 1) OLED_DrawCenteredString(USB_RES_ERR, Y);
    else OLED_DrawCenteredString(READY, Y);

    HAL_SPI_DeInit(&hspi2);
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; // ����� �������� ��������
    HAL_SPI_Init(&hspi2);
    osThreadResume(ADC_readHandle);
    osThreadResume(ERROR_INDICATE_taskHandle);

    OLED_UpdateScreen();
    osDelay(200);
}

void Screen_saver(){
    mode_redact = 4;
    osDelay(200);
    OLED_Clear(0);
    OLED_DrawXBM(0, 0, ScreenSaver_Static);
    OLED_UpdateScreen();
}

void PASSWORD(){
    index_pass = 0;
    pass[0] = 0;
    pass[1] = 0;
    pass[2] = 0;
    pass[3] = 0;
    pass[4] = 0;
    mode_redact = 3;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    OLED_DrawCenteredString(PASSWORD_IN, 25);
    OLED_UpdateScreen();
    osDelay(200);
}


// ������� �� ����� ������ �� ��� ���. 1 -�� 0 - ���
int YES_OR_NO(const char strArray[][40]){
    Keyboard_press_code = 0xFF;
    mode_redact = 5;
    int YES_NO = 0; // ���
    uint8_t update = 1;
    while (Keyboard_press_code != 'O'){

        if (Keyboard_press_code == 'L'){
            YES_NO = 1;
            update = 1;
        }
        if (Keyboard_press_code == 'R'){
            YES_NO = 0;
            update = 1;
        }
        if (update == 1){
            update = 0;
            OLED_DrawCenteredString(strArray, 20);
            OLED_DrawStr(YES[EEPROM.len], 35, 30, YES_NO^1);
            OLED_DrawStr(NO[EEPROM.len], 76, 30, YES_NO);
            OLED_UpdateScreen();
        }
        Keyboard_press_code = 0xFF;
        osDelay(50);
    }

    mode_redact = 2;
    OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
    return YES_NO;
}

// ���������� ��. 
// 1. �������� USB, ���� ���� �� �������� ����
// 2. ���� USB ����, ������ ���� �������� ����� � �����
// 3. ������������ � ������ Boot
void Update(){
    

}

void Flash_Format(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO(FORMAT) == 0){ mode_redact = 0; return;}

    OLED_DrawCenteredString(Clear, Y);
    OLED_DrawCenteredString(Divece, Y+10);
    OLED_DrawCenteredString(POWER_NOT, Y+20);
    OLED_UpdateScreen();

    W25_Chip_Erase();

    OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
    OLED_DrawCenteredString(READY, Y);
    OLED_UpdateScreen();
    osDelay(200);
}


// ���� 
void GSM_internet_test(){
    if (EEPROM.Communication == M2M_DISABLE) return;
    strcpy(GSM_data.GSM_site_status, ". . .");
    OLED_UpdateScreen();
    GSM_data.Status |= HTTP_SEND;
}
void GSM_sms_test(){
    if (EEPROM.Communication == M2M_DISABLE) return;
    strcpy(GSM_data.GSM_sms_status, ". . .");
    OLED_UpdateScreen();
    GSM_data.Status |= SMS_SEND;
}
void GSM_HTTP_SYNC(){
    if (EEPROM.Communication == M2M_DISABLE) return;
    strcpy(GSM_data.GSM_site_read_status, ". . .");
    OLED_UpdateScreen();
    GSM_data.Status |= HTTP_READ;
}

// ���������� �� 
void Programm_Update_USB(){}

void sleep(){
    // ���������� ������� 
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO(CYCLE) == 0){ mode_redact = 0; return;}

    // ������ �������� � �����
    //EEPROM.Mode = 1;
    EEPROM_SaveSettings(&EEPROM);
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
    HAL_PWR_DisableBkUpAccess();
    NVIC_SystemReset();
    osDelay(1000);
}


extern EEPROM_Settings_item EEPROM;
void Reset_settings(){
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO(RESET_ST) == 0)
    {
        mode_redact = 0;
        return;
    }
    ERRCODE.STATUS = 0;
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, 0);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, 0);
    HAL_PWR_DisableBkUpAccess();

    EEPROM.time_sleep_h = DEFAULT_TIME_SLEEP_H;           // ����� ��� ���������� (����)
    EEPROM.time_sleep_m = DEFAULT_TIME_SLEEP_M;           // ����� ��� ���������� (������)
    EEPROM.MAX_LVL = DEFAULT_MAX_LVL;                     // ������������ ������� (��������, 15 ������) ���
    EEPROM.ZERO_LVL = DEFAULT_ZERO_LVL;                   // ������� �������� (��������, 0 ������) ���
    EEPROM.Mode = DEFAULT_MODE;                           // ������� ����� ������
    EEPROM.Communication = GSM_MODE;                      // ������� GSM ��� ���
    EEPROM.RS485_prot = DEFAULT_RS485_PROT;               // �������� RS-485
    EEPROM.units_mes = DEFAULT_UNITS_MES;                 // ������� ��������� (�� ��������� �����)
    //EEPROM.USB_mode = DEFAULT_USB_MODE;                   // ����� ������ USB
    EEPROM.len = DEFAULT_LEN;                             // ���� ����
    EEPROM.mode_ADC = DEFAULT_MODE_ADC;                   // ����� ������ ���, 0 - 4-20��, 1 - 0-20��, 2 - ����
    EEPROM_SaveSettings(&EEPROM);

    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define Y 33
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawCenteredString(ERROR, Y);
    }
    else OLED_DrawCenteredString(READY, Y);
    OLED_UpdateScreen();
    osDelay(200);
}

void ALL_Reset_settings(){
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if (YES_OR_NO(ALL_RESET) == 0){ mode_redact = 0; return;}
    EEPROM_Settings_item EEPROM_RESET = {
        .version = {
            // ������� ������ ����������
            .VERSION_PCB = DEFAULT_VERSION_PCB, // ������ �������� �����
            .password = DEFAULT_PASSWORD,
        },
        .last_error_code = DEFAULT_LAST_ERROR_CODE, // ��������� ��� ������

        // �������� ������:
        .time_sleep_h = DEFAULT_TIME_SLEEP_H, // ����� ��� ���������� (����)
        .time_sleep_m = DEFAULT_TIME_SLEEP_M, // ����� ��� ���������� (������)
        .DEBUG_CATEG = DEBUG_NONE,
        .DEBUG_LEVL = DEBUG_LEVL_1,
        .DEBUG_Mode = USB_SNIFFING,
        .Phone = DEFAULT_PHONE,

        // ��������� ���:
        .ADC_ION = DEFAULT_ADC_ION,                 // ���������� ��� ���
        .ADC_RESISTOR = DEFAULT_ADC_RESISTOR,       // ������������� ���������
        .GVL_correct = DEFAULT_GVL_CORRECT,         // ��������� ������� ����� (�������� � �� �������� ��������) ���
        .k_koeff = DEFAULT_K_KOEFF,                 // ����������� ������� �������� ����������� (�� 2 ������, 20�� � 4��)
        .MAX_LVL = DEFAULT_MAX_LVL,                 // ������������ ������� (��������, 15 ������) ���
        .ZERO_LVL = DEFAULT_ZERO_LVL,               // ������� �������� (��������, 0 ������) ���
        .GVL_correct_4m = DEFAULT_GVL_CORRECT_4M,   // �������� 4��
        .GVL_correct_20m = DEFAULT_GVL_CORRECT_20M, // �������� 20��
        // ��������� ����������� (��������):
        .Crorrect_TEMP_A = DEFAULT_CRORRECT_TEMP_A, // �������� ������� ���������� �����������
        .Colibrate_koeff = DEFAULT_COLIBRATE_KOEFF, // ����������� ���������� 24 �����

        // ��������� select_bar:
        .Mode = DEFAULT_MODE,                           // ������� ����� ������ (0 - ����� ������� ���������, 1 - ����������� �����, 2 - ����� ��������)
        .Communication = DEFAULT_COMMUNICATION,         // ������� GSM ��� ���
        .RS485_prot = DEFAULT_RS485_PROT,               // �������� RS-485
        .units_mes = DEFAULT_UNITS_MES,                 // ������� ��������� (�� ��������� �����)
        .screen_sever_mode = DEFAULT_SCREEN_SEVER_MODE, // �������� ��� ��� �������� ��� ���������
        .USB_mode = DEFAULT_USB_MODE,                   // ����� ������ USB
        .Save_in = DEFAULT_SAVE_IN,                     // ���� ��������� ������: 0 - FLASH, 1 - SD, 2 - USB, 3 - ����
        .len = DEFAULT_LEN,                             // ���� ����
        .mode_ADC = DEFAULT_MODE_ADC,                   // ����� ������ ���, 0 - 4-20��, 1 - 0-20��, 2 - ����
        .block = DEFAULT_BLOCK                          // ���������� ����������: 1 - �������������, 0 - ��������������

    };
    HAL_StatusTypeDef status_1 = EEPROM_SaveSettings(&EEPROM_RESET);
    HAL_StatusTypeDef status_2 = EEPROM_ClearBuffer();
    if (status_1 != HAL_OK || status_2 != HAL_OK) {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define Y 33
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawCenteredString(ERROR_TEXT, Y);
    }
    else OLED_DrawCenteredString(READY, Y);
    OLED_UpdateScreen();
    osDelay(2000);
    EEPROM_clear_time_init();
    
    NVIC_SystemReset();

}


#define X_col 25
#define Y_col 20
// ! ����� �������� �� �����������������
void colibrate_4ma(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if ((ADC_data.ADC_Current < 0.0035) || (ADC_data.ADC_Current > 0.0045)){
        // ������
        OLED_DrawCenteredString(DIAPASON_ERR, Y_col);
        OLED_UpdateScreen();
        return;
    }
    EEPROM.GVL_correct_4m = ADC_data.ADC_Current;
    OLED_DrawCenteredString(CALIBRATE_4ma_CORRECT, Y_col);

    EEPROM_SaveSettings(&EEPROM);
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawCenteredString(ERROR_EEROM, Y_col+10);
    }

    OLED_DrawCenteredString(READY, Y_col+10);
    OLED_UpdateScreen();
    osDelay(200);
}
void colibrate_24v(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    
    int res = Read_ADC_Colibrate_24V();
    if (res == -1){
        // ������
        OLED_DrawCenteredString(ERROR_TEXT, Y_col);
        OLED_UpdateScreen();
        return;
    }
    OLED_DrawCenteredString(READY, Y_col);
    OLED_UpdateScreen();
    osDelay(500);
    mode_redact = 0;
}



void colibrate_20ma(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if ((ADC_data.ADC_Current < 0.019) || (ADC_data.ADC_Current > 0.025)){
        // ������
        OLED_DrawCenteredString(DIAPASON_ERR, Y_col);
        OLED_UpdateScreen();
        return;
    }
    EEPROM.GVL_correct_20m = ADC_data.ADC_Current;
    OLED_DrawCenteredString(CALIBRATE_20ma_CORRECT, Y_col);

    EEPROM_SaveSettings(&EEPROM);
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawCenteredString(ERROR_EEROM, Y_col+10);
    }
    OLED_DrawCenteredString(READY, Y_col+20);
    OLED_UpdateScreen();
    osDelay(200);
}

// ! ����� �������� �� �����������������
void temperature_colibrate(){

}

// ����������� ���� � ���������
void Instruction(){
    mode_redact = 2;
    OLED_Clear(0);
    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);
    OLED_DrawXBM(10, 23, QR);

    #define X 50
    #define Y 28
    OLED_DrawStr("����������", X, Y, 1);
    OLED_DrawStr("�� QR-����", X, Y+10, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

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
        memset(buffer, 0, sizeof(buffer));
        formatters[menu->data_in->data_type[i]](buffer, sizeof(buffer), menu->data_in->data[i]);
        if (menu->data_in->data_type[i] == 3) remove_braces_inplace(buffer);
        // �������� ���� ����� ��������
        if (menu->data_in->data_type[i] != 3) 
        {
            uint8_t len_dat = search_len_mass(buffer, 11, separat);
            for (int j = 0; j < (uint8_t)menu->data_in->len_data_zero_unredact[i] - len_dat; j++) separat[j] = '0';
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
    if (menu->data_in->data_type[0] == 2)
    if (menu->data_in->unsigned_signed[1] == 1) string[0] = '-'; 
    else string[0] = '+';
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
        char buffer[30] = {0};
        strncpy(buffer, menu->data_out, sizeof(buffer)-1);
        remove_braces_inplace(buffer);
        uint8_t len_add_signa = 0;
        len_add_signa = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
        OLED_DrawStr(buffer, winth_display - (OLED_GetWidthStr(buffer)) - (len_add_signa+4), pos_y * dist_y + height_up_menu, 1);
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

static void DrawBack(uint8_t px, menuItem *m)
{
    if ((mode_redact == 2) || (mode_redact == 3))
    {
        OLED_DrawTriangleFill(px, back_pic_pos_y, px + size_back_pic_x, back_pic_pos_y + size_back_pic_y, px + size_back_pic_x, back_pic_pos_y - size_back_pic_y);
        OLED_DrawPixel(px + 1, back_pic_pos_y);
        char string[20];
        if (EEPROM.len == 0) strcpy(string, m->Name_rus);
        if (EEPROM.len == 1) strcpy(string, m->Name_en);
        string[12] = '\0';
        OLED_DrawStr(string, left_pic_last_munu, top_pic_last_munu, 1);
        return;
    }
    if (m->Parent != (void *)&NULL_ENTRY)
    {
        OLED_DrawTriangleFill(px, back_pic_pos_y, px + size_back_pic_x, back_pic_pos_y + size_back_pic_y, px + size_back_pic_x, back_pic_pos_y - size_back_pic_y);
        OLED_DrawPixel(px + 1, back_pic_pos_y);
        char string[20];
        if (EEPROM.len == 0) strcpy(string, ((menuItem *)m->Parent)->Name_rus);
        if (EEPROM.len == 1) strcpy(string, ((menuItem *)m->Parent)->Name_en);
        string[12] = '\0';
        OLED_DrawStr(string, left_pic_last_munu, top_pic_last_munu, 1);
    }
}

static void DrawBattery(uint8_t *px, menuItem *m)
{
    sprintf(str, "%d", IntADC.ADC_AKB_Proc);
    if (IntADC.ADC_AKB_Proc < 10)
    {
        str[1] = '%';
        str[2] = '\0';
    }
    else if (IntADC.ADC_AKB_Proc < 100)
    {
        str[2] = '%';
        str[3] = '\0';
        *px -= 6;
    }
    else
    {
        str[3] = '%';
        str[4] = '\0';
        *px -= 12;
    }

    OLED_DrawStr(str, *px, top_akb_status + 1, 1);
    *px -= width_akb_status;
    
    int c = IntADC.ADC_AKB_Proc * 5 / 100 + 1;
    if (IntADC.ADC_AKB_Proc == 0)
        c = 0;
    for (; c > 0; c--)
    {
        OLED_DrawHLine(*px + 2, top_akb_status + 8 - c, 2, 1);
    }
    OLED_DrawXBM(*px, top_akb_status, akb);
}
static void DrawGSM(uint8_t *px, menuItem *m)
{
    if (EEPROM.Communication != M2M_DISABLE)
    {
        if (GSM_data.GSM_Signal_Level_3 == -2)
        {
            *px -= 7;
            OLED_DrawStr("SIM?\0", *px, top_GSM_status, 1);
            *px -= 8;
        }
        if (GSM_data.GSM_Signal_Level_3 == -1)
        {
            *px += 3;
            OLED_DrawXBM(*px, top_GSM_status, no_signal);
            *px -= 4;
        }

        const uint8_t *signal_icons[] = {signal_0, signal_1, signal_2, signal_3};
        // GSM_data.GSM_Signal_Level = 3;
        if (GSM_data.GSM_Signal_Level_3 >= 0 && GSM_data.GSM_Signal_Level_3 <= 3)
        {
            OLED_DrawXBM(*px, top_GSM_status, signal_icons[GSM_data.GSM_Signal_Level_3]);
            *px -= 8;
        }
    }
}

static void DrawUSB(uint8_t *px, menuItem *m)
{
    if (Appli_state == APPLICATION_READY)
    {
        OLED_DrawXBM(*px, top_akb_status, USB_XMB);
        *px -= 8;
    }
    
}
static void DrawCR(uint8_t *px, menuItem *m)
{
    if (EEPROM.Communication != M2M_DISABLE)
    {
        if ( ERRCODE.STATUS & STATUS_VBAT_LOW)
        {
            OLED_DrawXBM(*px, top_GSM_status, CR_XMB);
            *px -= 8;
        }
    }
}

void Display_TopBar(menuItem *CurrentMenu)
{
    right_ot = winth_display - 12 - 2;
    OLED_DrawHLine(line_indentation, line_ind_top, end_line, 1);
    DrawBack(back_pic_pos_x, CurrentMenu);
    DrawBattery(&right_ot, CurrentMenu);
    right_ot -= width_GSM_status;
    DrawGSM(&right_ot, CurrentMenu);
    DrawUSB(&right_ot, CurrentMenu);
    DrawCR(&right_ot, CurrentMenu); 
}

uint8_t speed_update = 0; // ���� �� �� �������� ������ ���� ��������� �������� ����������? (100��)
// ����������� ���� ������� ���� �� ��������
void Display_all_menu()
{
    if (mode_redact == 4) return;
    if ((mode_redact != 2) && (mode_redact != 3)) OLED_Clear(0);
    else OLED_DrawRectangleFill(0, 0, winth_display, 11, 0);
    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));
    speed_update = 0;

    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);
    if ((mode_redact == 2) || (mode_redact == 3)){
        OLED_UpdateScreen();
        return;
    }
    //DrawScrollBarForSection();

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
                    if (selectedMenuItem->data_in->data_type[i] == 3) separat[j] = '{';
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
    // �������� �� �������� ��������
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10); 
        if ((selectedMenuItem->data_in->data_type == 4) && (result<0)) result *= -1; 
        if (selectedMenuItem->data_in->data_type[i] == 3) break;
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
    }

    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10); 
        if ((selectedMenuItem->data_in->data_type == 4) && (result<0)) result*=-1; 
        if (selectedMenuItem->data_in->data_type[i] == 3){ strcpy(selectedMenuItem->data_in->data[i], selectedMenuItem->data_in->data_temp[i]); return;}
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
    EEPROM_clear_time_init();
    PowerUP_counter();
}

void Save_time_format(){
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10);
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
    }
    RTC_set_time();
    EEPROM_clear_time_init();
    PowerUP_counter();
}
void Save_time_sleep_format(){
    // �������� �� �������� ��������
    int32_t result_1 = strtol(selectedMenuItem->data_in->data_temp[0], NULL, 10); 
    int32_t result_2 = strtol(selectedMenuItem->data_in->data_temp[1], NULL, 10); 
    if (result_1 < selectedMenuItem->data_in->DOWN_data[0]) return;
    if ((result_2 < selectedMenuItem->data_in->DOWN_data[1]) && (result_1 == 0)) return;

    if (result_1 > selectedMenuItem->data_in->UP_data[0]) return;
    if (result_2 > selectedMenuItem->data_in->UP_data[1]) return;
    
    for (int i = 0; i<selectedMenuItem->data_in->Number_of_cells; i++){
        int32_t result = strtol(selectedMenuItem->data_in->data_temp[i], NULL, 10); 
        if (selectedMenuItem->data_in->data_type[i] == 1) *((uint16_t *)selectedMenuItem->data_in->data[i]) = (uint16_t)result;
    }
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
    State_update();
    EEPROM_SaveSettings(&EEPROM);
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
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
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 123) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 192;
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
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 123) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 123;
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
    if (selectedMenuItem->action != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->action(); //  ������ �� ������� ��� ��������
        return;
    }
    mode_check();
    
    if (selectedMenuItem->Child != (void *)&NULL_ENTRY)
    {
        selectedMenuItem->Num_menu = select_menu_in_page;
        select_menu_in_page = 0;
        menuChange(selectedMenuItem->Child);
    }
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
    else if (c >= 97 && c <= 123)
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
        if (mode_redact == 5){
            return;
        }

        if (mode_redact == 4){
            Keyboard_press_code = 0xFF;
            mode_redact = 0;
        }
        if (mode_redact == 2){ 
            mode_redact = 0;
            Keyboard_press_code = 0xFF;
            return;
        }
        if (mode_redact == 3){ 
            if (Keyboard_press_code == 'L'){
                mode_redact = 0;
                return;
            }
            pass[index_pass] = Keyboard_press_code;
            uint8_t len = OLED_GetWidthStr("*");
            uint16_t x = (winth_display - len*(index_pass+1))/2; // ������������� ���������
            for (uint8_t i = 0; i < (index_pass+1); i++) 
            {
                OLED_DrawStr("*", x + (len * i), 40, 1); // ��������� ���������
            }
            OLED_UpdateScreen();
            index_pass++;
            if (index_pass>4) index_pass--;
            if ((strcmp(pass, password) == 0) && (index_pass == strlen(password))){
                selectedMenuItem->Num_menu = select_menu_in_page;
                select_menu_in_page = 0;
                menuChange(selectedMenuItem->Child);
                mode_redact = 0;
            }
            Keyboard_press_code = 0xFF;
            return;
        }
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
        //xSemaphoreGive(Display_semaphore);
        Keyboard_press_code = 0xFF;
    }
}

///
/// ����� �����
///


extern const uint8_t frames[23][1026];
extern const uint16_t frame_delays[];

void UpdateFrameDiff(const uint8_t *new_frame) {
    static uint8_t old_frame[1024] __attribute__((section(".ram2"))) = {0}; // ������ ����
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
        HAL_IWDG_Refresh(&hiwdg);
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


