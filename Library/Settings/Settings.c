#include "Settings.h"
#include <time.h>     // Если вам нужны функции времени
#include "RTC_data.h" // Опционально, если нужны детали из RTC_data

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


uint64_t STATUS = 0x00;


// Код текущей операции
/* НАЧАЛО ОПИСАНИЯ КОДА ОПЕРАЦИИ
   0
   КОНЕЦ ОПИСАНИЯ КОДА ОПЕРАЦИИ
*/
uint32_t OPERATION_CODE = 0x0000;

// Внутренний АЦП
float ADC_in_temp = 0;
float ADC_MS5193T_temp = 0;
float OneWire_temp = 0;
char ADC_in_temp_char[5] = {'\0'};
char ADC_MS5193T_temp_char[11] = {'\0'};
char OneWire_temp_char[5] = {'\0'};

EEPROM_Settings_item EEPROM = {
    .version =
        {
            .VERSION_PROGRAMM = "0.15b",
            .VERSION_PCB = "3.7-001",
            .time_work_char = "0",
            .VER_PCB_IDEOLOGY = 3,
            .VER_PCB_VERSION = 7,
            .VER_PCB_INDEX = "-001"
        },
    .last_error_code = {0x00, 0x00, 0x00, 0x00},
    .time_work_h = 0,
    .time_work_m = 0,
    .time_sleep_h = 1, // Время сна устройства часы
    .time_sleep_m = 0, // Время сна устройства минуты

    //  АЦП  //
    .GVL_correct = 0,          // Коррекция нулевой точки (смещение +- от текущего значения)
    .ADC_ION = 1.75,              // Напряжение ИОН АЦП
    .ADC_RESISTOR = 100,          // Сопротивление резистора
    .MAX_LVL = 15,                // Максимальный уровень (например 15 метров) ВПИ
    .ZERO_LVL = 7,                // Нулевое значение     (например 0 метров)  НПИ
    // коррекция температуры (смещение) //
    .Crorrect_TEMP_A = 0, // Смещение датчика аналогового температуры
    .Crorrect_TEMP_D = 0, // Смещение датчика цифрового температуры

    // select_bar
    .Mode = 0,              // текущий режим работы/ 0 - настройка (экран) 1 - цикл 2 - обновление (см define в settings.h)
    .Communication = 0,     // Включен GSM или нет
    .RS485_prot = 0,        // Протокол RS-485
    .units_mes = 1,         // по умолчанию метры, еденицы измерения
    .screen_sever_mode = 0, // Включить или нет заставку при включении
    .USB_mode = 0,          // режим работы USB 
    .len = 0,               // Язык меню
    .mode_ADC = 1,
};

// Индикация статуса блока
char STATUS_CHAR[4][5] = {"OK", "ERR", "WAR"};

// Экземпляр структуры АЦП
ADC_MS5193T_item ADC_data = {
    .ADC_value = 0,   // Значение АЦП
    .ADC_Volts = 0,   // Напряжение на токовом шунте
    .ADC_Current = 0, // Ток на токовом шунте
    .ADC_SI_value = 0,         // Выходное значение без коррекции
    .ADC_SI_value_correct = 0, // Выходное значение с корректировкой
    .Status = 0,                          // Статус работы АЦП - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл

    .ADC_ION = &EEPROM.ADC_ION,           // Опорный источник
    .ADC_RESISTOR = &EEPROM.ADC_RESISTOR, // Сопротивление шунта
    .PPM = 10,          // PPM
    .mode = &EEPROM.mode_ADC,                            // Режим работы АЦП, 0 - 4-20мА, 1 - 0-20мА, 2 - выкл
    .GVL_correct = &EEPROM.GVL_correct,
    .k_koeff = &EEPROM.k_koeff,
    // Калибровка
    .MAX_LVL = &EEPROM.MAX_LVL,
    .ZERO_LVL = &EEPROM.ZERO_LVL,
    
    .MAX_LVL_char = {-9999, -9999},
    .ZERO_LVL_char = {-9999, -9999},
    .UP_LEVEL_CORRECT_char = {-9999, -9999},
    .DOWN_LEVEL_CORRECT_char = {-9999, -9999},
    .GVL_correct_char = {-9999, -9999},
    // Вывод значений
    .ADC_status_char = "ND",           // АЦП в виде строки
    .ADC_value_char = "ND",            // АЦП в виде строки
    .ADC_Volts_char = "ND",            // Напряжение
    .ADC_Current_char = "ND",          // Ток
    .ADC_SI_value_char = "ND",         // Выход без коррекции
    .ADC_SI_value_correct_char = "ND", // Выход с коррекцией
    .update_value = Nuss               // Ссылка на функцию чтения/обновления АЦП
};

// Экземпляр структуры GSM
GSM_STATUS_item GSM_data = {
    0, // Статус работы GSM - 0 - ERR,  1 - WAR, 2 - OK, 3 - выкл
    0, // Режим работы GSM, 0 - вкл, 2 - выкл

    99,                     // Код сигнала (0..31, 99)
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
// Статус памяти
char EEPROM_status_char[3] = "ERR";
char FLASH_status_char[3] = "ERR";
char SD_status_char[3] = "ERR";

// Режим экрана
uint8_t display_status = 0;

// Нажатые клавиши
char Keyboard_press_code = 0x00;
uint8_t Display_update = 0;

// Статус АКБ и батарейки
float ADC_AKB_volts = 0;
int ADC_AKB_Proc = 0;
int ADC_AKB_cell = 0;
char ADC_AKB_volts_char[4] = {'\0'};
char ADC_AKB_Proc_char[4] = {'\0'};
char error_code[4] = {'\0'};

////////////////////////////////////////////////////
//            Настроечные параметры
////////////////////////////////////////////////////

// Дата и время
RTC_TimeTypeDef Time = {0};
RTC_DateTypeDef Date = {0};

char time_work_char[10] = "\0";
char data_add_unit[3] = "\0";

const uint16_t Timer_key_one_press = 20;
const uint16_t Timer_key_press = 600;
const uint16_t Timer_key_press_fast = 80;

#define MY_USB_RX_BUFFER_SIZE 512 // Максимум, что хотим собрать
uint8_t g_myRxBuffer[MY_USB_RX_BUFFER_SIZE];
