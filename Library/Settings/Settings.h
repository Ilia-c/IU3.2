#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include "RTC_data.h"

#ifdef __cplusplus
extern "C"
{
#endif

// Глобальный статус (каждый бит - состояние блока)
extern uint64_t STATUS;

// define для режима работы
#define REDACT_MODE 0          // режим редактирования
#define CYCLE_MODE 1           // режим цикла
#define FIRMWARE_UPDATE_MODE 2 // режим обновления прошивки

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
    int32_t ADC_value;           // Значение АЦП
    float ADC_Volts;             // Напряжение на токовом шунте
    float ADC_Current;           // Ток на токовом шунте
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

    char ADC_status_char[5];
    char ADC_value_char[11];           // Значение АЦП в виде строки
    char ADC_Volts_char[5];            // Напряжение на токовом шунте
    char ADC_Current_char[5];          // Ток на токовом шунте
    char ADC_SI_value_char[5];         // Выходное значение без коррекции
    char ADC_SI_value_correct_char[5]; // Выходное значение с коррекцией

    void (*update_value)(void); // Ссылка на функцию обновления (чтение данных с АЦП)
  } ADC_MS5193T_item;

  typedef struct Prgramm_version
  {
    char VERSION_PROGRAMM[10];
    char VERSION_PCB[10];
    char time_work_char[10];
    uint8_t VER_PCB_IDEOLOGY;
    uint8_t VER_PCB_VERSION;
    char VER_PCB_INDEX[5];
  } Prgramm_version_item;

  ////////////////////////////////////////////////////////////////////////////////
  // Глобальные переменные (extern), чтобы использовать их в других файлах .c
  ////////////////////////////////////////////////////////////////////////////////

  // Время обновления экрана (для обновления времени и курсора)
  extern uint16_t time_update_display;


  // Код текущей операции
  extern uint32_t OPERATION_CODE;

  // Внутренний АЦП
  extern float ADC_in_temp;
  extern float ADC_MS5193T_temp;
  extern float OneWire_temp;
  extern char ADC_in_temp_char[5];
  extern char ADC_MS5193T_temp_char[11];
  extern char OneWire_temp_char[5];

  // Индикация статуса блока
  extern char STATUS_CHAR[4][5];

  // Структуры с данными
  extern ADC_MS5193T_item ADC_data;
  extern GSM_STATUS_item GSM_data;

  // Статус памяти
  extern char EEPROM_status_char[3];
  extern char FLASH_status_char[3];
  extern char SD_status_char[3];

  // Режим экрана (включен/выключен)
  extern uint8_t display_status;

  // Нажатые клавиши на клавиатуре
  extern char Keyboard_press_code;
  extern uint8_t Display_update;

  // Статус акб и батарейки
  extern float ADC_AKB_volts;
  extern int ADC_AKB_Proc;
  extern int ADC_AKB_cell;
  extern char ADC_AKB_volts_char[4];
  extern char ADC_AKB_Proc_char[4];

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

  extern const uint16_t Timer_key_one_press;
  extern const uint16_t Timer_key_press;
  extern const uint16_t Timer_key_press_fast;

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

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
