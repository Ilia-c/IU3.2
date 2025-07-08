#include "Settings.h"
#include <time.h> // Для работы с функциями времени (при необходимости)
#include "RTC_data.h"
#include "MS5193T.h" // Для функции Read_MS5193T_Data

////////////////////////////////////////////////////////////////////////////////
// Функция обновления GSM (пример)
// Используется для обновления и преобразования значений GSM
////////////////////////////////////////////////////////////////////////////////


char save_data[CMD_BUFFER_SIZE] __attribute__((section(".ram2"))); // Переменная в которой собирается набор данных для сохранения и передачи на сайт
////////////////////////////////////////////////////////////////////////////////
// Глобальная структура настроек, сохраняемых в EEPROM
////////////////////////////////////////////////////////////////////////////////
const char VERSION_PROGRAMM[20] = DEFAULT_VERSION_PROGRAMM;
EEPROM_Settings_item EEPROM = {
    .version = {
        // Текущая версия устройства
        .VERSION_PCB = DEFAULT_VERSION_PCB,    // Версия печатной платы
        .password = DEFAULT_PASSWORD,
    },
    .last_error_code = DEFAULT_LAST_ERROR_CODE, // Последний код ошибки
    .DEBUG_CATEG = DEBUG_NONE,
    .DEBUG_LEVL = DEBUG_LEVL_1,
    .DEBUG_Mode = USB_SNIFFING,

    // Вводимые данные:
    .time_sleep_h = DEFAULT_TIME_SLEEP_H,        // Время сна устройства (часы)
    .time_sleep_m = DEFAULT_TIME_SLEEP_M,        // Время сна устройства (минуты)

    .Phone = DEFAULT_PHONE,
    
    // Параметры АЦП:
    .ADC_ION = DEFAULT_ADC_ION,                     // Напряжение ИОН АЦП
    .ADC_RESISTOR = DEFAULT_ADC_RESISTOR,           // Сопротивление резистора
    .GVL_correct = DEFAULT_GVL_CORRECT,             // Коррекция нулевой точки (смещение ± от текущего значения) УГВ
    .k_koeff = DEFAULT_K_KOEFF,                     // Коэффициэнт наклона линейной зависимости (по 2 точкам, 20мА и 4мА)
    .MAX_LVL = DEFAULT_MAX_LVL,                     // Максимальный уровень (например, 15 метров) ВПИ
    .ZERO_LVL = DEFAULT_ZERO_LVL,                   // Нулевое значение (например, 0 метров) НПИ
    .GVL_correct_4m = DEFAULT_GVL_CORRECT_4M,       // Реальные 4мА
    .GVL_correct_20m = DEFAULT_GVL_CORRECT_20M,     // Реальные 20мА
    // Коррекция температуры (смещение):
    .Crorrect_TEMP_A = DEFAULT_CRORRECT_TEMP_A,     // Смещение датчика аналоговой температуры
    .Colibrate_koeff = DEFAULT_COLIBRATE_KOEFF,     // Коэффициэнт калибровки 24 вольт

    // Параметры select_bar:
    .Mode = DEFAULT_MODE,                           // Текущий режим работы (0 - режим текущие показания, 1 - циклический режим, 2 - режим выставки)
    .Communication = DEFAULT_COMMUNICATION,         // Включен GSM или нет
    .RS485_prot = DEFAULT_RS485_PROT,               // Протокол RS-485
    .units_mes = DEFAULT_UNITS_MES,                 // Единицы измерения (по умолчанию метры)
    .screen_sever_mode = DEFAULT_SCREEN_SEVER_MODE, // Включить или нет заставку при включении
    .USB_mode = DEFAULT_USB_MODE,                   // Режим работы USB
    .Save_in = DEFAULT_SAVE_IN,                     // Куда сохранять данные: 0 - FLASH, 1 - SD, 2 - USB, 3 - Сайт
    .len = DEFAULT_LEN,                             // Язык меню
    .mode_ADC = DEFAULT_MODE_ADC,                   // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    .block = DEFAULT_BLOCK                          // Блокировка устройства: 1 - заблокировано, 0 - разблокировано
};
////////////////////////////////////////////////////////////////////////////////
// Описание структуры ERRCODE
////////////////////////////////////////////////////////////////////////////////
ERRCODE_item ERRCODE = {
    .STATUS = 0x00,    // Глобальный статус (каждый бит - состояние блока)
    .STATUSCHAR = '\0', // Глобальный статус в виде строки
    .STATE_CAHAR = '\0',
    .Diagnostics_char = "ND"
};
////////////////////////////////////////////////////////////////////////////////
// Глобальная структура для данных АЦП
////////////////////////////////////////////////////////////////////////////////
ADC_MS5193T_item ADC_data = {
    .ADC_status_char = "ND",   // Статус АЦП
    .ADC_value = 0,            // Значение АЦП
    .ADC_Volts = 0,            // Напряжение на токовом шунте
    .ADC_Current = 0,          // Ток на токовом шунте
    .ADC_SI_value = 0,         // Выходное значение без коррекции по уровню
    .ADC_SI_value_correct = 0, // Выходное значение с корректировкой по уровню
    .Status = 0,               // Статус работы АЦП: 0 - ERR, 1 - WAR, 2 - OK, 3 - выкл

    .Temp_correct_A = &EEPROM.Crorrect_TEMP_A, // Коррекция температуры (смещение)
    .ADC_ION = &EEPROM.ADC_ION,           // Указатель на напряжение ИОН АЦП
    .ADC_RESISTOR = &EEPROM.ADC_RESISTOR, // Указатель на сопротивление резистора
    .PPM = 10,                            // PPM (по умолчанию)
    .mode = &EEPROM.mode_ADC,             // Режим работы АЦП: 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    .GVL_correct = &EEPROM.GVL_correct,   // Указатель на коррекцию нулевой точки (смещение ±)
    .k_koeff = &EEPROM.k_koeff,           // Указатель на коэффициэнт наклона линейной зависимости
    .MAX_LVL = &EEPROM.MAX_LVL,           // Указатель на максимальный уровень (ВПИ)
    .ZERO_LVL = &EEPROM.ZERO_LVL,         // Указатель на нулевое значение (НПИ)

    .MAX_LVL_char = {-9999, -9999},            // Установка максимального уровня (целая и дробная части)
    .ZERO_LVL_char = {-9999, -9999},           // Установка минимального уровня (целая и дробная части)
    .GVL_correct_char = {-9999, -9999},        // Коррекция нулевой точки (смещение ±) – дробная часть
    .Temp_correct = {-9999, -9999},        // Коррекция нулевой точки термометра (смещение ±) – дробная часть

    
    .ADC_value_char = "ND",            // Значение АЦП в виде строки
    .ADC_Volts_char = "ND",            // Напряжение на токовом шунте в виде строки
    .ADC_Current_char = "ND",          // Ток на токовом шунте в виде строки
    .ADC_SI_value_char = "ND",         // Выходное значение без коррекции в виде строки
    .ADC_SI_value_correct_char = "ND", // Выходное значение с корректировкой в виде строки

    .ADC_MS5193T_temp = 0,         // Температура на аналоговом датчике
    .ADC_MS5193T_temp_char = "ND", // Температура на аналоговом датчике в виде строки

    .update_value = Read_MS5193T_Data // Ссылка на функцию обновления (чтение данных с АЦП)
};


