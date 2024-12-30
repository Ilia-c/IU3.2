#include "Settings.h"
#include <time.h>      // Если вам нужны функции времени
#include "RTC_data.h"  // Опционально, если нужны детали из RTC_data

////////////////////////////////////////////////////////////////////////////////
//               Статус периферии (реальные определения)
////////////////////////////////////////////////////////////////////////////////

// Реализация функции (для примера)
void Nuss()
{
    // Если нужно, здесь код обновления или преобразования значений
}

// Блок GSM и АЦП определять заново не нужно (структуры описаны в Settings.h),
// но создаём "экземпляры" глобальных переменных:

// Время обновления экрана
uint16_t time_update_display = 20000; // Время обновления экрана (для обновления времени и курсора)

// ГЛОБАЛЬНЫЙ СТАТУС, каждый бит - состояние блока
/* НАЧАЛО ОПИСАНИЯ STATUS
   0 -                       
   1 -                       
   2 -                       
   3 -                       
   4 -                       
   5 -                       
   6 -                       
   7 -                       
   8 -                       
   9 -                       
   10 -                       
   11 -                       
   12 -                       
   13 -                       
   14 -                       
   15 -                       
   16 -                       
   17 -                       
   18 -                       
   19 -                       
   20 -                       
   21 -                       
   22 -                       
   23 -                       
   24 -                       
   25 -                       
   26 -                       
   27 -                       
   28 -                       
   29 -                       
   30 -                       
   31 -
 КОНЕЦ ОПИСАНИЯ STATUS */
uint32_t STATUS = 0x00000000;

// Код текущей операции
/* НАЧАЛО ОПИСАНИЯ КОДА ОПЕРАЦИИ
   0
   КОНЕЦ ОПИСАНИЯ КОДА ОПЕРАЦИИ
*/
uint16_t OPERATION_CODE = 0x0000;


// Внутренний АЦП
float ADC_in_temp           = 0;                  
float ADC_MS5193T_temp      = 0;             
float OneWire_temp          = 0;                 
char  ADC_in_temp_char[5]   = {'\0'};      
char  ADC_MS5193T_temp_char[11] = {'\0'}; 
char  OneWire_temp_char[5]  = {'\0'};     


// Индикация статуса блока
char STATUS_CHAR[3][5] = {
    "OK",
    "ERR",
    "WAR"
};

// Экземпляр структуры АЦП
ADC_MS5193T_item ADC_data = {
    .ADC_value = 0, // Значение АЦП
    .ADC_Volts = 0, // Напряжение на токовом шунте
    .ADC_Current = 0, // Ток на токовом шунте

    .ADC_SI_value = 0, // Выходное значение без коррекции
    .ADC_SI_value_correct = 0, // Выходное значение с корректировкой

    .ADC_ION = 1.17, // Опорный источник
    .ADC_RESISTOR = 49.99, // Сопротивление шунта
    .PPM = 10,    // PPM
    .Status = 0, // Статус работы АЦП - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    .mode = 2, // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    
    
    .GVL_correct = 0,
    .ADC_correct_zero_0_4 = 4,
    .ADC_correct_max_20 = 20,

    // Калибровка
    .UP_LEVEL_CORRECT = 0,        // Коррекция максиального уровня (при 20мА)
    .DOWN_LEVEL_CORRECT = {0, 0}, // Коррекция минимального уровня (0мА и 4мА)

    .MAX_LVL = 0,
    .ZERO_LVL = 0,

    .MAX_LVL_char = {-9999, 0},
    .ZERO_LVL_char = {0, 0},
    .UP_LEVEL_CORRECT_char = {0, 0},  
    .DOWN_LEVEL_CORRECT_char = {0, 0},
    .GVL_correct_char = {0, 0},

    // Вывод значений
    .ADC_status_char = "ND", // АЦП в виде строки
    .ADC_value_char = "ND", // АЦП в виде строки
    .ADC_Volts_char = "ND", // Напряжение
    .ADC_Current_char = "ND", // Ток
    .ADC_SI_value_char = "ND", // Выход без коррекции
    .ADC_SI_value_correct_char = "ND", // Выход с коррекцией
    .update_value = Nuss // Ссылка на функцию чтения/обновления АЦП
};

// Экземпляр структуры GSM
GSM_STATUS_item GSM_data = {
    0, // Статус работы GSM - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    0, // Режим работы GSM, 0 - вкл, 2 - выкл

    99, // Код сигнала (0..31, 99)
    .GSM_Signal_Level = -1, // Уровень сигнала GSM (0..3) или -1 (нет регистрации)

    "ND", // Статус GSM
    "ND", // Видит ли GSM SIM?
    "ND", // Готов ли GSM?
    "ND", // Зарегистрирован ли GSM
    "ND", // Название оператора
    "ND", // Уровень сигнала 0..99
    "ND", // GPRS?
    Nuss  // Функция обновления
};

// Экземпляр структуры GSM
Prgramm_version_item Prog_ver = {
    .VERSION_PROGRAMM = "0.15b",
    .VERSION_PCB = "3.7-001",
    .VER_PCB_IDEOLOGY = 3,
    .VER_PCB_VERSION = 7,
    .VER_PCB_INDEX = "-001",
    .time_work_h = 0,
    .time_work_m = 0,
    .time_work_char = "1",
    .update_value = Nuss  // Функция обновления
};



// Статус памяти
char EEPROM_status_char[3] = "ERR";
char FLASH_status_char[3]  = "ERR";
char SD_status_char[3]     = "ERR";

// Режим экрана
uint8_t display_status     = 0;

// Нажатые клавиши
char    Keyboard_press_code= 0;
uint8_t Display_update     = 0;

// Статус АКБ и батарейки
float  ADC_AKB_volts       = 0;  
int    ADC_AKB_Proc        = 0;   
int    ADC_AKB_cell        = 0;   
char   ADC_AKB_volts_char[4] = {'\0'}; 
char   ADC_AKB_Proc_char[4]  = {'\0'}; 
char error_code[4]         = {'\0'};


////////////////////////////////////////////////////
//            Настроечные параметры
////////////////////////////////////////////////////




// Дата и время
RTC_TimeTypeDef Time = {0};
RTC_DateTypeDef Date = {0};



uint32_t time_work_h              = 0;       
uint8_t  time_work_m              = 0;       
char     time_work_char[10]       = "\0";    

char data_add_unit[3]            = "\0";    

// Выбираемые значения
uint8_t Mode                     = 0;  
uint8_t Communication            = 0;  
uint8_t RS485_prot               = 0;  
uint8_t units_mes                = 1;   // по умолчанию метры
uint8_t screen_sever_mode        = 1;  
uint8_t USB_mode                 = 0;  


char len                         = 0x00;

// Настройка времени сна
uint16_t time_sleep_h            = 2; 
uint16_t time_sleep_m            = 0; 

uint16_t Timer_key_one_press     = 50;  
uint16_t Timer_key_press         = 300; 

