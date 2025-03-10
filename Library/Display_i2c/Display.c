#include "Display.h"
// #include "RTC_data.h"

#define winth_display 128
#define height_display 64

#define line_indentation 0                            //  отступы верхней линии слева и справа
#define end_line 125                                  //  отступы верхней линии слева и справа
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
int mode_redact = 0;  // 1 - режим редактирования данных, 0 - режим переключения страниц, 2 - режим отображения (всплывающее окно)
int pos_redact = 0;   // позиция для редактирования
char pass[5] = "";
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


char str[5];
int right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа

char trans_str[11];

extern char Keyboard_press_code;
extern int ADC_AKB_Proc;


const int max_munu_in_page = 4; // максимальное количество пунктов меню на странице видимое (+1)
int select_menu_in_page = 0;    // метущий пункт менюc
uint8_t Intermediate = 0;           // Промежуточная переменная, куда сохраняется настройка до сохранения при прокрутке (номер пункта)



#define height_up_menu 14                                            // выста верхнего пункта меню
#define dist_y 12 // расстояние между пунктами меню
#define pos_x_menu 4                                                 // отступ от края для названий пунктов меню
#define pos_x_menu_data 100                                          // отступ от края для вывода значений

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

// Вывод на экран при действии
const char Clear[2][40] = {"Отчистка FLASH",  "CLEAR FLASH"};
const char POWER_NOT[2][40] = {"НЕ ОТКЛЮЧАЙТЕ ПИТАНИЕ",  "НЕ ОТКЛЮЧАЙТЕ ПИТАНИЕ"};
const char READY[2][40] = {"Готово",  "Ready"};

// Выбираемые значения и статус
menuSelect_item Communication_DATA = { 
    .data = (uint8_t *)&EEPROM.Communication,
    .Name = {
        {"Выкл", "Off"},
        {"GSM/NB-IoT", "GSM/NB-IoT"}
    },
}; 
menuSelect_item RS485_MODE_DATA = {
    (uint8_t *)&EEPROM.RS485_prot,
    {
        {"Выкл.", "Off"},
        {"Modbus", "Modbus"}
    }
};
menuSelect_item UNITS_MODE_DATA = {
    (uint8_t *)&EEPROM.units_mes,
    {
        {"мм", "mm"},
        {"м", "m"},
        {"Па", "Pa"}
    }
};
menuSelect_item SCREENSAVER = {
    (uint8_t *)&EEPROM.screen_sever_mode ,
    {
        {"выкл.", "off"},
        {"вкл.", "on"}
    }
};

menuSelect_item LANGUAGE = {
    (uint8_t *)&EEPROM.len,
    {
        {"Русский", "Русский"},
        {"English", "English"}
    }
};


menuSelect_item CURRENT_LOOP = {
    (uint8_t *)&EEPROM.mode_ADC,
    {
        {"4-20мА", "4-20mA"},
        {"0-20мА", "0-20mA"},
        {"Выкл.", "Off"}
    }
};

// Режим USB, - Работа с внешней flash (0), Работа в коммандном режиме (1), Работа USB в режиме чтения внутренней flash (2), работа в режиме чтения SD карты (3), Работа в режиме дебага USB (4)
menuSelect_item USB_MODE_STRUCT = {
    (uint8_t *)&EEPROM.USB_mode,
    {
        {"Флешка USB", "FLASH USB"},           // ИЗМЕНИТЬ НАЗВАНИЕ
        {"Команды", "Comand"},
        {"Внут. FLASH", "Int. FLASH"},
        {"Внут. SD", "Int. SD"}
    }
};

// Куда сохранять данные из меню показаний
menuSelect_item SAVE_IN_STRUCT = {
    (uint8_t *)&EEPROM.Save_in,
    {
        {"FLASH", "FLASH"},           // ИЗМЕНИТЬ НАЗВАНИЕ
        {"SD", "SD"},
        {"USB", "USB"},
        {"Сайт", "Website"}
    }
};

// Блокировка устройства
menuSelect_item Block = {
    (uint8_t *)&EEPROM.block,
    {
        {"Разбл.", "Unlocked"},           // ИЗМЕНИТЬ НАЗВАНИЕ
        {"Забл.", "Locked"}
    }
};

// Блокировка устройства
menuSelect_item Mode_select = {
    (uint8_t *)&EEPROM.Mode,
    {
        {"Текущие", "Current"},           // ИЗМЕНИТЬ НАЗВАНИЕ
        {"Циклический", "Cyclic"},
        {"Выставка", "Exhibition"}
    }
};

menuSelect_item Unit_voltage = {
    (uint8_t *)&dat, { {"в", "v"} }
};
menuSelect_item Unit_hours = {
    (uint8_t *)&dat, { {"ч", "h"} }
};
menuSelect_item Unit_minuts = {
    (uint8_t *)&dat, { {"м", "m"} }
};
menuSelect_item Unit_degree = {
    (uint8_t *)&dat, { {"°", "°"} }
};
menuSelect_item Unit_resistance = {
    (uint8_t *)&dat, { {"Ом", "Ohms"} }
};
menuSelect_item Unit_current = {
    (uint8_t *)&dat, { {"мА", "mAmps"} }
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
    format_char,
    format_int32_t
};
void Null_func(){}