Internal_ADC_item IntADC = 
{
  .ADC_AKB_volts = 0, // Напряжение на АКБ
  .ADC_AKB_Proc = 0,    // Процент заряда на акб
  .Colibrate_koeff = &EEPROM.Colibrate_koeff, // Колибровочный коэффициэнт
  .MK_VBAT = 0,          // Напряжение на CR2032
  .MK_temp= 0,             // Температура, измеренная МК
  .MK_temp_char = "ND",  // Температура МК в виде строки
  .MK_vbat_char = "ND",  // VBAT МК в виде строки
  .ADC_AKB_volts_char = "ND", // Напряжение на АКБ строка
  .ADC_AKB_Proc_char = "ND"  // Процент заряда на акб строка
};
////////////////////////////////////////////////////////////////////////////////
// Глобальная структура для данных GSM
////////////////////////////////////////////////////////////////////////////////
GSM_STATUS_item GSM_data = {
    .Status                = 0,                              // Нет установленных битов статуса
    .GSM_Signal_Level      = 99,                             // Если нет сети – значение -1
    .GSM_Signal_Level_3   = -2,                              // Если нет сети – значение 99
    .GSM_Signal_Errors     = 0,                              // Нет ошибок
    .Operator_code = 0,

    .Modem_mode = MODEM_STATUS[2],                          // "ND" – режим работы (GSM/NBIOT)
    .GSM_status_char       = (char *)STATUS_CHAR[3],         // "ND" – статус не определён
    .GSM_SIMCARD_char      = (char *)SIM_STATUS[2],          // "UNKNOWN" – статус SIM не определён
    .GSM_status_ready_char = (char *)GSM_READY_STATUS[1],    // "NRDY" – GSM не готов
    .GSM_status_reg_char   = (char *)GSM_REG_STATUS[2],      // "UNKNOWN" – регистрация неизвестна
    .GSM_region_char       = (char *)Countries[0].iso,       // Регион, например, "UNDEF" (Countries[0])
    .GSM_operator_char     = (char *)GSM_Operators[0].name,  // Оператор, например, "Undefine" (GSM_Operators[0])
    .GSM_signal_lvl_char   = "ND",                           // Текстовое представление уровня сигнала
    .GSM_err_lvl_char   = "ND",                           // Текстовое представление уровня сигнала
    .GSM_gprs_on_char      = (char *)GPRS_STATUS[1],         // "DISCONNECTED" – GPRS не подключён
    .GSM_sms_status      = "",         // Статус отправки смс
    .GSM_site_status      = "",         // Статус запроса на сайт
    .GSM_site_read_status = "",

    .GSM_LastResponseTime  = 0,                              // Время последнего ответа – 0 секунд
    .update_value          = Update_Data                            // Функция обновления значений
};


