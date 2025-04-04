#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "RTC_data.h"
#include "MS5193T.h"
#include "GSM.h"



// define для режима работы
#define REDACT_MODE 0          // режим редактирования
#define CYCLE_MODE 1           // режим цикла
#define FIRMWARE_UPDATE_MODE 2 // режим обновления прошивки

#define MY_USB_RX_BUFFER_SIZE 512 // Максимум, что хотим собрать в USB (байт)

#define BKP_REG_INDEX_RESET_PROG  RTC_BKP_DR1 // Регистр для проверки, что сброс для входа в режим Цикл
#define DATA_RESET_PROG  0xDEADBEEF


#define RAM2_SECTION __attribute__((section(".ram2")))
  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структур EEPROM_Settings и Prgramm_version
  ////////////////////////////////////////////////////////////////////////////////
  extern char save_data[CMD_BUFFER_SIZE];

#define Mode_set 0
#define Mode_cycle 0
#define Mode_demo 0



  #define BKP_REG_INDEX_ERROR_CODE_1  RTC_BKP_DR2
  #define BKP_REG_INDEX_ERROR_CODE_2  RTC_BKP_DR3
  // Коды для разных Fault'ов:
  #define FAULT_CODE_NMI            1U
  #define FAULT_CODE_HARDFAULT      2U
  #define FAULT_CODE_MEMMANAGE      3U
  #define FAULT_CODE_BUSFAULT       4U
  #define FAULT_CODE_USAGEFAULT     5U

  typedef struct Prgramm_version
  {
    char VERSION_PCB[11];
    char password[10];
    char time_work_char[10];
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

    char Phone[20];      // Последний код ошибки
    //  АЦП  //
    double ADC_ION;     // Напряжение ИОН АЦП
    float ADC_RESISTOR; // Сопротивление резистора
    double GVL_correct; // Коррекция нулевой точки (смещение +- от текущего значения) УГВ
    double GVL_correct_4m;    // Реальные 4мА
    double GVL_correct_20m;   // Реальные 20мА
    double k_koeff;     // Коэффициэнт наклона линейной зависисимости. (По 2 точкам, 20мА и 4мА)
    double MAX_LVL;     // Максимальный уровень (например 15 метров) ВПИ
    double ZERO_LVL;    // Нулевое значение     (например 0 метров)  НПИ
    // коррекция температуры (смещение) //
    double Crorrect_TEMP_A; // Смещение датчика аналогового температуры
    double Colibrate_koeff;

    /*-----------------*/
    // select_bar      //
    /*-----------------*/
    uint8_t Mode;              // текущий режим работы
    uint8_t Communication;     // Включен GSM или нет
    uint8_t RS485_prot;        // Протокол RS-485
    uint8_t units_mes;         // по умолчанию метры, еденицы измерения
    uint8_t screen_sever_mode; // Включить или нет заставку при включении
    uint8_t USB_mode;          // режим работы USB
    uint8_t Save_in;          // режим работы USB
    uint8_t len;               // Язык меню
    uint8_t mode_ADC;          // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    uint8_t block;            // Блокировка устройства
  } EEPROM_Settings_item;


  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структуры ERRCODE
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ERRCODE_
  {
    uint64_t STATUS;     // Глобальный статус (каждый бит - состояние блока)
    char STATUSCHAR[20]; // Глобальный статус в виде строки
  } ERRCODE_item;

  ////////////////////////////////////////////////////////////////////////////////
  //               Описание структуры ADC_MS5193T
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ADC_MS5193T
  {
    char ADC_status_char[10];       // Статус GSM
    int32_t ADC_value;           // Значение АЦП
    double ADC_Volts;            // Напряжение на токовом шунте
    double ADC_Current;          // Ток на токовом шунте
    double ADC_SI_value;         // Выходное значение без коррекции по уровню
    double ADC_SI_value_correct; // Выходное значение с корректировкой по уровню
    uint8_t Status;              // Статус работы АЦП - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл

    double *Temp_correct_A; // Коррекция температуры (смещение)
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
    int32_t GVL_correct_char[2];        // Коррекция нулевой точки (смещение +- от текущего значения) дробная часть
    int32_t Temp_correct[2];        // Коррекция нулевой точки термометра (смещение +- от текущего значения) дробная часть

    char ADC_value_char[15];            // Значение АЦП в виде строки
    char ADC_Volts_char[15];            // Напряжение на токовом шунте в виде строки
    char ADC_Current_char[15];          // Ток на токовом шунте в виде строки
    char ADC_SI_value_char[15];         // Выходное значение без коррекции в виде строки
    char ADC_SI_value_correct_char[15]; // Выходное значение с коррекцией в виде строки

    double ADC_MS5193T_temp;        // Температура на аналоговом датчике
    char ADC_MS5193T_temp_char[15]; // Температура на аналоговом датчике в виде строки

    void (*update_value)(void); // Ссылка на функцию обновления (чтение данных с АЦП)
  } ADC_MS5193T_item;

  typedef struct Internal_ADC
  {
    float ADC_AKB_volts; // Напряжение на АКБ
    uint8_t ADC_AKB_Proc;    // Процент заряда на акб
    double *Colibrate_koeff; // Колибровочный коэффициэнт
    uint16_t R1;
    uint16_t R2;
    float VBAT;          // Напряжение на CR2032
    char ADC_AKB_volts_char[6]; // Напряжение на АКБ строка
    char ADC_AKB_Proc_char[6];  // Процент заряда на акб строка
  } Internal_ADC_item;
  extern Internal_ADC_item IntADC;
  ////////////////////////////////////////////////////////////////////////////////
  //     Описание структуры GSM_STATUS
  ////////////////////////////////////////////////////////////////////////////////
