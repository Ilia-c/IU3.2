#ifndef SETTINGS_H
#define SETTINGS_H

// Подключаем стандартные типы: uint8_t, int8_t, int32_t и т.д.
#include <stdint.h>
// Если нужно время, можно оставить <time.h>. 
// Здесь же можно подключать нужные HAL-заголовки (stm32fxx_hal.h) и т.п.
#include "RTC_data.h"  // Подключение вашего заголовка с RTC_TimeTypeDef, RTC_DateTypeDef

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////
// Прототип функции
////////////////////////////////////////////////////////////////////////////////
void Nuss(void);

////////////////////////////////////////////////////////////////////////////////
//               Описание структур GSM_STATUS_item и ADC_MS5193T_item
////////////////////////////////////////////////////////////////////////////////

// Блок GSM
/*
  Структура GSM_STATUS:
  - Хранит информацию о статусе GSM-модуля, уровне сигнала, имени оператора,
    SIM-карте и т.д.
  - Поле update_value указывает на функцию, преобразующую внутренние поля в текст
    (например, для отладки или вывода на экран).
*/
typedef struct GSM_STATUS
{
    uint8_t Status; // Статус работы GSM - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    uint8_t mode;   // Режим работы GSM, 0 - вкл, 2 - выкл

    // Значение Связи (регистрация в сети, уровень сигнала GSM, уровень сигнала (палочки), оператор)
    uint8_t GSM_Signal;       // Код сигнала от самого модуля GSM (0-31 и 99)
    int8_t  GSM_Signal_Level; // Уровень сигнала GSM (0..3) или -1, если нет регистрации
    
    char GSM_status_char[5];       // Статус GSM
    char GSM_SIMCARD_char[5];      // Видит ли GSM SIM?
    char GSM_status_ready_char[5]; // Готов ли GSM?
    char GSM_status_reg_char[5];   // Зарегистрирован ли GSM в сети
    char GSM_operator_char[10];    // Название оператора MTS, Beeline и т.д.
    char GSM_signal_lvl_char[2];   // Уровень сигнала 0-99
    char GSM_gprs_on_char[3];      // Включен ли gprs?

    void (*update_value)(void);    // Ссылка на функцию обновления GSM (перевод в текст)
} GSM_STATUS_item;


// Блок АЦП
/*
  Структура ADC_MS5193T:
  - Описывает данные по измеряемому току/напряжению на шунте, выходное
    значение без/с коррекцией и др.
  - Поле update_value указывает на функцию, которая производит чтение/обновление
    значений с АЦП.
*/
typedef struct ADC_MS5193T
{
    int32_t ADC_value;              // Значение АЦП
    float   ADC_Volts;              // Напряжение на токовом шунте
    float   ADC_Current;            // Ток на токовом шунте
    float   ADC_SI_value;           // Выходное значение без коррекции по уровню
    float   ADC_SI_value_correct;   // Выходное значение с корректировкой по уровню

    uint16_t ADC_RESISTOR;
    int32_t PPM;
    uint8_t Status; // Статус работы АЦП - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    uint8_t mode;   // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл

    // Корректировка УГВ
    double GVL_correct;      // Коррекция нулевой точки (смещение +- от текущего значения)

    // Калибровка
    double UP_LEVEL_CORRECT;      // Коррекция максимального уровня (при 20мА)
    double DOWN_LEVEL_CORRECT[2]; // Коррекция минимального уровня (при 0мА и 4мА)

    double MAX_LVL;  // Максимальный уровень (например 15 метров)
    double ZERO_LVL; // Нулевое значение     (например 0 метров)

    int32_t MAX_LVL_char[2];                // Установка максиального уровня (1 - до запятой | 2 - после запятой)
    int32_t ZERO_LVL_char[2];               // Установка минимального уровня (1 - 0мА/4мА  | 2 - до запятой | 3 - после запятой)
    int32_t UP_LEVEL_CORRECT_char[2];       // Коррекция максиального уровня (20мА) - учитывает смещение от этого уровня
    int32_t DOWN_LEVEL_CORRECT_char[2];     // Коррекция минимального уровня (0мА/4мА)  - учитывает смещение от этого уровня

    int32_t GVL_correct_char[2];              // Коррекция нулевой точки (смещение +- от текущего значения) дробная часть

    // Вывод значений
    char ADC_status_char[5];
    char ADC_value_char[11];               // Значение АЦП в виде строки
    char ADC_Volts_char[5];                // Напряжение на токовом шунте
    char ADC_Current_char[5];              // Ток на токовом шунте
    char ADC_SI_value_char[5];             // Выходное значение без коррекции
    char ADC_SI_value_correct_char[5];     // Выходное значение с коррекцией

    void (*update_value)(void); // Ссылка на функцию обновления (чтение данных с АЦП)
} ADC_MS5193T_item;



typedef struct Prgramm_version
{
    const char VERSION_PROGRAMM[10];
    char VERSION_PCB[22];

    uint8_t VER_PCB_IDEOLOGY;     
    uint8_t VER_PCB_VERSION;      
    char    VER_PCB_INDEX[11];  
    uint32_t time_work_h;
    uint32_t time_work_m;
    char     time_work_char[10];
    void (*update_value)(void);    // Ссылка на функцию обновления версии ПО
} Prgramm_version_item;


////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные (extern), чтобы использовать их в других файлах .c
////////////////////////////////////////////////////////////////////////////////

// Время обновления экрана (для обновления времени и курсора)
extern uint16_t time_update_display;

// Глобальный статус (каждый бит - состояние блока)
extern uint32_t STATUS;

// Код текущей операции
extern uint16_t OPERATION_CODE;

// Внутренний АЦП
extern float ADC_in_temp;                  
extern float ADC_MS5193T_temp;            
extern float OneWire_temp;                
extern char  ADC_in_temp_char[5];         
extern char  ADC_MS5193T_temp_char[5];    
extern char  OneWire_temp_char[5];        

// Индикация статуса блока
extern char STATUS_CHAR[3][5];

// Структуры с данными
extern ADC_MS5193T_item ADC_data;
extern GSM_STATUS_item  GSM_data;

// Статус памяти
extern char EEPROM_status_char[3];
extern char FLASH_status_char[3];
extern char SD_status_char[3];

// Режим экрана (включен/выключен)
extern uint8_t display_status;

// Нажатые клавиши на клавиатуре
extern char    Keyboard_press_code;
extern uint8_t Display_update;

// Статус акб и батарейки
extern float  ADC_AKB_volts;         
extern int    ADC_AKB_Proc;          
extern int    ADC_AKB_cell;          
extern char   ADC_AKB_volts_char[4]; 
extern char   ADC_AKB_Proc_char[4];  

extern char error_code[4];

//////////////////////
// Настроечные параметры
//////////////////////

// Дата и время
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;



extern char data_add_unit[3];

// Выбираемые значения
extern uint8_t Mode;               
extern uint8_t Communication;      
extern uint8_t RS485_prot;         
extern uint8_t units_mes;          
extern uint8_t screen_sever_mode;  
extern uint8_t USB_mode;           

// Вводимые значения
extern double  zero_level;         
extern int32_t zero_level_int;     
extern uint8_t zero_level_float;   

extern double  max_height;         
extern double  min_height;         
extern int32_t lvl_int;            
extern int32_t lvl_float;          

extern char len;                  

// Настройка времени сна
extern uint16_t time_sleep_h;  
extern uint16_t time_sleep_m;  

extern uint16_t Timer_key_one_press;  
extern uint16_t Timer_key_press;     


#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