////////////////////////////////////////////////////////////////////////////////
// Глобальная переменная конфигурации
////////////////////////////////////////////////////////////////////////////////
uint16_t time_update_display = 20000; // Время обновления экрана (для обновления времени и курсора)
uint8_t USB_TERMINAL_STATUS = TERMINAL_DISABLE; // Статус USB: 0 - не подключен, 1 - подключен, 2 - ошибка подключения
////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные для функционирования
////////////////////////////////////////////////////////////////////////////////
char Keyboard_press_code = 0x00;     // Код нажатой клавиши на клавиатуре
char EEPROM_status_char[10] = "ND"; // Статус доступности EEPROM
char FLASH_status_char[10] = "ND";  // Статус доступности FLASH
char time_work_char[15] = "ND"; // Время работы в виде строки в часах
uint32_t time_work = 0;         // Время работы устройства в секундах
////////////////////////////////////////////////////////////////////////////////
// Дата и время (RTC)
////////////////////////////////////////////////////////////////////////////////
RTC_TimeTypeDef Time = {0}; // Дата
RTC_DateTypeDef Date = {0}; // Время

RTC_TimeTypeDef Time_start = {0}; // Дата старта МК
RTC_DateTypeDef Date_start  = {0}; // Время старта МК
////////////////////////////////////////////////////////////////////////////////
// Глобальные константы
////////////////////////////////////////////////////////////////////////////////
const uint16_t Timer_key_press = 400;                      // Время ожидания до быстрого проматывания
const uint16_t Timer_key_press_fast = 120;                 // Время быстрого нажатия клавиши
// #define MAX_PASSWPRD_LEN 10
char password[MAX_PASSWPRD_LEN] = "UO";

// Буфер для приёма данных по USB
uint8_t g_myRxBuffer[MY_USB_RX_BUFFER_SIZE]  __attribute__((section(".ram2"))); // Максимальное количество байт для сбора данных по USB

const char MODEM_STATUS[3][6] = {"GSM", "NBIOT", "ND"}; // Индикация статуса блока "GSM", "NBIOT", "ND"
const char STATUS_CHAR[4][5] = {"OK", "ERR", "WAR", "ND"}; // Индикация статуса блока "OK", "ERR", "WAR", "ND"
const char SIM_STATUS[3][8] = {"OK", "NO", "ND"};// Статус SIM-карты: PRESENT – установлена, ABSENT – не установлена, UNKNOWN – неизвестно 
const char GSM_READY_STATUS[2][5] = {"RDY", "NRDY"};// Статус готовности GSM: RDY – готов, NRDY – не готов
const char GSM_REG_STATUS[3][8] = {"REG", "NREG", "ND"};// Статус регистрации в сети: REG – зарегистрирован, NREG – не зарегистрирован, UNKNOWN – неизвестно
const char GPRS_STATUS[2][5] = {"CONN", "DISC"}; // Статус GPRS соединения: CONNECTED – установлено, DISCONNECTED – разорвано