// Изменяемые параметры
extern GSM_STATUS_item GSM_data;
// Редактирования даты RTC
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
menuSelect_item_char Date_redact = {
    {'.', '.', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    3,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    '\0',
    //  ЯЧЕЙКИ
    {&Date.Date, &Date.Month, &Date.Year},  // исходные значения
    {0, 0, 0},                  // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                     // знак (автоматически)
    {2, 2, 2},                  // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                  // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},         // промежуточное значение
    {31, 12, 99},               // максимальные значения
    {0, 0, 0},                  // минимальные значения
    Save_date_format,           // ссылка на функцию завершения работы
    '\0'
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Редактирования времени RTC
menuSelect_item_char Time_redact = {
    {':', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    '\0',
    //  ЯЧЕЙКИ
    {&Time.Hours, &Time.Minutes, '\0'},  // исходные значения
    {0, 0, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                   // знак (автоматически)
    {2, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {23, 59, '\0'},              // максимальные значения
    {0, 0, '\0'},                // минимальные значения
    Save_time_format,                    // ссылка на функцию завершения работы
    '\0',
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Время сна, устройства

menuSelect_item_char Time_sleep_redact = {
    {'ч', 'м', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    '\0',
    //  ЯЧЕЙКИ
    {&EEPROM.time_sleep_h, &EEPROM.time_sleep_m, '\0'},  // исходные значения
    {1, 1, 1},                  // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                  // знак (автоматически)
    {3, 2, 0},                  // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {1, 1, 0},                  // ширина ячеек вне редактирования (минимальная) (001 - ширина 3, 23 - ширина 2) 
    {"\0", "\0", "\0"},         // промежуточное значение
    {255, 59, '\0'},            // максимальные значения
    {0, 5, '\0'},               // минимальные значения
    Save_time_sleep_format,         // ссылка на функцию завершения работы
    '\0',
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)


// ВПИ, верхний предел измерений
menuSelect_item_char Max_Level_Mesurment = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    &ADC_data.MAX_LVL,
    //  ЯЧЕЙКИ
    {&ADC_data.MAX_LVL_char[0], &ADC_data.MAX_LVL_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов), 4 - float (знаковый)
    {0, 0},                     // знак (автоматически)
    {6, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {999999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    SAVE_DOUBLE
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// НПИ, нижний предел измерений
menuSelect_item_char Min_Level_Mesurment = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    &ADC_data.ZERO_LVL,
    //  ЯЧЕЙКИ
    {&ADC_data.ZERO_LVL_char[0], &ADC_data.ZERO_LVL_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов) 4 - int32 беззнаковый
    {0, 0},                   // знак (автоматически)
    {6, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    SAVE_DOUBLE
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

// Калибровка ВПИ 
void SAVE_DOUBLE(double **save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed){
    **save_data = 0;
    **save_data+=*value_int;
    double temp = (double)*value_float;
    for (int i=0; i<size_float; i++) temp/=10;
    **save_data+=temp;
    if (_signed == 1) **save_data*=-1;
}

menuSelect_item_char GVL_Correct = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    &ADC_data.GVL_correct,
    //  ЯЧЕЙКИ
    {&ADC_data.GVL_correct_char[0], &ADC_data.GVL_correct_char[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                      // знак (автоматически)
    {5, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 2},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    SAVE_DOUBLE
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

menuSelect_item_char Serial_number = {
    {'\0', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    1,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    '\0',
    //  ЯЧЕЙКИ
    {&EEPROM.version.VERSION_PCB, '\0', '\0'},  // исходные значения
    {3, 0, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                      // знаковые данные или беззнаковые
    {10, 0, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {0, 0, 0},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {'\0', '\0', '\0'},              // максимальные значения
    {'\0', '\0', '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    '\0'
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)

menuSelect_item_char Password = {
    {'\0', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    1,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    '\0',
    //  ЯЧЕЙКИ
    {&EEPROM.version.password, '\0', '\0'},  // исходные значения
    {3, 0, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                      // знаковые данные или беззнаковые
    {5, 0, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {0, 0, 0},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {'\0', '\0', '\0'},              // максимальные значения
    {'\0', '\0', '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    '\0'
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)


menuSelect_item_char Temp_correct = {
    {'.', '\0', '\0'},   // разделители, если нету, то '\0', последний может использоваться в качестве приписки
    2,                  // колтичество ячеек данных
    0,                  // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено 
    &ADC_data.Temp_correct_A,
    //  ЯЧЕЙКИ
    {&ADC_data.Temp_correct[0], &ADC_data.Temp_correct[1], '\0'},  // исходные значения
    {2, 2, 0},                   // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов)
    {0, 0},                      // знак (автоматически)
    {5, 2, 0},                   // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    {2, 2, 0},                   // ширина ячеек вне редактирования (001 - ширина 3, 23 - ширина 2)
    {"\0", "\0", "\0"},          // промежуточное значение
    {99999, 999, '\0'},              // максимальные значения
    {-99999, 0, '\0'},                // минимальные значения
    Save_general_format,                    // ссылка на функцию завершения работы
    SAVE_DOUBLE
}; // Промежуточная переменная, куда сохраняется настройка до сохранения (char)



// uint8_t dat = 0;
menuSelect_item NO_SIGNED = {
    (uint8_t *)&dat,
    {{"", ""}}
};



MAKE_MENU(Menu_1, "Режимы", "Modes", 0, UPTADE_OFF, NO_SIGNED, Menu_2, PREVISION_MENU, PARENT_MENU, Menu_1_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_1_1, "Цикл", "Cycle", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2, PREVISION_MENU, Menu_1, CHILD_MENU, sleep, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_1_2, "Диагностика", "Test check", 0, UPTADE_OFF, NO_SIGNED, Menu_1_3, Menu_1_1, Menu_1, Menu_1_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_1_2_1, "Ошибка", "Error", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_2, PREVISION_MENU, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ERRCODE.STATUSCHAR);
		MAKE_MENU(Menu_1_2_2, "Полный тест", "Full test", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_3, Menu_1_2_1, Menu_1_2, CHILD_MENU, full_test, SELECT_BAR, DATA_IN, DATA_OUT); // ! Написать тест
		MAKE_MENU(Menu_1_2_3, "АКБ", "BAT", 0, UPTADE_OFF, Unit_voltage, Menu_1_2_4, Menu_1_2_2, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, IntADC.ADC_AKB_volts_char);
		MAKE_MENU(Menu_1_2_4, "Связь", "Mobile network", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_5, Menu_1_2_3, Menu_1_2, Menu_1_2_4_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_1, "Тест СМС", "SMS test", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_4_2, PREVISION_MENU, Menu_1_2_4, CHILD_MENU, GSM_sms_test, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_2, "Тест САЙТ", "SITE test", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_4_3, Menu_1_2_4_1, Menu_1_2_4, CHILD_MENU, GSM_internet_test, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_3, "SIM-карта", "SIM-card", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_4, Menu_1_2_4_2, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_4, "Режим", "Mode", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_5, Menu_1_2_4_3, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_5, "Регистрация", "Logging", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_6, Menu_1_2_4_4, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_6, "Страна", "Country", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_7, Menu_1_2_4_5, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_7, "Оператор", "Mobile operator", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_8, Menu_1_2_4_6, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_8, "Уровень сигн.", "Signal level", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_9, Menu_1_2_4_7, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_9, "Потери", "Signal err", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_4_10, Menu_1_2_4_8, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
			MAKE_MENU(Menu_1_2_4_10, "Статус GPRS", "GPRS status", 0, UPTADE_ON, NO_SIGNED, NEXT_MENU, Menu_1_2_4_9, Menu_1_2_4, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_1_2_5, "АЦП", "ADC", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_6, Menu_1_2_4, Menu_1_2, Menu_1_2_5_1, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_status_char);
			MAKE_MENU(Menu_1_2_5_1, "АЦП", "ADC", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_5_2, PREVISION_MENU, Menu_1_2_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_value_char);
			MAKE_MENU(Menu_1_2_5_2, "Глубина", "Metering", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_2_5_3, Menu_1_2_5_1, Menu_1_2_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_char);
			MAKE_MENU(Menu_1_2_5_3, "УГВ", "GWL", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_2_5_4, Menu_1_2_5_2, Menu_1_2_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_correct_char);
			MAKE_MENU(Menu_1_2_5_4, "Ток датчика", "Current", 0, UPTADE_ON, Unit_current, NEXT_MENU, Menu_1_2_5_3, Menu_1_2_5, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_Current_char);
		MAKE_MENU(Menu_1_2_6, "EEPROM", "EEPROM", 0, UPTADE_ON, NO_SIGNED, Menu_1_2_7, Menu_1_2_5, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM_status_char); // ! обновить статус
		MAKE_MENU(Menu_1_2_7, "SD-карта", "SD-card", 0, UPTADE_OFF, NO_SIGNED, Menu_1_2_8, Menu_1_2_6, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, FLASH_status_char); // ! обновить статус
		MAKE_MENU(Menu_1_2_8, "FLASH", "FLASH", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_1_2_7, Menu_1_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, SD_status_char); // ! обновить статус
	MAKE_MENU(Menu_1_3, "Показания", "Sensor reading", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_1_2, Menu_1, Menu_1_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_1_3_1, "Глубина", "Metering", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_3_2, PREVISION_MENU, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_char);
		MAKE_MENU(Menu_1_3_2, "УГВ", "GWL", 0, UPTADE_ON, UNITS_MODE_DATA, Menu_1_3_3, Menu_1_3_1, Menu_1_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_SI_value_correct_char);
		MAKE_MENU(Menu_1_3_3, "Сохранить", "Save", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_1_3_2, Menu_1_3, CHILD_MENU, SAVE_IZM, SELECT_BAR, DATA_IN, DATA_OUT); // ! Сохранить показания в зависисмости от режима
MAKE_MENU(Menu_2, "Настройки", "Settings", 0, UPTADE_OFF, NO_SIGNED, Menu_3, Menu_1, PARENT_MENU, Menu_2_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_1, "Дата", "Date", 0, UPTADE_OFF, NO_SIGNED, Menu_2_2, PREVISION_MENU, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Date_redact, DATA_OUT);
	MAKE_MENU(Menu_2_2, "Время", "Time", 0, UPTADE_OFF, NO_SIGNED, Menu_2_3, Menu_2_1, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_redact, DATA_OUT);
	MAKE_MENU(Menu_2_3, "Время сна", "Seep time", 0, UPTADE_OFF, NO_SIGNED, Menu_2_4, Menu_2_2, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Time_sleep_redact, DATA_OUT);
	MAKE_MENU(Menu_2_4, "Корр. УГВ", "GWL correction", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_5, Menu_2_3, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, GVL_Correct, DATA_OUT);
	MAKE_MENU(Menu_2_5, "ВПИ", "U-range", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_6, Menu_2_4, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Max_Level_Mesurment, DATA_OUT);
	MAKE_MENU(Menu_2_6, "НПИ", "L-range", 0, UPTADE_OFF, UNITS_MODE_DATA, Menu_2_7, Menu_2_5, Menu_2, CHILD_MENU, ACTION_MENU, SELECT_BAR, Min_Level_Mesurment, DATA_OUT);
	MAKE_MENU(Menu_2_7, "Связь", "Network", 0, UPTADE_OFF, NO_SIGNED, Menu_2_8, Menu_2_6, Menu_2, CHILD_MENU, ACTION_MENU, Communication_DATA, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_8, "Синхронизация", "Sin", 0, UPTADE_OFF, NO_SIGNED, Menu_2_9, Menu_2_7, Menu_2, CHILD_MENU, GSM_HTTP_READ, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_9, "Сохранить в", "Save in", 0, UPTADE_OFF, NO_SIGNED, Menu_2_10, Menu_2_8, Menu_2, CHILD_MENU, ACTION_MENU, SAVE_IN_STRUCT, DATA_IN, DATA_OUT); // ! Выбор куда сохранить (USB, SD, FLASH, сайт)
	MAKE_MENU(Menu_2_10, "Выгрузка на USB", "USB write", 0, UPTADE_OFF, NO_SIGNED, Menu_2_11, Menu_2_9, Menu_2, CHILD_MENU, SAVE_USB, SELECT_BAR, DATA_IN, DATA_OUT); // ! Выгрузка на USB
	MAKE_MENU(Menu_2_11, "Токовая петля", "Current loop ", 0, UPTADE_OFF, NO_SIGNED, Menu_2_12, Menu_2_10, Menu_2, CHILD_MENU, ACTION_MENU, CURRENT_LOOP, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_12, "RS-485", "RS-485", 0, UPTADE_OFF, NO_SIGNED, Menu_2_13, Menu_2_11, Menu_2, CHILD_MENU, ACTION_MENU, RS485_MODE_DATA, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_13, "Ед. изм.", "Unit measure", 0, UPTADE_OFF, NO_SIGNED, Menu_2_14, Menu_2_12, Menu_2, CHILD_MENU, ACTION_MENU, UNITS_MODE_DATA, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_14, "Язык", "Language", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15, Menu_2_13, Menu_2, CHILD_MENU, ACTION_MENU, LANGUAGE, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_15, "Инжен. меню", "Administration menu", 0, UPTADE_OFF, NO_SIGNED, Menu_2_16, Menu_2_14, Menu_2, Menu_2_15_1, PASSWORD, SELECT_BAR, DATA_IN, DATA_OUT); // ! Пароль на вход
		MAKE_MENU(Menu_2_15_1, "Заставка", "Startup screen", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_2, PREVISION_MENU, Menu_2_15, CHILD_MENU, ACTION_MENU, SCREENSAVER, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_2, "ID устр.", "ID Device", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_3, Menu_2_15_1, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Serial_number, DATA_OUT);
		MAKE_MENU(Menu_2_15_3, "Пароль", "Password", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_4, Menu_2_15_2, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Password, DATA_OUT);
		MAKE_MENU(Menu_2_15_4, "Режим USB", "USB mode", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_5, Menu_2_15_3, Menu_2_15, CHILD_MENU, ACTION_MENU, USB_MODE_STRUCT, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_5, "Обновление ПО", "Update of software", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_6, Menu_2_15_4, Menu_2_15, CHILD_MENU, Programm_Update_USB, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_6, "Калибровка 24в", "Colibrate 24v", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_7, Menu_2_15_5, Menu_2_15, CHILD_MENU, Read_ADC_Colibrate_24V, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_7, "Калибровка 20мА", "Calibration(up)", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_8, Menu_2_15_6, Menu_2_15, CHILD_MENU, colibrate_20ma, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_8, "Калибровка 4мА", "Calibration(low)", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_9, Menu_2_15_7, Menu_2_15, CHILD_MENU, colibrate_4ma, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_9, "Калиб. темп.", "Shunt resistance", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_10, Menu_2_15_8, Menu_2_15, CHILD_MENU, temperature_colibrate, SELECT_BAR, DATA_IN, DATA_OUT); // ! Связаться с датчиком цифровым и откорректировать аналоговый
		MAKE_MENU(Menu_2_15_10, "Темп. корр.", "Offset temperature", 0, UPTADE_OFF, Unit_degree, Menu_2_15_11, Menu_2_15_9, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, Temp_correct, DATA_OUT); // ! Вводе корректирующего коэффициента
		MAKE_MENU(Menu_2_15_11, "Темп. анал.", "Analog temp.", 0, UPTADE_ON, Unit_degree, Menu_2_15_12, Menu_2_15_10, Menu_2_15, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, ADC_data.ADC_MS5193T_temp_char);
		MAKE_MENU(Menu_2_15_12, "Темп. цифр.", "Digital sensor temp.", 0, UPTADE_ON, Unit_degree, Menu_2_15_13, Menu_2_15_11, Menu_2_15, CHILD_MENU, SELECT_BAR, SELECT_BAR, DATA_IN, OneWire_temp_char); // ! обновить статус
		MAKE_MENU(Menu_2_15_13, "Тест FLASH", "FLASH test", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_14, Menu_2_15_12, Menu_2_15, CHILD_MENU, flash_test, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_14, "Тест EEPROM", "EEPROM test", 0, 0, NO_SIGNED, Menu_2_15_15, Menu_2_15_13, Menu_2_15, CHILD_MENU, EEPROM_test, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_15, "Тест SD-карты", "SD-card test", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_16, Menu_2_15_14, Menu_2_15, CHILD_MENU, SD_test, SELECT_BAR, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_16, "Режим", "Mode", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_17, Menu_2_15_15, Menu_2_15, CHILD_MENU, SELECT_BAR, Mode_select, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_17, "БЛОКИРОВКА", "BLOCK", 0, UPTADE_OFF, NO_SIGNED, Menu_2_15_18, Menu_2_15_16, Menu_2_15, CHILD_MENU, SELECT_BAR, Block, DATA_IN, DATA_OUT);
		MAKE_MENU(Menu_2_15_18, "Полный Сброс", "FULL RESET", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_2_15_17, Menu_2_15, CHILD_MENU, ALL_Reset_settings, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_2_16, "Формат. SD", "SD formatting", 0, UPTADE_OFF, NO_SIGNED, Menu_2_17, Menu_2_15, Menu_2, CHILD_MENU, SD_Format, SELECT_BAR, DATA_IN, DATA_OUT); // ! Форматирование SD
	MAKE_MENU(Menu_2_17, "Формат. Flash", "SD formatting", 0, UPTADE_OFF, NO_SIGNED, Menu_2_18, Menu_2_16, Menu_2, CHILD_MENU, Flash_Format, SELECT_BAR, DATA_IN, DATA_OUT); // ! Форматирование FLASH
	MAKE_MENU(Menu_2_18, "Сброс настроек", "Factory reset", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_2_17, Menu_2, CHILD_MENU, Reset_settings, SELECT_BAR, DATA_IN, DATA_OUT);
MAKE_MENU(Menu_3, "Сведения", "Info", 0, UPTADE_OFF, NO_SIGNED, Menu_4, Menu_2, PARENT_MENU, Menu_3_1, ACTION_MENU, SELECT_BAR, DATA_IN, DATA_OUT);
	MAKE_MENU(Menu_3_1, "ID устр.", "ID Device", 0, UPTADE_OFF, NO_SIGNED, Menu_3_2, PREVISION_MENU, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM.version.VERSION_PCB);
	MAKE_MENU(Menu_3_2, "Вер. ПО", "Software version", 0, UPTADE_OFF, NO_SIGNED, Menu_3_3, Menu_3_1, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM.version.VERSION_PROGRAMM);
	MAKE_MENU(Menu_3_3, "Время работы", "Elapsed time", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_3_2, Menu_3, CHILD_MENU, ACTION_MENU, SELECT_BAR, DATA_IN, EEPROM.version.time_work_char);
MAKE_MENU(Menu_4, "Инструкция", "Instruction", 0, UPTADE_OFF, NO_SIGNED, NEXT_MENU, Menu_3, PARENT_MENU, CHILD_MENU, Instruction, SELECT_BAR, DATA_IN, DATA_OUT);

void Add_units(void)
{
    // Теперь присваиваем указатели на строки, полученные из GSM_data
    Menu_1_3_1.add_signat   = &UNITS_MODE_DATA;       // Вкладка связь
    Menu_1_3_2.add_signat = &UNITS_MODE_DATA;
    Menu_1_2_5_2.add_signat = &UNITS_MODE_DATA;
    Menu_1_2_5_3.add_signat = &UNITS_MODE_DATA;
}
void Remove_units(void)
{
    // Теперь присваиваем указатели на строки, полученные из GSM_data
    Menu_1_3_1.add_signat   = &NO_SIGNED;       // Вкладка связь
    Menu_1_3_2.add_signat = &NO_SIGNED;
    Menu_1_2_5_2.add_signat = &NO_SIGNED;
    Menu_1_2_5_3.add_signat = &NO_SIGNED;
}

void InitMenus(void)
{
    // Теперь присваиваем указатели на строки, полученные из GSM_data
    Menu_1_2_4.data_out   = GSM_data.GSM_status_char;       // Вкладка связь
    Menu_1_2_4_3.data_out = GSM_data.GSM_SIMCARD_char;
    Menu_1_2_4_4.data_out = GSM_data.Modem_mode;
    Menu_1_2_4_5.data_out = GSM_data.GSM_status_reg_char;
    Menu_1_2_4_6.data_out = GSM_data.GSM_region_char;
    Menu_1_2_4_7.data_out = GSM_data.GSM_operator_char;
    Menu_1_2_4_8.data_out = GSM_data.GSM_signal_lvl_char;
    Menu_1_2_4_9.data_out = GSM_data.GSM_err_lvl_char;
    Menu_1_2_4_10.data_out = GSM_data.GSM_gprs_on_char;
}
////////////////////////////////////////////////////
//                Обработка меню                  //
////////////////////////////////////////////////////
menuItem *selectedMenuItem = &Menu_1;

////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////

void PROGRESS_BAR(uint8_t procent){
    #define Y 25
    #define leight 80
    #define first_point (winth_display-leight)/2
    uint8_t procent_len = (float)((float)leight/100)*procent;
    OLED_DrawRectangle(first_point, Y+15, leight+first_point, Y+25);
    OLED_DrawRectangleFill(first_point, Y+15, procent_len+first_point, Y+25, 1);
}


// полный тест при нажатии кнопки
void full_test(){}

void USB_FLASH_SAVE(){}
// Тест отправки смс
void GSM_sms_test(){
    GSM_data.Status |= SMS_SEND;
}

void flash_test(){}
void SD_test(){}
void EEPROM_test(){}

// Сохранить изменения, учесть куда выбрано сохранение данных 
void SAVE_IZM(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 25
    OLED_DrawStr("Сохранение...", X, Y, 1);
    OLED_UpdateScreen();

    Collect_DATA();
    update_flash_end_ptr();
    flash_append_record(save_data);

    //flash_read_record_by_index(flash_end_ptr, save_data);

    char flash_end_ptr_char[30] = {0};
    snprintf(flash_end_ptr_char, 30, "Записей: %ld", flash_end_ptr);
    
    OLED_DrawStr("Готово", X+5, Y+10, 1);
    OLED_DrawStr(flash_end_ptr_char, X+5, Y+20, 1);
    //OLED_DrawStr(save_data, X-15, Y+30, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

void SAVE_USB(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 25
    OLED_DrawStr("Сохранение...", X, Y, 1);
    OLED_UpdateScreen();

    backup_records_to_external();
    
    OLED_DrawStr("Готово", X+5, Y+10, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

void Screen_saver(){
    mode_redact = 4;
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
    #define X 25
    #define Y 25
    OLED_DrawStr("Введите пароль", X, Y, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

void SD_Format(){}

void Flash_Format(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    //W25_Chip_Erase();
    uint8_t len = OLED_GetWidthStr(Clear[EEPROM.len]);
    OLED_DrawStr(Clear[EEPROM.len], (winth_display-len)/2, Y, 1);
    len = OLED_GetWidthStr(POWER_NOT[EEPROM.len]);
    OLED_DrawStr(POWER_NOT[EEPROM.len], (winth_display-len)/2, Y+10, 1);


    OLED_UpdateScreen();

    W25_Chip_Erase();
    update_flash_end_ptr();
    OLED_DrawRectangleFill(0, 15, winth_display, 60, 0);
    len = OLED_GetWidthStr(READY[EEPROM.len]);
    OLED_DrawStr(READY[EEPROM.len], (winth_display-len)/2, Y, 1);
    OLED_UpdateScreen();
    osDelay(200);
}


// Тест 
void GSM_internet_test(){
    GSM_data.Status |= HTTP_SEND;
}
void GSM_HTTP_READ(){
    GSM_data.Status |= HTTP_READ;
}

// Обновление ПО 
void Programm_Update_USB(){}

void sleep(){
    // Сохранения конфига 
    EEPROM.Mode = 1;
    EEPROM_SaveSettings(&EEPROM);
    if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_RESET_PROG, DATA_RESET_PROG);
    HAL_PWR_DisableBkUpAccess();
    NVIC_SystemReset();
    osDelay(1000);
    ERRCODE.STATUS |= STATUS_CRITICAL_ERROR;
}


extern EEPROM_Settings_item EEPROM;
void Reset_settings(){

    EEPROM.time_sleep_h = 1; // Время сна устройства (часы)
    EEPROM.time_sleep_m = 0; // Время сна устройства (минуты)
    EEPROM.MAX_LVL = 15;    // Максимальный уровень (например, 15 метров) ВПИ
    EEPROM.ZERO_LVL = 0;    // Нулевое значение (например, 0 метров) НПИ
    EEPROM.Mode = 0;              // Текущий режим работы
    EEPROM.Communication = 1;     // Включен GSM или нет
    EEPROM.RS485_prot = 0;        // Протокол RS-485
    EEPROM.units_mes = 1;         // Единицы измерения (по умолчанию метры)
    EEPROM.screen_sever_mode = 1; // Включить или нет заставку при включении
    EEPROM.USB_mode = 0;          // Режим работы USB
    EEPROM.len = 0;               // Язык меню
    EEPROM.mode_ADC = 1;          // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    EEPROM_SaveSettings(&EEPROM);


    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 33
    if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawStr("Ошибка сброса", X, Y, 1);
    }
    else OLED_DrawStr("Сброс успешен", X, Y, 1);
    OLED_UpdateScreen();
    osDelay(200);
}
void ALL_Reset_settings(){

    EEPROM.time_sleep_h = 1; // Время сна устройства (часы)
    EEPROM.time_sleep_m = 0; // Время сна устройства (минуты)
    EEPROM.MAX_LVL = 15;    // Максимальный уровень (например, 15 метров) ВПИ
    EEPROM.ZERO_LVL = 0;    // Нулевое значение (например, 0 метров) НПИ
    EEPROM.Mode = 0;              // Текущий режим работы
    EEPROM.Communication = 1;     // Включен GSM или нет
    EEPROM.RS485_prot = 0;        // Протокол RS-485
    EEPROM.units_mes = 1;         // Единицы измерения (по умолчанию метры)
    EEPROM.screen_sever_mode = 1; // Включить или нет заставку при включении
    EEPROM.USB_mode = 0;          // Режим работы USB
    EEPROM.len = 0;               // Язык меню
    EEPROM.mode_ADC = 1;          // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл

    strcpy(EEPROM.version.VERSION_PROGRAMM, "0.15b");
    strcpy(EEPROM.version.VERSION_PCB, "3.75-A001V");
    strcpy(EEPROM.version.password, "1234");
    strcpy(EEPROM.version.time_work_char, "0");
    for (int i = 0; i<4; i++) EEPROM.last_error_code[i] = 0; // Последний код ошибки
    EEPROM.time_work_h = 0;                            // Время работы устройства (часы)
    EEPROM.time_work_m = 0;                            // Время работы устройства (минуты)
    EEPROM.time_sleep_h = 1; // Время сна устройства (часы)
    EEPROM.time_sleep_m = 0; // Время сна устройства (минуты)

    strcpy(EEPROM.Phone, "+79150305966");


    // Параметры АЦП:
    EEPROM.ADC_ION = 1.17;     // Напряжение ИОН АЦП
    EEPROM.ADC_RESISTOR = 49.99; // Сопротивление резистора
    EEPROM.GVL_correct = 0;    // Коррекция нулевой точки (смещение ± от текущего значения) УГВ
    EEPROM.k_koeff = 0;        // Коэффициэнт наклона линейной зависимости (по 2 точкам, 20мА и 4мА)
    EEPROM.MAX_LVL = 15;       // Максимальный уровень (например, 15 метров) ВПИ
    EEPROM.ZERO_LVL = 0;       // Нулевое значение (например, 0 метров) НПИ
    EEPROM.GVL_correct_4m = 0.004;    // Реальные 4мА
    EEPROM.GVL_correct_20m = 0.02;   // Реальные 20мА
    // Коррекция температуры (смещение):
    EEPROM.Crorrect_TEMP_A = 0; // Смещение датчика аналоговой температуры
    EEPROM.Colibrate_koeff = 1.0;

    // Параметры select_bar:
    EEPROM.Mode = 0;              // Текущий режим работы (0 - режим текущие показания, 1 - циклический режим 2 - режим выставки)
    EEPROM.Communication = 1;     // Включен GSM или нет
    EEPROM.RS485_prot = 0;        // Протокол RS-485
    EEPROM.units_mes = 1;         // Единицы измерения (по умолчанию метры)
    EEPROM.screen_sever_mode = 1; // Включить или нет заставку при включении
    EEPROM.USB_mode = 0;          // Режим работы USB
    EEPROM.Save_in = 0;          // Куда сохранять данные 0 - FLASH, 1 - SD, 2 - USB, 3 - Сайт
    EEPROM.len = 0;               // Язык меню
    EEPROM.mode_ADC = 0;           // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    EEPROM.block = 0;              // Блокировка устройства 1 - заблокировано 0 - разблокировано
    EEPROM_SaveSettings(&EEPROM);


    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    #define X 20
    #define Y 33
    if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawStr("Ошибка сброса", X, Y, 1);
    }
    else OLED_DrawStr("Сброс успешен", X, Y, 1);
    OLED_UpdateScreen();
    osDelay(200);
}


#define X_col 25
#define Y_col 20
// ! Нужна проверка на работоспособность
void colibrate_4ma(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if ((ADC_data.ADC_Current < 0.0035) || (ADC_data.ADC_Current > 0.0045)){
        // Ошибка
        OLED_DrawStr("Вне диапазона", X_col, Y_col, 1);
        OLED_UpdateScreen();
        return;
    }
    EEPROM.GVL_correct_4m = ADC_data.ADC_Current;
    OLED_DrawStr("Калибровка 4мА успешна", X_col, Y_col, 1);

    EEPROM_SaveSettings(&EEPROM);
    if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawStr("Ошибка сохранения", X_col, Y_col+10, 1);
    }
    OLED_DrawStr("Сохранение успешно", X_col, Y_col+10, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

void colibrate_20ma(){
    mode_redact = 2;
    OLED_Clear(0);
    FontSet(font);
    Display_TopBar(selectedMenuItem);
    if ((ADC_data.ADC_Current < 0.019) || (ADC_data.ADC_Current > 0.025)){
        // Ошибка
        OLED_DrawStr("Вне диапазона", X_col, Y_col, 1);
        OLED_UpdateScreen();
        return;
    }
    EEPROM.GVL_correct_20m = ADC_data.ADC_Current;
    OLED_DrawStr("Калибровка 4мА успешна", X_col, Y_col, 1);

    EEPROM_SaveSettings(&EEPROM);
    if (!EEPROM_CheckDataValidity()){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        OLED_DrawStr("Ошибка сохранения", X_col, Y_col+10, 1);
    }
    OLED_DrawStr("Сохранение успешно", X_col, Y_col+10, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

// ! Нужна проверка на работоспособность
void temperature_colibrate(){

}

// Отображение окна с заставкой
void Instruction(){
    mode_redact = 2;
    OLED_Clear(0);
    menuItem *menu_s = (menuItem *)(selectedMenuItem);
    FontSet(font);
    Display_TopBar(menu_s);
    OLED_DrawXBM(10, 23, QR);
    #define X 50
    #define Y 28
    OLED_DrawStr("Инструкция", X, Y, 1);
    OLED_DrawStr("по QR-коду", X, Y+10, 1);
    OLED_UpdateScreen();
    osDelay(200);
}

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


void split_double(double *number, int32_t *int_part, int32_t *frac_part, uint8_t precision) {
    if (number == NULL || int_part == NULL || frac_part == NULL) {
        return; // Защита от передачи NULL-указателей
    }
    // Извлекаем целую часть
    *int_part = (int32_t)(*number); // Сохраняем знак в целой части
    // Вычисляем дробную часть
    double fractional = *number - (double)(*int_part);
    // Умножаем дробную часть на 10^precision и округляем
    double scaling_factor = pow(10, precision);
    *frac_part = (int32_t)(fractional * scaling_factor + 0.5); // Округление дробной части
}


// Вывод занчений data_in вне режима редоктирования
void Data_in_no_redact(menuItem *menu, int pos_y){
    char string[20] = {'\0'};
    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        char buffer[20] = {'\0'};
        char separat[20] = {'\0'}; // массив разделителей, в рамках данного блока, разделителей нету
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

    uint8_t len_add_signa = 0;
    len_add_signa = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
    uint8_t len_string = OLED_GetWidthStr(string);
    OLED_DrawStr(string, winth_display - (len_string + len_add_signa + 4), pos_y * dist_y + height_up_menu, 1);
}

// Вывод занчений data_in в режиме редоктирования
void Data_in_redact(menuItem *menu, int pos_y){
    
    char string[11] = {'\0'};
    uint8_t len_sig = 0;
    uint8_t separator_count = 0;
    if (menu->data_in->data_type[0] == 2) if (menu->data_in->unsigned_signed[1] == 1) string[0] = '-'; else string[0] = '+';
    len_sig = OLED_GetWidthStr(string);

    for (int i = 0; i < menu->data_in->Number_of_cells; i++)
    {
        menu->data_in->data_temp[i][menu->data_in->len_data_zero[i]] = '\0'; // на всякий случай
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

/*  тип меню (char)0b543210 РЕДАКТИРОВАНИЕ ДЕЙСТВИЙ 
    6 - вкладка                                             0x40
    5 - ввод числа                                          0x20
    4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
    3 - вывод незменяемого значвения char[]                 0x08
    0 - по нажатию - действие                               0x01
*/
void Select_diplay_functions(menuItem *menu, int pos_y)
{
    uint8_t leng_font = 0;
    uint8_t len_signat = 0;
    len_signat = OLED_GetWidthStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len]);
    if (len_signat > 0) len_signat += 4;
    OLED_DrawStr(menu->add_signat->Name[*menu->add_signat->data][EEPROM.len], winth_display-len_signat, pos_y * dist_y + height_up_menu, 1);
    
    
    // Вывод режима (прокрутка)
    if (menu->select_bar != (void *)&NULL_ENTRY)
    {   
        uint8_t num_menu = *menu->select_bar->data;
        if ((selectedMenuItem == menu) && (mode_redact == 1)) num_menu = Intermediate;

        int len_dat = OLED_GetWidthStr(menu->select_bar->Name[num_menu][EEPROM.len]);
        if ((mode_redact == 1) && (selectedMenuItem == menu))
        {
            OLED_DrawStr(menu->select_bar->Name[num_menu][EEPROM.len], winth_display - len_dat - 8, pos_y * dist_y + height_up_menu, 1);  // вывод если включен выбор (редактирование)
            int pos_cursor = select_menu_in_page * dist_y + height_up_menu + 2;
            int x_left = winth_display-len_dat-11;
            int x_right = winth_display-7;
            if (menu->select_bar->Name[Intermediate+1][0][0] != '\0') OLED_DrawTriangleFill(x_right, pos_cursor - 1, x_right, pos_cursor + 3, x_right+2, pos_cursor+1);
            if (Intermediate != 0) OLED_DrawTriangleFill(x_left, pos_cursor - 1, x_left, pos_cursor + 3, x_left-2, pos_cursor+1);
        }
        else{
            OLED_DrawStr(menu->select_bar->Name[num_menu][EEPROM.len], winth_display - len_dat - 4, pos_y * dist_y + height_up_menu, 1); // вывод если нет выбора 
        }
    }
}

void Display_punkt_menu(menuItem *menu, int pos_y) // отображение одного пункта меню
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
        // режим не редактирования
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
    //OLED_DrawStr("005ч08м", 1, 1, 1);
/*
    W25_Ini();
    unsigned int id = W25_Read_ID();
    sprintf(str, "ID Value: 0x%X\n", id);
    OLED_DrawStr(str, 1, 1, 1);
*/


    sprintf(str, "%d", IntADC.ADC_AKB_Proc);
    if (IntADC.ADC_AKB_Proc < 10){
        str[1] = '%';
        str[2] = '\0';
    }
    else if (IntADC.ADC_AKB_Proc < 100)
    {
        str[2] = '%';
        str[3] = '\0';
        right_ot -= 6;
    }
    else
    {
        str[3] = '%';
        str[4] = '\0';
        right_ot -= 12;
    }

    OLED_DrawStr(str, right_ot, top_akb_status + 1, 1);
    right_ot -= width_akb_status;

    int c = IntADC.ADC_AKB_Proc * 5 / 100 + 1;
    if (IntADC.ADC_AKB_Proc == 0)
        c = 0;
    // OLED_DrawRectangle(right_ot+2, top_akb_status+7-c, right_ot+3, top_akb_status+2+c);
    for (; c > 0; c--)
    {
        OLED_DrawHLine(right_ot + 2, top_akb_status + 8 - c, 2, 1);
    }
    OLED_DrawXBM(right_ot, top_akb_status, akb);
    right_ot -= width_GSM_status;



    if (GSM_data.GSM_Signal_Level_3 == -2)
    {
        right_ot -= 7;
        OLED_DrawStr("SIM?\0", right_ot, top_GSM_status, 1);
    }
    if (GSM_data.GSM_Signal_Level_3 == -1)
    {
        right_ot += 3;
        OLED_DrawXBM(right_ot, top_GSM_status, no_signal);
    }
    
    const uint8_t* signal_icons[] = {signal_0, signal_1, signal_2, signal_3};
    //GSM_data.GSM_Signal_Level = 3;
    if (GSM_data.GSM_Signal_Level_3 >= 0 && GSM_data.GSM_Signal_Level_3 <= 3) {
        OLED_DrawXBM(right_ot, top_GSM_status, signal_icons[GSM_data.GSM_Signal_Level_3]);
    }
    right_ot = winth_display - 12 - 2; // Ширина экрана минус 2 символа - процент заряда (0-9%) и - 2 отступ справа
    //Вариант оптимизации
}

uint8_t speed_update = 0; // Есть ли на страницы пункты меню требующие быстрого обновления? (100мс)
// отображение всех пунктов меню на странице
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

// вход в режим редактирования, если есть соответствующее поле
void mode_check()
{
    if ((selectedMenuItem->data_in != (void *)&NULL_ENTRY) || (selectedMenuItem->select_bar != (void *)&NULL_ENTRY))
    {
        pos_redact = 0;
        // ввод значений
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


// Все операции рабюоты со знаком можно и нужно оптимизировать
void Save_general_format(){
    uint8_t presence_of_number = 0; // проверка на не 0
    // Проверка на неверное значение
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
void Save_time_sleep_format(){
    // Проверка на неверное значение
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
    // Запись данных в изначальные величины и вызов необходимых функций

    if (selectedMenuItem->data_in != (void *)&NULL_ENTRY){ 
        uint8_t len_witout_separator = selectedMenuItem->data_in->len_data_zero[0] + selectedMenuItem->data_in->len_data_zero[1] + selectedMenuItem->data_in->len_data_zero[2];
        pos_redact = len_witout_separator - 1;

        if (selectedMenuItem->data_in->end_redact_func != '\0') selectedMenuItem->data_in->end_redact_func(); // вызов функции для сохранения изменений
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
        uint8_t pos_search = 0; // позиция до которой нужно не допустить перемещение

        pos_redact--;
        led_cursor = 0;
        if (pos_redact < pos_search)
        {
            HAL_TIM_Base_Stop_IT(&htim6);
            TIM6->SR &= ~TIM_SR_UIF;
            mode_redact = 0;
            led_cursor = 1;
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

/**
 * Переключает символ c из латиницы в кириллицу и обратно (Windows-1251).
 * D -> Г, d -> г, и т.д.
 * "Лишние" русские буквы (после 26-й) отправляем в 'Z'/'z'.
 */
static unsigned char switch_char(unsigned char c)
{
    // Заглавные латинские [A..Z] = [65..90]
    if (c >= 65 && c <= 90)
    {
        unsigned char index = c - 65;     // 0..25
        // Прибавляем к 'А'(192). Если вдруг что-то вышло за предел 25, 
        // принудительно ставим последнюю «допустимую» букву (Я) или логику ниже
        if (index < 26) 
            c = 192 + index; // 192..217
        else
            c = 223;         // 'Я'
    }
    // Заглавные русские [А..Я] = [192..223]
    else if (c >= 192 && c <= 223)
    {
        unsigned char index = c - 192;    // 0..31
        // Если индекс больше 25 (т.е. Э, Ю, Я …), отправляемся в 'Z' (90)
        if (index < 26) 
            c = 65 + index;  // 65..90
        else
            c = 90;          // 'Z'
    }
    // Строчные латинские [a..z] = [97..122]
    else if (c >= 97 && c <= 122)
    {
        unsigned char index = c - 97;     // 0..25
        if (index < 26) 
            c = 224 + index; // 224..249
        else
            c = 255;         // 'я'
    }
    // Строчные русские [а..я] = [224..255]
    else if (c >= 224 && c <= 255)
    {
        unsigned char index = c - 224;    // 0..31
        // Если индекс > 25 (э, ю, я …) => 'z'
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
    
    // если тип данных char
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
// события по нажатию кнопки на клавиатуре
void Keyboard_processing()
{
    if (Keyboard_press_code != 0xFF)
    {
        time_counter = 0;
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
            if (Keyboard_press_code == 'L') mode_redact = 0;
            pass[index_pass] = Keyboard_press_code;
            index_pass++;
            if (index_pass>4) index_pass--;
            if ((strcmp(pass, password) == 0) && (index_pass == 4)){
                selectedMenuItem->Num_menu = select_menu_in_page;
                select_menu_in_page = 0;
                menuChange(selectedMenuItem->Child);
                mode_redact = 0;
            }
            Keyboard_press_code = 0xFF;
            return;
        }
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
                press_plus_minus();
            }

            // Левая часть панели
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