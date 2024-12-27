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


int led_cursor = 1;
extern uint16_t time_update_display;
extern xSemaphoreHandle Display_semaphore;
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
/// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

extern int ADC_AKB_Proc;

extern GSM_STATUS_item GSM_data;
extern ADC_MS5193T_item ADC_data;

extern uint8_t Mode;
extern uint8_t Communication;
extern uint8_t RS485_prot;
extern uint8_t units_mes;
extern uint8_t screen_sever_mode;


char str[4];
int right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа

char trans_str[11];

extern char Keyboard_press_code;
extern int ADC_AKB_Proc;


const int max_munu_in_page = 4; // максимальное количество пунктов меню на странице видимое (+1)
int select_menu_in_page = 0;    // метущий пункт менюc
extern char len;                //  0 - русский язык;  1 -  английский
uint8_t Intermediate = 0;           // Промежуточная переменная, куда сохраняется настройка до сохранения



#define height_up_menu 14                                            // выста верхнего пункта меню
#define dist_y 12 // расстояние между пунктами меню
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

extern char ver_board[];
extern const char VERSION_PROGRAMM[10];
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

void NULL_F(){}
#define MAKE_MENU(Id, Name_rus, Name_en, Num_menu, add_signat_ru, add_signat_en, Next, Previous, Parent, Child, action, select_bar, data_in, Data_out) \
    extern menuItem Next;                                                                                                                              \
    extern menuItem Previous;                                                                                                                          \
    extern menuItem Parent;                                                                                                                            \
    extern menuItem Child;                                                                                                                             \
    menuItem Id = {Name_rus, Name_en, 0, add_signat_ru, add_signat_en, (void *)&Next, (void *)&Previous, (void *)&Parent, (void *)&Child, (void *)&action, (menuSelect_item *)&select_bar, (menuSelect_item_char *)&data_in, (char *)&Data_out}

// Выбираемые значения и статус
menuSelect_item Communication_DATA = { 
    .data = (uint8_t *)&Communication,
    .Name = {
        {"GSM/NB-IoT", "GSM/NB-IoT"},
        {"Выкл", "Off"}
    },
}; 
menuSelect_item RS485_MODE_DATA = {
    (uint8_t *)&RS485_prot,
    {
        {"Выкл.", "Off"},
        {"Modbus", "Modbus"}
    }
};
menuSelect_item UNITS_MODE_DATA = {
    (uint8_t *)&units_mes,
    {
        {"мм", "mm"},
        {"м", "m"}
    }
};
menuSelect_item SCREENSAVER = {
    (uint8_t *)&screen_sever_mode ,
    {
        {"вкл.", "on"},
        {"выкл.", "off"}
    }
};

menuSelect_item LANGUAGE = {
    (uint8_t *)&len,
    {
        {"Русский", "Русский"},
        {"English", "English"}
    }
};


menuSelect_item CURRENT_LOOP = {
    (uint8_t *)&ADC_data.mode,
    {
        {"4-20мА", "4-20mA"},
        {"0-20мА", "0-20mA"},
        {"Выкл.", "Off"}
    }
};

// отладка по USB
extern uint8_t USB_mode;
// Режим USB, - Работа с внешней flash (0), Работа в коммандном режиме (1), Работа USB в режиме чтения внутренней flash (2), работа в режиме чтения SD карты (3), Работа в режиме дебага USB (4)
menuSelect_item USB_MODE_STRUCT = {
    (uint8_t *)&USB_mode,
    {
        {"Флешка USB", "FLASH USB"},           // ИЗМЕНИТЬ НАЗВАНИЕ
        {"Команды", "Comand"},
        {"Внут. FLASH", "Int. FLASH"},
        {"Внут. SD", "Int. SD"},
        {"USB DEBUG", "USB DEBUG"}
    }
};

// Форматирование для uint8_t
void format_uint8_t(char *buffer, size_t size, void *data) {
    snprintf(buffer, size, "%u", *(uint8_t *)data);
}
// Форматирование для uint16_t
void format_uint16_t(char *buffer, size_t size, void *data) {
    snprintf(buffer, size, "%u", *(uint16_t *)data);
}
// Форматирование для int32_t
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
    format_char
};
void Null_func(){}