const GSM_Operator_item GSM_Operators[] = {
    {0, "ND"},       // Необрежеленный
    // Российские операторы (MCC = 250)
    {25001, "MTS"},       // МТС (MNC 01)
    {25002, "Megafon"},   // МегаФон (MNC 02)
    {25004, "Yota"},      // Yota (MNC 04)
    {25007, "Tele2"},     // Tele2 (MNC 07)
    {25099, "Beeline"},   // Beeline (MNC 99)
    // Известные мировые операторы:
    // США
    {310410, "AT&T"},         // AT&T
    {311480, "Verizon"},      // Verizon
    {310260, "T-Mobile US"},  // T-Mobile
    // Великобритания
    {23415, "Vodafone UK"},   // Vodafone
    {23410, "EE"},            // EE (MCC 234, MNC 10)
    // Германия
    {26201, "T-Mobile DE"},   // T-Mobile Germany
    {26202, "Vodafone DE"},   // Vodafone Germany
    // Франция
    {20801, "Orange"},        // Orange
    // Италия
    {22201, "TIM Italy"},     // Telecom Italia Mobile
};

const Country_operator_item Countries[] = {
    {0, "ND"},
    {212, "AFG"}, // Afghanistan (MCC 412) – здесь выбран представительский номер: 412 ? AFG
    {276, "ALB"}, // Albania (MCC 276)
    {603, "DZA"}, // Algeria (MCC 603)
    {213, "AND"}, // Andorra (MCC 213)
    {631, "AGO"}, // Angola (MCC 631)
    {344, "ATG"}, // Antigua and Barbuda (MCC 344)
    {722, "ARG"}, // Argentina (MCC 722)
    {283, "ARM"}, // Armenia (MCC 283)
    {363, "ABW"}, // Aruba (MCC 363)
    {505, "AUS"}, // Australia (MCC 505)
    {232, "AUT"}, // Austria (MCC 232)
    {400, "AZE"}, // Azerbaijan (MCC 400)
    {364, "BHS"}, // Bahamas (MCC 364)
    {426, "BHR"}, // Bahrain (MCC 426)
    {470, "BGD"}, // Bangladesh (MCC 470)
    {342, "BRB"}, // Barbados (MCC 342)
    {257, "BLR"}, // Belarus (MCC 257)
    {206, "BEL"}, // Belgium (MCC 206)
    {702, "BLZ"}, // Belize (MCC 702)
    {616, "BEN"}, // Benin (MCC 616)
    {350, "BMU"}, // Bermuda (MCC 350)
    {402, "BTN"}, // Bhutan (MCC 402)
    {736, "BOL"}, // Bolivia (MCC 736)
    {218, "BIH"}, // Bosnia and Herzegovina (MCC 218)
    {652, "BWA"}, // Botswana (MCC 652)
    {724, "BRA"}, // Brazil (MCC 724)
    {995, "IOT"}, // British Indian Ocean Territory (MCC 995)
    {348, "VGB"}, // British Virgin Islands (MCC 348)
    {528, "BRN"}, // Brunei (MCC 528)
    {284, "BGR"}, // Bulgaria (MCC 284)
    {613, "BFA"}, // Burkina Faso (MCC 613)
    {642, "BDI"}, // Burundi (MCC 642)
    {456, "KHM"}, // Cambodia (MCC 456)
    {624, "CMR"}, // Cameroon (MCC 624)
    {302, "CAN"}, // Canada (MCC 302)
    {625, "CPV"}, // Cape Verde (MCC 625)
    {346, "CYM"}, // Cayman Islands (MCC 346)
    {623, "CAF"}, // Central African Republic (MCC 623)
    {622, "TCD"}, // Chad (MCC 622)
    {730, "CHL"}, // Chile (MCC 730)
    {460, "CHN"}, // China (MCC 460)
    {732, "COL"}, // Colombia (MCC 732)
    {654, "COM"}, // Comoros (MCC 654)
    {629, "COG"}, // Congo (MCC 629)
    {548, "COK"}, // Cook Islands (MCC 548)
    {712, "CRI"}, // Costa Rica (MCC 712)
    {219, "HRV"}, // Croatia (MCC 219)
    {368, "CUB"}, // Cuba (MCC 368)
    {280, "CYP"}, // Cyprus (MCC 280)
    {230, "CZE"}, // Czech Republic (MCC 230)
    {238, "DNK"}, // Denmark (MCC 238)
    {638, "DJI"}, // Djibouti (MCC 638)
    {366, "DMA"}, // Dominica (MCC 366)
    {370, "DOM"}, // Dominican Republic (MCC 370)
    {515, "TLS"}, // East Timor (Timor-Leste) (MCC 515)
    {740, "ECU"}, // Ecuador (MCC 740)
    {602, "EGY"}, // Egypt (MCC 602)
    {706, "SLV"}, // El Salvador (MCC 706)
    {627, "GNQ"}, // Equatorial Guinea (MCC 627)
    {657, "ERI"}, // Eritrea (MCC 657)
    {248, "EST"}, // Estonia (MCC 248)
    {636, "ETH"}, // Ethiopia (MCC 636)
    {750, "FLK"}, // Falkland Islands (MCC 750)
    {288, "FRO"}, // Faroe Islands (MCC 288)
    {542, "FJI"}, // Fiji (MCC 542)
    {244, "FIN"}, // Finland (MCC 244)
    {208, "FRA"}, // France (MCC 208)
    {742, "GUF"}, // French Guiana (MCC 742)
    {547, "PYF"}, // French Polynesia (MCC 547)
    {628, "GAB"}, // Gabon (MCC 628)
    {607, "GMB"}, // Gambia (MCC 607)
    {282, "GEO"}, // Georgia (MCC 282)
    {262, "DEU"}, // Germany (MCC 262)
    {620, "GHA"}, // Ghana (MCC 620)
    {266, "GIB"}, // Gibraltar (MCC 266)
    {202, "GRC"}, // Greece (MCC 202)
    {290, "GRL"}, // Greenland (MCC 290)
    {352, "GRD"}, // Grenada (MCC 352)
    {340, "GLP"}, // Guadeloupe (MCC 340)
    {310, "GUM"}, // Guam (MCC 310)
    {704, "GTM"}, // Guatemala (MCC 704)
    {234, "GGY"}, // Guernsey (MCC 234)
    {611, "GIN"}, // Guinea (MCC 611)
    {632, "GNB"}, // Guinea-Bissau (MCC 632)
    {738, "GUY"}, // Guyana (MCC 738)
    {372, "HTI"}, // Haiti (MCC 372)
    {708, "HND"}, // Honduras (MCC 708)
    {454, "HKG"}, // Hong Kong (MCC 454)
    {216, "HUN"}, // Hungary (MCC 216)
    {274, "ISL"}, // Iceland (MCC 274)
    {404, "IND"}, // India (MCC 404)
    {510, "IDN"}, // Indonesia (MCC 510)
    {432, "IRN"}, // Iran (MCC 432)
    {418, "IRQ"}, // Iraq (MCC 418)
    {272, "IRL"}, // Ireland (MCC 272)
    {234, "IMN"}, // Isle of Man (MCC 234)
    {425, "ISR"}, // Israel (MCC 425)
    {222, "ITA"}, // Italy (MCC 222)
    {338, "JAM"}, // Jamaica (MCC 338)
    {440, "JPN"}, // Japan (MCC 440)
    {234, "JEY"}, // Jersey (MCC 234)
    {416, "JOR"}, // Jordan (MCC 416)
    {401, "KAZ"}, // Kazakhstan (MCC 401)
    {639, "KEN"}, // Kenya (MCC 639)
    {545, "KIR"}, // Kiribati (MCC 545)
    {221, "KOS"}, // Kosovo (MCC 221)
    {419, "KWT"}, // Kuwait (MCC 419)
    {437, "KGZ"}, // Kyrgyzstan (MCC 437)
    {457, "LAO"}, // Laos (MCC 457)
    {247, "LVA"}, // Latvia (MCC 247)
    {415, "LBN"}, // Lebanon (MCC 415)
    {651, "LSO"}, // Lesotho (MCC 651)
    {618, "LBR"}, // Liberia (MCC 618)
    {606, "LBY"}, // Libya (MCC 606)
    {295, "LIE"}, // Liechtenstein (MCC 295)
    {246, "LTU"}, // Lithuania (MCC 246)
    {270, "LUX"}, // Luxembourg (MCC 270)
    {455, "MAC"}, // Macau (MCC 455)
    {294, "MKD"}, // North Macedonia (MCC 294)
    {646, "MDG"}, // Madagascar (MCC 646)
    {650, "MWI"}, // Malawi (MCC 650)
    {502, "MYS"}, // Malaysia (MCC 502)
    {472, "MDV"}, // Maldives (MCC 472)
    {278, "MLT"}, // Malta (MCC 278)
    {551, "MHL"}, // Marshall Islands (MCC 551)
    {340, "MTQ"}, // Martinique (MCC 340)
    {609, "MRT"}, // Mauritania (MCC 609)
    {617, "MUS"}, // Mauritius (MCC 617)
    {175, "MYT"}, // Mayotte (MCC 175)
    {334, "MEX"}, // Mexico (MCC 334)
    {550, "FSM"}, // Micronesia (Federated States of) (MCC 550)
    {259, "MDA"}, // Moldova (MCC 259)
    {250, "RUS"}, // Russia (MCC 250)
    {635, "RWA"}, // Rwanda (MCC 635)
    {226, "ROU"}, // Romania (MCC 226)
    {214, "ESP"}, // Spain (MCC 214)
    {413, "LKA"}, // Sri Lanka (MCC 413)
    {634, "SDN"}, // Sudan (MCC 634)
    {746, "SUR"}, // Suriname (MCC 746)
    {653, "SWZ"}, // Swaziland/Eswatini (MCC 653)
    {240, "SWE"}, // Sweden (MCC 240)
    {228, "CHE"}, // Switzerland (MCC 228)
    {417, "SYR"}, // Syria (MCC 417)
    {466, "TWN"}, // Taiwan (MCC 466)
    {436, "TJK"}, // Tajikistan (MCC 436)
    {640, "TZA"}, // Tanzania (MCC 640)
    {520, "THA"}, // Thailand (MCC 520)
    {615, "TGO"}, // Togo (MCC 615)
    {554, "TKL"}, // Tokelau (MCC 554)
    {539, "TON"}, // Tonga (MCC 539)
    {374, "TTO"}, // Trinidad and Tobago (MCC 374)
    {605, "TUN"}, // Tunisia (MCC 605)
    {286, "TUR"}, // Turkey (MCC 286)
    {438, "TKM"}, // Turkmenistan (MCC 438)
    {376, "TCA"}, // Turks and Caicos Islands (MCC 376)
    {553, "TUV"}, // Tuvalu (MCC 553)
    {641, "UGA"}, // Uganda (MCC 641)
    {255, "UKR"}, // Ukraine (MCC 255)
    {424, "ARE"}, // United Arab Emirates (MCC 424)
    {234, "GBR"}, // United Kingdom (MCC 234)
    {310, "USA"}, // United States (MCC 310)
    {748, "URY"}, // Uruguay (MCC 748)
    {434, "UZB"}, // Uzbekistan (MCC 434)
    {541, "VUT"}, // Vanuatu (MCC 541)
    {225, "VAT"}, // Vatican City (MCC 225)
    {734, "VEN"}, // Venezuela (MCC 734)
    {452, "VNM"}, // Vietnam (MCC 452)
    {543, "WLF"}, // Wallis and Futuna (MCC 543)
    {421, "YEM"}, // Yemen (MCC 421)
    {645, "ZMB"}, // Zambia (MCC 645)
    {648, "ZWE"}, // Zimbabwe (MCC 648)

    // Дополнительные территории:
    {308, "SPM"}, // Saint Pierre and Miquelon (MCC 308)
    {362, "SXM"}, // Sint Maarten (MCC 362)
    {332, "VIR"}  // United States Virgin Islands (MCC 332)
};
const int GSM_OperatorsCount = sizeof(GSM_Operators) / sizeof(GSM_Operators[0]);
const int CountriesCount     = sizeof(Countries) / sizeof(Countries[0]);
