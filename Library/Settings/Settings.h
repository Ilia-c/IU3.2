#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "RTC_data.h"
#include "MS5193T.h"

#ifdef __cplusplus
extern "C"
{
#endif

// define для режима работы
#define REDACT_MODE 0          // режим редактирования
#define CYCLE_MODE 1           // режим цикла
#define FIRMWARE_UPDATE_MODE 2 // режим обновления прошивки

#define MY_USB_RX_BUFFER_SIZE 512 // Максимум, что хотим собрать в USB (байт)

  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структур EEPROM_Settings и Prgramm_version
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct Prgramm_version
  {
    char VERSION_PROGRAMM[10];
    char VERSION_PCB[10];
    char time_work_char[10];
    uint8_t VER_PCB_IDEOLOGY;
    uint8_t VER_PCB_VERSION;
    char VER_PCB_INDEX[5];
  } Prgramm_version_item;

  // Структура сохраняемая в EEPROM
  typedef struct EEPROM_Settings
  {
    Prgramm_version_item version; // Текущая версия устройства
    char last_error_code[4];      // Последний код ошибки
    uint32_t time_work_h;         // Время работы устройства часы
    uint32_t time_work_m;         // Время работы устройства минуты

    /*-----------------*/
    // Вводимые данные //
    /*-----------------*/
    uint16_t time_sleep_h; // Время сна устройства часы
    uint16_t time_sleep_m; // Время сна устройства минуты
    //  АЦП  //
    double ADC_ION;     // Напряжение ИОН АЦП
    float ADC_RESISTOR; // Сопротивление резистора
    double GVL_correct; // Коррекция нулевой точки (смещение +- от текущего значения) УГВ
    double k_koeff;     // Коэффициэнт наклона линейной зависисимости. (По 2 точкам, 20мА и 4мА)
    double MAX_LVL;     // Максимальный уровень (например 15 метров) ВПИ
    double ZERO_LVL;    // Нулевое значение     (например 0 метров)  НПИ
    // коррекция температуры (смещение) //
    double Crorrect_TEMP_A; // Смещение датчика аналогового температуры
    double Crorrect_TEMP_D; // Смещение датчика цифрового температуры

    /*-----------------*/
    // select_bar      //
    /*-----------------*/
    uint8_t Mode;              // текущий режим работы
    uint8_t Communication;     // Включен GSM или нет
    uint8_t RS485_prot;        // Протокол RS-485
    uint8_t units_mes;         // по умолчанию метры, еденицы измерения
    uint8_t screen_sever_mode; // Включить или нет заставку при включении
    uint8_t USB_mode;          // режим работы USB
    uint8_t len;               // Язык меню
    uint8_t mode_ADC;          // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
  } EEPROM_Settings_item;


  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структуры ERRCODE
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ERRCODE_
  {
    uint64_t STATUS;     // Глобальный статус (каждый бит - состояние блока)
    char STATUSCHAR[12]; // Глобальный статус в виде строки
  } ERRCODE_item;

  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структуры ADC_MS5193T
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ADC_MS5193T
  {
    char ADC_status_char[5];       // Статус GSM
    int32_t ADC_value;           // Значение АЦП
    double ADC_Volts;            // Напряжение на токовом шунте
    double ADC_Current;          // Ток на токовом шунте
    double ADC_SI_value;         // Выходное значение без коррекции по уровню
    double ADC_SI_value_correct; // Выходное значение с корректировкой по уровню
    uint8_t Status;              // Статус работы АЦП - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл

    double *ADC_ION;
    float *ADC_RESISTOR;
    int32_t PPM;
    uint8_t *mode; // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    // Корректировка УГВ
    double *GVL_correct; // Коррекция нулевой точки (смещение +- от текущего значения)
    double *k_koeff;     // Коэффициэнт наклона линейной зависисимости. (По 2 точкам, 20мА и 4мА)
    // Калибровка
    double *MAX_LVL;  // Максимальный уровень (например 15 метров) ВПИ
    double *ZERO_LVL; // Нулевое значение     (например 0 метров)  НПИ

    // Вывод значений (отображение)
    int32_t MAX_LVL_char[2];            // Установка максиального уровня (1 - до запятой | 2 - после запятой)
    int32_t ZERO_LVL_char[2];           // Установка минимального уровня (1 - 0мА/4мА  | 2 - до запятой | 3 - после запятой)
    int32_t UP_LEVEL_CORRECT_char[2];   // Коррекция максиального уровня (20мА) - учитывает смещение от этого уровня
    int32_t DOWN_LEVEL_CORRECT_char[2]; // Коррекция минимального уровня (0мА/4мА)  - учитывает смещение от этого уровня
    int32_t GVL_correct_char[2];        // Коррекция нулевой точки (смещение +- от текущего значения) дробная часть

    char ADC_value_char[15];            // Значение АЦП в виде строки
    char ADC_Volts_char[15];            // Напряжение на токовом шунте в виде строки
    char ADC_Current_char[15];          // Ток на токовом шунте в виде строки
    char ADC_SI_value_char[15];         // Выходное значение без коррекции в виде строки
    char ADC_SI_value_correct_char[15]; // Выходное значение с коррекцией в виде строки

    double ADC_MS5193T_temp;        // Температура на аналоговом датчике
    char ADC_MS5193T_temp_char[15]; // Температура на аналоговом датчике в виде строки

    void (*update_value)(void); // Ссылка на функцию обновления (чтение данных с АЦП)
  } ADC_MS5193T_item;

  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структуры GSM_STATUS
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct GSM_STATUS
  {
    uint8_t Status; // Статус работы GSM - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    uint8_t mode;   // Режим работы GSM, 0 - вкл, 2 - выкл

    // Значение Связи (регистрация в сети, уровень сигнала GSM, уровень сигнала (палочки), оператор)
    uint8_t GSM_Signal;      // Код сигнала от самого модуля GSM (0-31 и 99)
    int8_t GSM_Signal_Level; // Уровень сигнала GSM (0..3) или -1, если нет регистрации

    char GSM_status_char[5];       // Статус GSM
    char GSM_SIMCARD_char[5];      // Видит ли GSM SIM?
    char GSM_status_ready_char[5]; // Готов ли GSM?
    char GSM_status_reg_char[5];   // Зарегистрирован ли GSM в сети
    char GSM_operator_char[10];    // Название оператора MTS, Beeline и т.д.
    char GSM_signal_lvl_char[2];   // Уровень сигнала 0-99
    char GSM_gprs_on_char[3];      // Включен ли gprs?

    void (*update_value)(void); // Ссылка на функцию обновления GSM (перевод в текст)
  } GSM_STATUS_item;
  
  ////////////////////////////////////////////////////////////////////////////////
  //  Структуры с данными
  ////////////////////////////////////////////////////////////////////////////////
  extern ADC_MS5193T_item ADC_data;
  extern GSM_STATUS_item GSM_data;
  extern ERRCODE_item ERRCODE;

  ////////////////////////////////////////////////////////////////////////////////
  // Глобальные переменные РЕЖИМЫ
  ////////////////////////////////////////////////////////////////////////////////
  extern uint8_t Mode;
  extern uint8_t Communication;
  extern uint8_t RS485_prot;
  extern uint8_t units_mes;
  extern uint8_t screen_sever_mode;
  extern uint8_t USB_mode;

  ////////////////////////////////////////////////////////////////////////////////
  // Глобальные переменные КОНФИГУРАЦИИ
  ////////////////////////////////////////////////////////////////////////////////
  extern uint16_t time_update_display; // Время обновления экрана (для обновления времени и курсора)

  ////////////////////////////////////////////////////////////////////////////////
  // Глобальные переменные ДЛЯ ФУНКЦИОНИРОВАНИЯ
  ////////////////////////////////////////////////////////////////////////////////
  extern char Keyboard_press_code;   // Код нажатой клавиши на клавиатуре
  extern float ADC_AKB_volts;        //-
  extern int ADC_AKB_Proc;           //-
  extern int ADC_AKB_cell;           //- ------------- Статус акб и батарейки
  extern char ADC_AKB_volts_char[4]; //-
  extern char ADC_AKB_Proc_char[4];  //-
  extern char error_code[4];         // Код глобальной ошибки
  extern double OneWire_temp;        // Температура OneWire
  extern char OneWire_temp_char[5];  // Температура OneWire в виде строки

  extern char EEPROM_status_char[3]; // Статус доступности EEPROM
  extern char FLASH_status_char[3];  // Статус доступности FLASH
  extern char SD_status_char[3];     // Статус доступности SD

  extern RTC_TimeTypeDef Time; // Дата
  extern RTC_DateTypeDef Date; // Время
  ////////////////////////////////////////////////////////////////////////////////
  // Глобальные КОНСТАНТЫ
  ////////////////////////////////////////////////////////////////////////////////
  extern const char STATUS_CHAR[4][5]; // Индикация статуса блока
  extern const uint16_t Timer_key_one_press;
  extern const uint16_t Timer_key_press;
  extern const uint16_t Timer_key_press_fast;

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