#define GSM_RDY (1UL << 0)              // Модуль включился
#define SIM_PRESENT (1UL << 1)          // SIM-карта установлена
#define NETWORK_REGISTERED (1UL << 2)   // Устройство зарегистрировано в сети
#define SIGNAL_PRESENT (1UL << 3)       // Сигнал сети присутствует
#define OPERATOR_IDENTIFIED (1UL << 4)  // Оператор сети опознан
#define SMS_SENT_SUCCESS (1UL << 5)     // Отправка SMS успешна
#define DATA_REQUEST_SUCCESS (1UL << 6) // Запрос данных выполнен успешно
#define OPERATION_SEND_COMPLETED (1UL << 7)  // Операция успешно отправлена !!!
#define RESPONSE_RECEIVED (1UL << 8)    // Получен ответ на команду
#define GPRS_CONNECTED (1UL << 9)       // GPRS соединение установлено
#define GPRS_DISCONNECTED (1UL << 10)    // GPRS соединение разорвано
#define HTTP_SEND (1UL << 11)    // отправить http запрос
#define HTTP_READ (1UL << 12)    // отправить http запрос (чтение данных)
#define SMS_SEND (1UL << 13)    // ОТПРАВИТЬ SMS
#define HTTP_SEND_Successfully  (1UL << 14)    //  http запрос успешен
#define HTTP_READ_Successfully (1UL << 15)    // http запрос (чтение данных) успешен
#define SMS_SEND_Successfully (1UL << 16)    //  SMS успешено отправлено
#define NETWORK_REGISTERED_SET_HTTP (1UL << 17)   // Устройство зарегистрировано в сети для регистрации HTTP при включении
#define DATA_READ (1UL << 18) // Сброс активного состояния

  typedef struct GSM_STATUS_item
  {
    uint32_t Status;                // Статус работы в виде битовой маски
    int8_t GSM_Signal_Level;        // Уровень сигнала GSM или -1, если нет регистрации
    int8_t GSM_Signal_Level_3;      // Уровень сигнала GSM или для отображения в виде значков
    int8_t GSM_Signal_Errors;       // Ошибки сигнала GSM или -1, если нет регистрации
    uint32_t Operator_code;         // Ошибки сигнала GSM или -1, если нет регистрации

    char *GSM_status_char;          // "OK", "ERR", "WAR", "ND" – индикатор статуса блока
    char *Modem_mode;
    char *GSM_SIMCARD_char;         // Например, SIM_STATUS[0] для PRESENT или SIM_STATUS[1] для ABSENT
    char *GSM_status_ready_char;    // Например, GSM_READY_STATUS[0] для RDY
    char *GSM_status_reg_char;      // Например, GSM_REG_STATUS[0] для REG
    char *GSM_region_char;          // Строка, определяющая регион (например, ISO код страны)
    char *GSM_operator_char;        // Название оператора (будет меняться во время работы)
    char GSM_signal_lvl_char[3];    // Текстовое представление уровня сигнала
    char GSM_err_lvl_char[3];       // Текстовое представление ошибок сигнала
    char *GSM_gprs_on_char;         // Например, GPRS_STATUS[0] или GPRS_STATUS[1]
    char GSM_sms_status[10];         // Статус отправки смс
    char GSM_site_status[10];         // Статус запроса на сайт
    char GSM_site_read_status[10];

    uint32_t GSM_LastResponseTime;  // Время последнего ответа (секунды)
    void (*update_value)(void);     // Функция обновления значений
  } GSM_STATUS_item;

