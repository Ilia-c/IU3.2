#ifndef MENU_DATA_H
#define MENU_DATA_H

#ifdef __cplusplus
extern "C"
{
#endif
/*.
* Общий файл для всех объявлений меню
*
*
*/
#include "OLED.h"
#include "OLED_Fonts.h"
#include "OLED_Icons.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "main.h"
#include "MS5193T.h"
#include "Settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "RTC_data.h"
#include "AT24C02.h"
#include "Status_codes.h"
#include "Internal_ADC.h"
#include "w25q128.h"
#include "Settings_default.h"
#include "qr2xbm.h"
#include "Hard_fault.h"
#include "Flash_colibrate_data.h"

extern osThreadId_t  ADC_readHandle;
extern osThreadId_t  ERROR_INDICATE_taskHandle;
extern SPI_HandleTypeDef hspi2;


#define font my5x7fonts
#define winth_display 128
#define height_display 64

#define line_indentation 0                            // отступы верхней линии слева и справа
#define end_line 125                                  // отступы верхней линии слева и справа
#define line_ind_top 11                               // оттуп линии сверху
#define back_pic_pos_x 0                              // начало иконки предыдущий пункт меню x
#define back_pic_pos_y 4                              // начало иконки предыдущий пункт меню y
#define size_back_pic_x 3                             // размер треугольника по x
#define size_back_pic_y 3                             // размер треугольника по y
#define top_pic_last_munu 1                           // отступ сверху до названия предыдущего пункта меню
#define left_pic_last_munu 7                          // отступ слева до названия предыдущего пункта меню
#define top_GSM_status 2                              // отступ сверху до статуса связи
#define width_GSM_status 15                           // ширина одного значка статуса связи
#define top_akb_status 1                              // отступ сверху до уровня заряда
#define width_akb_status 7                            // ширина одного уровня заряда

#define time_led_cursor 500      // Время обновления индикации курсоора при вводе данных
#define time_updateDisplay 20000 // Время обновления экрана вне ввода данных
////////////////////////////////////////////////////
//                  Пункты меню                   //
////////////////////////////////////////////////////
#define SCROLLBAR_X      (winth_display - 7)  // Отступ от правого края экрана
#define SCROLLBAR_Y      14                   // Y-координата верхнего угла области полосы
#define SCROLLBAR_HEIGHT 45                   // Высота области полосы прокрутки
#define SCROLLBAR_WIDTH  3                    // Ширина полосы прокрутки
#define MIN_THUMB_HEIGHT 3                    // Минимальная высота ползунка

typedef struct Menu_item
{
    uint8_t *data;              // привязанное значение
    const char * const Name[10][2]; // Название пункта меню на русском и английском
} menuSelect_item;

typedef void (*DataFormatter)(char *buffer, size_t size, void *data);

typedef struct Menu_item_char
{
    char separators[3];     // разделители, 3 штуки, если нету, то '\0'. последний может использоваться в качестве приписки
    uint8_t Number_of_cells; // количетво ячеек данных
    uint8_t redact_right_end; // завершение редактирования при нажатии вправо на краю данных 1-включено, 0-выключено

    //  ЯЧЕЙКИ
    double **data_double;
    void *data[3];  // ссылки на исходные значения 
    uint8_t data_type[3]; // тип данных 0-uint8_t, 1 - uint16_t, 2 - int32_t, 3 - char[] (при таком режиме - ширина ячеек вне редактировани, а ширина ячеек будет задавать максимум символов) 4 - float
    uint8_t unsigned_signed[2]; // есть минус или нет 1-да, 0 - нет.  Первое - текущее, второе - промежуточное
    uint8_t len_data_zero[3]; // ширина ячеек (001 - ширина 3, 23 - ширина 2)
    uint8_t len_data_zero_unredact[3]; // ширина ячеек в режиме не редактирования (001 - ширина 3, 23 - ширина 2)
    char data_temp[3][11];   // промежуточные данные (для редактирования)
    int32_t UP_data[3];         // минимальные значения
    int32_t DOWN_data[3];       // максимальные значения
    void (*end_redact_func)(void);
    void (*end_redact_func_2)(double *save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed); // функция вызываемая при окончании редактирования (сохранение значений)
} menuSelect_item_char;



typedef struct MAKE_MENU
{
    const char *Name_rus;  // Название пункта меню на русском
    const char *Name_en;   // Название пункта меню на английском
    int Num_menu;             // Номер вкладки сверху при переходе
    const int data_update;             // Обновлять или нет этот пункт меню каждые 100мс (1/0 вкл/выкл)
    menuSelect_item *add_signat;       // дополнительная надпись справа

    /*  тип меню (char)0b543210
        6 - вкладка                                             0x40
        5 - ввод числа                                          0x20
        4 - изменяемые пункты (трубуется ссылка на структуру)   0x10
        3 - вывод незменяемого значвения char[]                 0x08
        2 - Ввод времени                                        0x04
        1 - Ввод даты                                           0x02
        0 - по нажатию - действие                               0x01
    */
    void *Next;     // Следующий пункт меню (Id)
    void *Previous; // Предыдущий пункт меню (Id)

    void *Parent; // Родительский пункт меню
    void *Child;  // На какой пункт меню ссылается

    void (*action)(void); // действие при нажатии
    menuSelect_item *select_bar;
    menuSelect_item_char *data_in;
    char *data_out;
} menuItem;


void InitMenus(void);
void Display_all_menu();
void Display_punkt_menu(menuItem *menu, int pos_y);
void menuChange(menuItem *NewMenu);
void Keyboard_processing();
void convert_string_to_ascii(const char *input, char *output);
char convert_to_ascii(unsigned char c);
void ADC_Init();
void data_redact_pos(char data);
void Start_video();
void Save_general_format();
void Save_time_format();
void Save_date_format();
void Programm_GVL_CORRECT();
void split_double(double *number, int32_t *int_part, int32_t *frac_part, uint8_t precision);
void SAVE_DOUBLE(double **save_data, int32_t *value_int, int32_t *value_float, int size_float, uint8_t _signed);
void Add_units(void);
void Remove_units(void);
void Display_TopBar(menuItem *CurrentMenu);
void full_test();
void USB_FLASH_SAVE();
void GSM_sms_test();
void flash_test();
void EEPROM_test();
void SAVE_IZM();
void Flash_Format();
void GSM_internet_test();
void Programm_Update_USB();
void sleep();
void Instruction();
void Reset_settings();
void colibrate_4ma();
void colibrate_20ma();
void temperature_colibrate();
void PASSWORD();
void Screen_saver();
void ALL_Reset_settings();
void Save_time_sleep_format();
void PROGRESS_BAR(uint8_t procent);
void SAVE_USB();
void GSM_HTTP_SYNC();
void colibrate_24v();
void Update_programm();
void Reset_time_work();
void DrawBack(uint8_t px, menuItem *m);
void State_update();
void QR_status();
void Add_units(void);
void Calibrate();
void OLED_DrawCenteredString_OFFSETX(char strArray[40], uint16_t Y_pos, uint8_t width_display, uint8_t offset);
void OLED_DrawCenteredString(const char strArray[][40], uint16_t Y_pos);
int YES_OR_NO(const char strArray[][40]);
bool Boot_CopyVersion(Bootloader_data_item* out, size_t out_sz);
void Reset_errors();


typedef void (*TopBarDrawFunc)(int *px, menuItem *m);
typedef struct {
    TopBarDrawFunc  draw;
} TopBarElement;

#define PASSWORD_LEN 5


extern int mode_redact;
extern char Keyboard_press_code;
extern menuItem *selectedMenuItem;
extern RNG_HandleTypeDef hrng;



void CalibrateTable(void);
void Initial_setup(void);
HAL_StatusTypeDef Collect_ADC_Data(uint8_t channel, int ref_mA);
uint8_t InputString(char *buffer, uint8_t length, const char *title);


#ifdef __cplusplus
}
#endif

#endif