// Изменяемые параметры
extern GSM_STATUS_item GSM_data;
extern Prgramm_version_item Prog_ver;
// Редактирования даты RTC
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
menuSelect_item_char Date_redact = {
    {'.', '.', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    3,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&Date.Date, &Date.Month, &Date.Year},  // исходные значения
    {0, 0, 0},                  // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0, 0},                  // знаковые данные или беззнаковые
    {2, 2, 2},                  // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                  // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},         // промежуточное значение
    {31, 12, 99},               // максимальные значения
    {0, 0, 0},                  // минимальные значения
    Save_date_format            // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Редактирования времени RTC
menuSelect_item_char Time_redact = {
    {':', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&Time.Hours, &Time.Minutes, '\0'},  // исходные значения
    {0, 0, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0, 0},                   // знаковые данные или беззнаковые
    {2, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {23, 59, '\0'},              // максимальные значения
    {0, 0, '\0'},                // минимальные значения
    Save_time_format                    // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Время сна, устройства
extern uint16_t time_sleep_h;
extern uint16_t time_sleep_m;
menuSelect_item_char Time_sleep_redact = {
    {'ч', 'м', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&time_sleep_h, &time_sleep_m, '\0'},  // исходные значения
    {1, 1, 1},                  // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0, 0},                  // знаковые данные или беззнаковые
    {3, 2, 0},                  // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {1, 1, 1},                  // ширина ячеек вне редактирования (минимальная) (001 - ширина 3, 23 - ширина 2) 
    {"\0", "\0", "\0"},         // промежуточное значение
    {999, 59, '\0'},            // максимальные значения
    {0, 5, '\0'},               // минимальные значения
    Save_general_format         // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)







// Максимальный уровень погружного датчика
menuSelect_item_char Max_Level_Mesurment = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&ADC_data.MAX_LVL_char[0], &ADC_data.MAX_LVL_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов), 4 - float (знаковый)
    {1, 0, 0},                   // знаковые данные или беззнаковые
    {6, 3, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format                    // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Минимальный уровень погружного датчика
menuSelect_item_char Min_Level_Mesurment = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&ADC_data.ZERO_LVL_char[0], &ADC_data.ZERO_LVL_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {1, 0, 0},                   // знаковые данные или беззнаковые
    {6, 3, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format                    // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

menuSelect_item_char GVL_Correct = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&ADC_data.GVL_correct_char[0], &ADC_data.GVL_correct_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {1, 0, 0},                   // знаковые данные или беззнаковые
    {6, 3, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format                    // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)



menuSelect_item_char Serial_number = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    3,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    //  ЯЧЕЙКИ
    {&Prog_ver.VER_PCB_IDEOLOGY, &Prog_ver.VER_PCB_VERSION, &Prog_ver.VER_PCB_INDEX},  // исходные значения
    {0, 0, 3},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0, 0},                   // знаковые данные или беззнаковые
    {2, 2, 4},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 0},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99, 99, '\0'},              // максимальные значения
    {0, 0, '\0'},                // минимальные значения
    Save_general_format                    // ссылка на функцию завершения работы
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

////////////////////////////////////////////////////
//                 Функции меню                   //
////////////////////////////////////////////////////




////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////




extern char EEPROM_status_char[3]; // Статус доступности EEPROM
extern char FLASH_status_char[3];  // Статус доступности FLASH
extern char SD_status_char[3];     // Статус доступности SD
extern char  ADC_in_temp_char[5];

// полный тест при нажатии кнопки
void full_test(){}

// Тест отправки смс
void GSM_sms_test(){}

// Тест 
void GSM_internet_test(){}

// Обновление ПО 
void Programm_Update_USB(){}

// Калибровка ВПИ 
void Programm_DOWN_LVL_CORRECT(){}

// Калибровка НПИ
void Programm_UP_LVL_CORRECT(){}


  MAKE_MENU(Menu_1    , "Режимы"       , "Modes"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2         , PREVISION_MENU , PARENT_MENU    , Menu_1_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_1_1  , "Цикл"         , "Cycle"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2       , PREVISION_MENU , Menu_1         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_1_2  , "Диагностика"         , "Test check"         , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_3       , Menu_1_1       , Menu_1         , Menu_1_2_1     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_1, "Код ошибки"   , "Error code"   , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_2     , PREVISION_MENU , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_2, "Полный тест"  , "Full test"    , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_3     , Menu_1_2_1     , Menu_1_2       , CHILD_MENU     , full_test      , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_2_3, "АКБ"          , "BAT"          , 0              , "в"            , "v"            , Menu_1_2_4     , Menu_1_2_2     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_AKB_volts_char);
      MAKE_MENU(Menu_1_2_4, "АЦП"          , "ADC"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5     , Menu_1_2_3     , Menu_1_2       , Menu_1_2_4_1   , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_status_char);
        MAKE_MENU(Menu_1_2_4_1, "АЦП"          , "ADC"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_4_2   , PREVISION_MENU , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_value_char);
        MAKE_MENU(Menu_1_2_4_2, "Показания"    , "Metering values"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_4_3   , Menu_1_2_4_1   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_char);
        MAKE_MENU(Menu_1_2_4_3, "Сопротивление шунта"    , "Shunt resistance"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_4_4   , Menu_1_2_4_2   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_correct_char);
        MAKE_MENU(Menu_1_2_4_4, "Напряжение шунта", "Shunt voltage"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_4_5   , Menu_1_2_4_3   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN           , ADC_data.ADC_Volts_char);
        MAKE_MENU(Menu_1_2_4_5, "Ток шунта"          , "Shunt current"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_1_2_4_4   , Menu_1_2_4     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_Current_char);
      MAKE_MENU(Menu_1_2_5, "Связь"          , "Mobile network"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_6     , Menu_1_2_4     , Menu_1_2       , Menu_1_2_5_1   , ACTION_MENU    , SELECT_BAR     , DATA_IN            ,    GSM_data.GSM_status_char);
        MAKE_MENU(Menu_1_2_5_1, "Тест СМС"     , "SMS test"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_2   , PREVISION_MENU , Menu_1_2_5     , CHILD_MENU     , GSM_sms_test   , SELECT_BAR     , DATA_IN        , DATA_OUT);
        MAKE_MENU(Menu_1_2_5_2, "Тест САЙТ"    , "SITE test"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_3   , Menu_1_2_5_1   , Menu_1_2_5     , CHILD_MENU     , GSM_internet_test, SELECT_BAR   , DATA_IN        , DATA_OUT);
        MAKE_MENU(Menu_1_2_5_3, "SIM-карта"    , "SIM-card"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_4   , Menu_1_2_5_2   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_SIMCARD_char);
        MAKE_MENU(Menu_1_2_5_4, "Статус"   , "Status"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_5   , Menu_1_2_5_3   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_status_ready_char);
        MAKE_MENU(Menu_1_2_5_5, "Регистрация"  , "Logging"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_6   , Menu_1_2_5_4   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_status_reg_char);
        MAKE_MENU(Menu_1_2_5_6, "Оператор"     , "Mobile operator"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_7   , Menu_1_2_5_5   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_operator_char);
        MAKE_MENU(Menu_1_2_5_7, "Уровень сигн.", "Signal level"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_5_8   , Menu_1_2_5_6   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_signal_lvl_char);
        MAKE_MENU(Menu_1_2_5_8, "Статус GPRS", "GPRS status"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_1_2_5_7   , Menu_1_2_5     , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , GSM_data.GSM_gprs_on_char);
      MAKE_MENU(Menu_1_2_6, "EEPROM"       , "EEPROM"       , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_7     , Menu_1_2_5     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , EEPROM_status_char);
      MAKE_MENU(Menu_1_2_7, "SD-карта"           , "SD-card"           , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_1_2_8     , Menu_1_2_6     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , FLASH_status_char);
      MAKE_MENU(Menu_1_2_8, "FLASH"        , "FLASH"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_1_2_7     , Menu_1_2       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , SD_status_char);
    MAKE_MENU(Menu_1_3  , "Показания датчиков"    , "Sensor reading"         , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_1_2       , Menu_1         , Menu_1_3_1     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_1_3_1, "Тепература"   , "Temperature "         , 0              , "°"            , "°"            , Menu_1_3_2     , PREVISION_MENU , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_MS5193T_temp_char);
      MAKE_MENU(Menu_1_3_2, "Глубина"      , "Data"         , 0              , "м"           , "m"  , Menu_1_3_3     , Menu_1_3_1     , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_char);
      MAKE_MENU(Menu_1_3_3, "УГВ"   , "GWL"         , 0              , "м"           , "m"  , NEXT_MENU      , Menu_1_3_2     , Menu_1_3       , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , ADC_data.ADC_SI_value_correct_char);
  MAKE_MENU(Menu_2    , "Настройки"    , "Settings"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_3         , Menu_1         , PARENT_MENU    , Menu_2_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_1  , "Дата"         , "Date"         , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_2       , PREVISION_MENU , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Date_redact    , DATA_OUT);
    MAKE_MENU(Menu_2_2  , "Время"        , "Time"         , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_3       , Menu_2_1       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Time_redact    , DATA_OUT);
    MAKE_MENU(Menu_2_3  , "Время сна"    , "Seep time"   , 0              , "ч"            , "h"            , Menu_2_4       , Menu_2_2       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Time_sleep_redact, DATA_OUT);
    MAKE_MENU(Menu_2_4  , "Корр. УГВ"     , "GWL correction"        , 0              , "м"            , "m"            , Menu_2_5       , Menu_2_3       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , GVL_Correct        , DATA_OUT);
    MAKE_MENU(Menu_2_5  , "ВПИ"          , "Upper-range value "        , 0              , "м"            , "m"            , Menu_2_6       , Menu_2_4       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Max_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_6  , "НПИ"          , "Lower-range value"        , 0              , "м"            , "m"            , Menu_2_7       , Menu_2_5       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Min_Level_Mesurment, DATA_OUT);
    MAKE_MENU(Menu_2_7  , "Связь"        , "Mobile network"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_8       , Menu_2_6       , Menu_2         , CHILD_MENU     , ACTION_MENU    , Communication_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_8  , "Выгр. на сайт", "Data uploading"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_9       , Menu_2_7       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_9  , "Запись на USB", "USB write"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_10       , Menu_2_8       , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_10 , "Токовая петля"   , "Current loop "        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_11       , Menu_2_9       , Menu_2         , CHILD_MENU     , ACTION_MENU    , CURRENT_LOOP   , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_11 , "RS-485"        , "RS-485"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_12       , Menu_2_10       , Menu_2         , CHILD_MENU     , ACTION_MENU    , RS485_MODE_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_12 , "Ед. изм."     , "Unit measure"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13      , Menu_2_11       , Menu_2         , CHILD_MENU     , ACTION_MENU    , UNITS_MODE_DATA, DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_13 , "Инжен. меню"  , "Administration menu"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_14      , Menu_2_12       , Menu_2         , Menu_2_13_1    , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_1, "Заставка"     , "Startup screen"    , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_2      , PREVISION_MENU    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SCREENSAVER    , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_2, "ID устр.", "ID Device"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_3    , Menu_2_13_1 , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , Serial_number  , DATA_OUT);
      MAKE_MENU(Menu_2_13_3, "Обновление ПО", "Update of software"          , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_4       , Menu_2_13_2       , Menu_2_13      , CHILD_MENU     , Programm_Update_USB, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_4, "Режим USB", "USB mode"  , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_5    , Menu_2_13_3    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , USB_MODE_STRUCT, DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_5, "Калибровка шутна(верх)", "Shunt calibration(up)"  , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_6    , Menu_2_13_4    , Menu_2_13      , CHILD_MENU     , Programm_DOWN_LVL_CORRECT, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_6, "Калибровка шутна(низ)", "Shunt calibration(low)"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_7    , Menu_2_13_5    , Menu_2_13      , CHILD_MENU     , Programm_UP_LVL_CORRECT, SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_7, "Сопротивление шунта", "Shunt resistance"      , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_8    , Menu_2_13_6    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_8, "Температурная корр."   , "Offset temperature"      , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_9    , Menu_2_13_7    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_9, "Нелинейность"     , "Non-linearity"      , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_10    , Menu_2_13_8    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_10, "Температура МК"     , "MCU temp."      , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_11    , Menu_2_13_9    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_11, "Температура анал."  , "Analog sensor temp."     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_12    , Menu_2_13_10    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_12, "Температура цифр."  , "Digital sensor temp."     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_13   , Menu_2_13_11    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_13, "Тест FLASH"   , "FLASH test"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_14   , Menu_2_13_12    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_14, "Тест EEPROM"  , "EEPROM test"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_15   , Menu_2_13_13    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_15, "Тест SD-карты"      , "SD-card test"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_13_16   , Menu_2_13_14    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
      MAKE_MENU(Menu_2_13_16, "Режим БЛОКИРОВКА"   , "BLOCK mode"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU   , Menu_2_13_15    , Menu_2_13      , CHILD_MENU     , ACTION_MENU    , DATA_IN        , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_14 , "Язык"         , "Language"     , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_15      , Menu_2_13      , Menu_2         , CHILD_MENU     , ACTION_MENU    , LANGUAGE       , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_15 , "Сброс настроек"        , "Factory reset"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_2_16      , Menu_2_14      , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_2_16 , "Формат. SD"   , "SD formatting"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_2_15      , Menu_2         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
  MAKE_MENU(Menu_3    , "Сведения"     , "Info"         , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_4         , Menu_2         , PARENT_MENU    , Menu_3_1       , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);
    MAKE_MENU(Menu_3_1  , "ID устр."     , "ID Device"    , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_3_2       , PREVISION_MENU , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , Prog_ver.VERSION_PCB);
    MAKE_MENU(Menu_3_2  , "Вер. ПО"      , "Software version"        , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , Menu_3_3       , Menu_3_1       , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , Prog_ver.VERSION_PROGRAMM);
    MAKE_MENU(Menu_3_3  , "Время работы" , "Elapsed time"        , 0              , "ч"  , "h"  , NEXT_MENU      , Menu_3_2       , Menu_3         , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , Prog_ver.time_work_char);
  MAKE_MENU(Menu_4    , "Инструкция"   , "Instruction"  , 0              , ADD_SIGNAT_RU  , ADD_SIGNAT_EN  , NEXT_MENU      , Menu_3         , PARENT_MENU    , CHILD_MENU     , ACTION_MENU    , SELECT_BAR     , DATA_IN        , DATA_OUT);

////////////////////////////////////////////////////
//                Обработка меню                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

/// Возвращает длину строки без учета точек и двоеточий
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



void Data_in_no_redact(menuItem *menu, int pos_y){
    char string[20] = {'\0'};
    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        char buffer[20] = {'\0'};
        char separat[20] = {'\0'}; // массив разделителей, в рамках данного блока, разделителей нету

        uint8_t minus = 0;
        if ((menu->data_in->unsigned_signed[i] == 1) && (*(int32_t *)menu->data_in->data[i]<0)) string[0] = '-';

        formatters[menu->data_in->data_type[i]](buffer, sizeof(buffer), menu->data_in->data[i]);
        // дописать нули перед символом
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
    uint8_t len_string = OLED_GetWidthStr(string) + 4;
    OLED_DrawStr(string, winth_display - len_string, pos_y * dist_y + height_up_menu, 1);
}


void Data_in_redact(menuItem *menu, int pos_y){
    
    char string[11] = {'\0'};
    uint8_t separators_count = 0;

    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        menu->data_in->data_temp[i][menu->data_in->len_data_zero[i]] = '\0'; // на всякий случай
        strcat(string, menu->data_in->data_temp[i]);
        char temp[2] = {menu->data_in->separators[i], '\0'};
        if (menu->data_in->separators[i] != '\0') separators_count++;
        strcat(string, temp);
    }
    uint8_t len_string = OLED_GetWidthStr(string) + 4;
    OLED_DrawStr(string, winth_display - len_string, pos_y * dist_y + height_up_menu, 1);
    
    
    //uint8_t len_witout_separator = menu->data_in->len_data_zero[0] + menu->data_in->len_data_zero[1] + menu->data_in->len_data_zero[2];
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

/*  тип меню (char)0b543210 РЕДАКТИРОВАНИЕ ДЕЙСТВИЙ 
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
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
    
    // Вывод режима (прокрутка)
    if (menu->select_bar != (void *)&NULL_ENTRY)
    {   
        uint8_t num_menu = *menu->select_bar->data;
        if ((selectedMenuItem == menu) && (mode_redact == 1)) num_menu = Intermediate;

        int len_dat = OLED_GetWidthStr(menu->select_bar->Name[num_menu][leng_font]);
        
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            OLED_DrawStr(menu->select_bar->Name[num_menu][leng_font], winth_display - len_dat - 8, pos_y * dist_y + height_up_menu, 1);  // вывод если включен выбор (редактирование)
            int pos_cursor = select_menu_in_page * dist_y + height_up_menu + 2;
            int x_left = winth_display-len_dat-11;
            int x_right = winth_display-7;
            if (menu->select_bar->Name[Intermediate+1][0][0] != '\0') OLED_DrawTriangleFill(x_right, pos_cursor - 1, x_right, pos_cursor + 3, x_right+2, pos_cursor+1);
            if (Intermediate != 0) OLED_DrawTriangleFill(x_left, pos_cursor - 1, x_left, pos_cursor + 3, x_left-2, pos_cursor+1);
        }
        else{
            OLED_DrawStr(menu->select_bar->Name[num_menu][leng_font], winth_display - len_dat - 4, pos_y * dist_y + height_up_menu, 1); // вывод если нет выбора 
        }
    }


    if (menu->data_in != (void *)&NULL_ENTRY)
    {
        // режим не редактирования
        if ((mode_redact == 0) || (selectedMenuItem != menu)){
            Data_in_no_redact(menu, pos_y);
        }

        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {   
            Data_in_redact(menu, pos_y);
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
{
    FontSet(font);
    
    if (len == 0x00)
    {   
        OLED_DrawStr(menu->Name_rus, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }
    else
    {
        //leng_font = 1;
        OLED_DrawStr(menu->Name_en, pos_x_menu, pos_y * dist_y + height_up_menu, 1);
    }

    if (menu->data_out != (void *)&NULL_ENTRY)
    {
        uint8_t len_add_signa = 0;
        if ((len == 0x00) && (menu->add_signat_ru != (void *)&NULL_ENTRY)) len_add_signa = OLED_GetWidthStr(menu->add_signat_ru);
        if ((len != 0x00) && (menu->add_signat_ru != (void *)&NULL_ENTRY))  len_add_signa = OLED_GetWidthStr(menu->add_signat_en);
        OLED_DrawStr(menu->data_out, winth_display - (OLED_GetWidthStr(menu->data_out)) - (len_add_signa+4), pos_y * dist_y + height_up_menu, 1);
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
    //OLED_DrawStr("005ч08м", 1, 1, 1);
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
    right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа
    //Вариант оптимизации
}

// отображение всех пунктов меню на странице
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

// вход в режим редактирования, если есть соответствующее поле
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        // ввод значений
        if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
        {   
            for (int i = 0; i<3; i++)
            {
                char separat[11] = {'\0'};
                for (int j = 0; j<11; j++) selectedMenuItem->data_in->data_temp[i][j] = '\0';

                uint8_t minus = 0;
                if ((selectedMenuItem->data_in->unsigned_signed[i] == 1) && (*(int32_t *)selectedMenuItem->data_in->data[i]<0)) minus = 1;
                formatters[selectedMenuItem->data_in->data_type[i]](selectedMenuItem->data_in->data_temp[i], sizeof(selectedMenuItem->data_in->data_temp[i]), selectedMenuItem->data_in->data[i]);
                
                
                uint8_t len_dat = search_len_mass(selectedMenuItem->data_in->data_temp[i], 11, separat);
                for (int j = 0; j < (uint8_t)selectedMenuItem->data_in->len_data_zero[i] - len_dat; j++){
                    if (selectedMenuItem->data_in->data_type[i] != 3) separat[j] = '0';
                    if (selectedMenuItem->data_in->data_type[i] == 3) separat[j] = 0x99;
                }
                
                strcat(separat, selectedMenuItem->data_in->data_temp[i]);
                strcpy(selectedMenuItem->data_in->data_temp[i], separat);
                if (minus == 1) selectedMenuItem->data_in->data_temp[i][selectedMenuItem->data_in->len_data_zero[i]] = '-'; 
                else if (selectedMenuItem->data_in->unsigned_signed[i] == 1) selectedMenuItem->data_in->data_temp[i][selectedMenuItem->data_in->len_data_zero[i]] = '+'; 
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
        if (selectedMenuItem->data_in->data_type[i] == 3){ strcpy(selectedMenuItem->data_in->data[i], selectedMenuItem->data_in->data_temp[i]); return;}
        if (result < selectedMenuItem->data_in->DOWN_data[i]) return;
        if (result > selectedMenuItem->data_in->UP_data[i]) return;
        if (selectedMenuItem->data_in->data_type[i] == 0) *((uint8_t *)selectedMenuItem->data_in->data[i]) = (uint8_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 1) *((uint16_t *)selectedMenuItem->data_in->data[i]) = (uint16_t)result;
        if (selectedMenuItem->data_in->data_type[i] == 2) *((int32_t *)selectedMenuItem->data_in->data[i]) = result;
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
    // Запись данных в изначальные величины и вызов необходимых функций

    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){ 
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        pos_redact = len_witout_separator - 1;
        selectedMenuItem->data_in->end_redact_func(); // вызов функции для сохранения изменений
    }
    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY) *selectedMenuItem->select_bar->data = Intermediate;
    
    //pos_redact = len_dat - 1;
    mode_redact = 0;
    led_cursor = 1;
    time_update_display = time_updateDisplay;
}


void position_calculate(int8_t *_add_pos, int *_position)
{
    int separators_count = selectedMenuItem->data_in->Number_of_cells;
    //for (int i = 0; i < 3; i++) if (selectedMenuItem->data_in->separators[i] != '\0') separators_count++;
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

// проверка на соответствие данных минимуму и максимому

void up_redact()
{
    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY)
    {
        // действия при вводе. Нажатие вниз
        int8_t add_pos = 0;
        int position = pos_redact;
        position_calculate(&add_pos, &position);
        // если тип данных char
        if (selectedMenuItem->data_in->data_type[add_pos] == 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] < 255) selectedMenuItem->data_in->data_temp[add_pos][position]++;
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 122) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 192;
        // ДОБАВИТЬ АДЕКВАТНОЕ УСЛОВИЕ НА ПЕРЕКЛЮЧЕНИЕ ТОЛЬКО ПО АЛФАВИТАМ
        
        }
        // если тип данных положительный
        if (selectedMenuItem->data_in->data_type[add_pos] != 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == '9') selectedMenuItem->data_in->data_temp[add_pos][position] = '0';
            else selectedMenuItem->data_in->data_temp[add_pos][position]++;
        }
        //data_correct(add_pos);
        // добавить проверку min-max

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
        int8_t add_pos = 0;
        int position = pos_redact;
        position_calculate(&add_pos, &position);
        // если тип данных char
        if (selectedMenuItem->data_in->data_type[add_pos] == 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] < 31) selectedMenuItem->data_in->data_temp[add_pos][position] = 33;
            if (selectedMenuItem->data_in->data_temp[add_pos][position]-1 > 32) selectedMenuItem->data_in->data_temp[add_pos][position]--;
            if ((selectedMenuItem->data_in->data_temp[add_pos][position] > 122) && (selectedMenuItem->data_in->data_temp[add_pos][position] < 192)) selectedMenuItem->data_in->data_temp[add_pos][position] = 122;
        }
        // если тип данных положительный
        if (selectedMenuItem->data_in->data_type[add_pos] != 3){ 
            if (selectedMenuItem->data_in->data_temp[add_pos][position] == '0') selectedMenuItem->data_in->data_temp[add_pos][position] = '9';
            else selectedMenuItem->data_in->data_temp[add_pos][position]--;
        }
        //data_correct(add_pos);
        // добавить проверку min-max

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
        led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
        if (pos_redact >= len_witout_separator)
        {
            if (selectedMenuItem->data_in->redact_right_end == 1) redact_end();
            else pos_redact--;
        }

        // действия при вводе. Нажатие вправо
    }

    if (selectedMenuItem->select_bar != (void *)&NULL_ENTRY)
    {
        if (selectedMenuItem->select_bar->Name[Intermediate+1][0][0] != '\0'){
            //*selectedMenuItem->select_bar->data+=1;
            Intermediate+=1;
        }
        // действие при прокрутке вправо
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
/*  тип меню (char)0b543210
    6 - вкладка                                             0x40
    5 - ввод значения                                       0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
*/

// редактирование позиции с учетом  разделителей
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
        led_cursor = 0; // нужно что бы курсор сразу загорелся при переклбчении
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        if (pos_redact >= len_witout_separator)
            pos_redact -=  1;
    }
}
void null_fun(){}
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
            if (Keyboard_press_code >= '0' && Keyboard_press_code <= '9')
            {
                key_press_data_write(Keyboard_press_code);
            }
        }
        Keyboard_press_code = 0xFF;
    }
}