typedef struct GSM_Operator_item
{
  const uint16_t code;    // Код оператора, получаемый от модема
  const char name[10]; // Название оператора (например, "MTS", "Beeline")
} GSM_Operator_item;

typedef struct Country_operator_item
{
  const uint16_t mcc;         // Мобильный код страны (MCC)
  const char iso[10]; // ISO 3166-1 alpha-3 код страны
}Country_operator_item;

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


extern char password[5];
////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные КОНФИГУРАЦИИ
////////////////////////////////////////////////////////////////////////////////
extern uint16_t time_update_display; // Время обновления экрана (для обновления времени и курсора)

////////////////////////////////////////////////////////////////////////////////
// Глобальные переменные ДЛЯ ФУНКЦИОНИРОВАНИЯ
////////////////////////////////////////////////////////////////////////////////
extern char Keyboard_press_code;   // Код нажатой клавиши на клавиатуре
extern char error_code[4];         // Код глобальной ошибки
extern double OneWire_temp;        // Температура OneWire
extern char OneWire_temp_char[5];  // Температура OneWire в виде строки

extern char EEPROM_status_char[10]; // Статус доступности EEPROM
extern char FLASH_status_char[10];  // Статус доступности FLASH

extern RTC_TimeTypeDef Time; // Дата
extern RTC_DateTypeDef Date; // Время
extern RTC_TimeTypeDef Time_start; // Дата старта МК
extern RTC_DateTypeDef Date_start; // Время старта МК
////////////////////////////////////////////////////////////////////////////////
// Глобальные КОНСТАНТЫ
////////////////////////////////////////////////////////////////////////////////
extern const char STATUS_CHAR[4][5]; // Индикация статуса блока
extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press;
extern const uint16_t Timer_key_press_fast;
extern const char MODEM_STATUS[3][6]; // Индикация статуса блока "GSM", "NBIOT", "ND"
extern const char STATUS_CHAR[4][5]; // Индикация статуса блока "OK", "ERR", "WAR", "ND"
extern const char SIM_STATUS[3][8];// Статус SIM-карты: PRESENT – установлена, ABSENT – не установлена, UNKNOWN – неизвестно 
extern const char GSM_READY_STATUS[2][5];// Статус готовности GSM: RDY – готов, NRDY – не готов
extern const char GSM_REG_STATUS[3][8];// Статус регистрации в сети: REG – зарегистрирован, NREG – не зарегистрирован, UNKNOWN – неизвестно
extern const char GPRS_STATUS[2][5]; // Статус GPRS соединения: CONNECTED – установлено, DISCONNECTED – разорвано
extern const int GSM_OperatorsCount;
extern const int CountriesCount;
extern const Country_operator_item Countries[];
extern const GSM_Operator_item GSM_Operators[];

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