///
/// ВЫВОД ВИДЕО
///


extern const uint8_t frames[23][1026];
extern const uint16_t frame_delays[];

void UpdateFrameDiff(const uint8_t *new_frame) {
    static uint8_t old_frame[1024] = {0}; // Старый кадр
    uint8_t temp_buffer[1024] = {0}; // Промежуточный буфер для нового кадра

    uint8_t width = new_frame[0];
    uint8_t height = new_frame[1];
    const uint8_t* data_ptr = &new_frame[2];
    int widthInBytes = (width + 7) / 8;

    // Декодируем новый кадр в temp_buffer без прямой записи в oled_buffer
    // В данном случае мы просто раскодируем XBM в формат схожий с oled_buffer
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t data = data_ptr[(y * widthInBytes) + (x / 8)];
            uint8_t bit = 1 << (x & 7);
            if (data & bit) {
                temp_buffer[x + (y/8)*OLED_WIDTH] |= (1 << (y & 7));
            }
        }
    }

    // Сравниваем temp_buffer с old_frame, чтобы найти изменившиеся биты
    uint8_t changed_pages[8] = {0}; // Отслеживаем какие страницы изменились
    for (int i = 0; i < 1024; i++) {
        uint8_t diff = temp_buffer[i] ^ old_frame[i];
        if (diff != 0) {
            int page = i / OLED_WIDTH;
            int x_pos = i % OLED_WIDTH;
            for (int bit = 0; bit < 8; bit++) {
                if (diff & (1 << bit)) {
                    // Пиксель изменился
                    uint8_t new_pixel = (temp_buffer[i] & (1 << bit)) ? 1 : 0;
                    OLED_DrawPixelStatus(x_pos, page*8 + bit, new_pixel);
                    changed_pages[page] = 1;
                }
            }
        }
    }

    // Обновляем только те страницы, которые изменились
    for (int p = 0; p < 8; p++) {
        if (changed_pages[p]) {
            OLED_UpdateOnePage(p);
        }
    }

    // Запоминаем текущий кадр как старый для следующего прохода
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
            OLED_SendCommand(0xD9);  //  Установка фаз
            OLED_SendCommand(charge_period);  // Установка фаз
            contrast -= 31;
            charge_period-=28;
        }
    }
    
    OLED_SendCommand(0xD9);  //  Установка фаз на рабочий режим
    OLED_SendCommand(0xF1);  //  Установка фаз на рабочий режим
    OLED_SetContrast(0xFF);
}