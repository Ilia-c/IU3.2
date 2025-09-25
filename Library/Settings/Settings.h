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
#include "Settings_default.h"

// define ��� ������ ������
#define REDACT_MODE 0          // ����� ��������������
#define CYCLE_MODE 1           // ����� �����
#define FIRMWARE_UPDATE_MODE 2 // ����� ���������� ��������
#define MAX_PASSWPRD_LEN 10

#define MY_USB_RX_BUFFER_SIZE 512 // ��������, ��� ����� ������� � USB (����)

#define BKP_REG_INDEX_RESET_PROG RTC_BKP_DR1 // ������� ��� ��������, ��� ����� ��� ����� � ����� ����
#define DATA_RESET_PROG 0xDEADBEEF

// ����� � ������� ����������
#define BOOTVER_ADDR ((const char *)0x0800FFC0)

#define RAM2_SECTION __attribute__((section(".ram2")))
  ////////////////////////////////////////////////////////////////////////////////
  //               �������� �������� EEPROM_Settings � Prgramm_version
  ////////////////////////////////////////////////////////////////////////////////
  extern char save_data[CMD_BUFFER_SIZE];

#define BKP_REG_INIT_FLAG RTC_BKP_DR0 // ������� ��� �������� ������������� RTC
#define BKP_REG_INDEX_ERROR_CODE_1 RTC_BKP_DR2
#define BKP_REG_INDEX_ERROR_CODE_2 RTC_BKP_DR3
#define BKP_UPDATE_REG RTC_BKP_DR5

#define BKP_REG_TIME RTC_BKP_DR7
#define BKP_REG_TIME_INIT RTC_BKP_DR8
#define BKP_REG_CNT_POWERUP RTC_BKP_DR9 // ���������� ��������� ����������
#define BKP_MAGIC 0xA5A5
#define FACTORY_SETTINGS_MAGIC 0xA5A5A5A5

#define BKP_UPDATE_FLAG 0xEAFC
// ���� ��� ������ Fault'��:
#define FAULT_CODE_NMI 1U
#define FAULT_CODE_HARDFAULT 2U
#define FAULT_CODE_MEMMANAGE 3U
#define FAULT_CODE_BUSFAULT 4U
#define FAULT_CODE_USAGEFAULT 5U

// ���� ������� USB USB_mode
#define USB_FLASH 0
#define USB_DEBUG 1
#define USB_OFF 2

// ���� ������ �������
#define DEBUG_LEVL_1 0
#define DEBUG_LEVL_2 1
#define DEBUG_LEVL_3 2
#define DEBUG_LEVL_4 3

  typedef enum
  {
    M2M_DISABLE = 0x00,
    GSM_MODE = 0x01,
    NB_IOT_MODE = 0x02,
    LORA_MODE = 0x03
  } HAL_Communication_M2M_TypeDef;

// ��������� ������� �� ������� DEBUG_CATEG
#define DEBUG_GSM (1 << 0)
#define AT_COMMANDS (1 << 1)
#define DEBUG_RS485 (1 << 2)
#define DEBUG_ADC (1 << 3)
#define DEBUG_ADC_IN (1 << 4)
#define DEBUG_FLASH (1 << 5)
#define DEBUG_EEPROM (1 << 6)
#define DEBUG_OTHER (1 << 7)
#define DEBUG_ALL (DEBUG_GSM | AT_COMMANDS | DEBUG_RS485 | DEBUG_ADC | DEBUG_ADC_IN | DEBUG_FLASH | DEBUG_EEPROM | DEBUG_OTHER)
#define DEBUG_NONE 0

// DEBUG_Mode
#define USB_SNIFFING 0
#define USB_AT_DEBUG 1

  // ��������� ��� �������� ������ � ���������� � ������ ��������� (������������ �� ������ .bootloader)
  typedef struct DATA_BOOTLOADER
  {
    uint32_t magic;   // ���������� ����� ��� �������� ����������
    char version[32]; // ������ ����������
    uint32_t version_board; // ������ �����
  } Bootloader_data_item;
  extern Bootloader_data_item bootloader_data;

  typedef struct Prgramm_version
  {
    char VERSION_PCB[11];
    char password[10];
  } Prgramm_version_item;

  typedef struct Main_data_settings
  {
    Prgramm_version_item version; // ������� ������ ����������
    double k_koeff[3];            // ����������� ������� �������� �������������. (�� 2 ������, 20�� � 4��)
    double b_koeff[3];            // ����������� ��������. (�� 2 ������, 20�� � 4��)
    double real_current[2][3];    // �������� �������� ���� 4 � 20�� 1 - 4�� ��� 20��. 2 - ����� 1, 2 ��� 3
    double Colibrate_koeff;       // ������������� ����������� ���
    uint8_t AES_KEY[16];          // ���� ���������� AES-128
    uint8_t Rezerv[16];           // ��������������� ��� ������� ����������
    uint32_t crc32;               // CRC32 ��� �������� �����������
  } Main_data_settings_item;
  extern Main_data_settings_item Main_data;

  typedef struct Factory_data
  {
    uint32_t magic;  // ���������� ����� ��� �������� ������� ������������� ������
    uint8_t version; // ������ ������������� ������
    uint8_t _rsv1[3];
    Main_data_settings_item data[3]; // ������� ������ ����������
    uint32_t crc32;                  // CRC32 ��� �������� �����������
    uint32_t _rsv2;                  // ������������ �� 8 ����
  } Factory_data_item;
  extern Factory_data_item Factory_data;

  // ��������� ����������� � EEPROM
  typedef struct EEPROM_Settings
  {
    uint8_t DEBUG_CATEG; // ��������� ��� �������
    uint8_t DEBUG_LEVL;  // �������� ������� �������
    uint8_t DEBUG_Mode;  // ����� ������� - ������ � GSM ��� �������������
    /*-----------------*/
    // �������� ������ //
    /*-----------------*/
    uint16_t time_sleep_h; // ����� ��� ���������� ����
    uint16_t time_sleep_m; // ����� ��� ���������� ������

    char Phone[20]; // ����� �������� ��� �������� ���

    double Correct[3]; // ��������� ������� ����� (�������� +- �� �������� ��������) ���
    double MAX_LVL[3];     // ������������ ������� (�������� 15 ������) ���
    double ZERO_LVL[3];    // ������� ��������     (�������� 0 ������)  ���
    // ��������� ����������� (��������) //
    uint32_t time_stablized; // ����� ������������ �������� ������� (���)
    /*-----------------*/
    // select_bar      //
    /*-----------------*/
    uint8_t Mode;              // ������� ����� ������
    uint8_t Communication;     // ������� GSM ��� ���
    uint8_t Communication_http_mqtt;     // ������� GSM ��� ���
    uint8_t RS485_prot;        // �������� RS-485
    uint8_t units_mes[3];         // �� ��������� �����, ������� ��������� - ��� ������� ������ ��������
    uint8_t screen_sever_mode; // �������� ��� ��� �������� ��� ���������
    uint8_t USB_mode;          // ����� ������ USB
    uint8_t Save_in;           // ���� ���������
    uint8_t len;               // ���� ����
    uint8_t mode_ADC[3];       // ����� ������ ���, 0 - 4-20��, 1 - 0-20��, 2 - ����
    uint8_t mode_name_ADC;     // ������� ������ ��� (����������, ������� � �.�. ��� 3� �������)
    uint8_t block;             // ���������� ����������
  } EEPROM_Settings_item;
  extern EEPROM_Settings_item EEPROM;
  ////////////////////////////////////////////////////////////////////////////////
  //               �������� ��������� ERRCODE
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ERRCODE_
  {
    uint64_t STATUS;           // ���������� ������ (������ ��� - ��������� �����)
    char STATUSCHAR[20];       // ���������� ������ � ���� ������
    char STATE_CAHAR[20];      // ���������� ������ � ���� ������
    char Diagnostics_char[10]; // ���������� ������ � ���� ������
  } ERRCODE_item;

  ////////////////////////////////////////////////////////////////////////////////
  //               �������� ��������� ADC_MS5193T
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ADC_MS5193T
  {
    char ADC_status_char[10]; // ������ ���
    uint8_t Status;           // ������ ������ ��� - 0 - ERR,  1 - WAR, 2 - OK, 3 - ����
    // ����� 1
    double ADC_Volts[3];            // ���������� �� ������� �����
    double ADC_Current[3];          // ��� �� ������� �����
    double ADC_SI_value[3];         // �������� �������� ��� ��������� �� ������
    double ADC_SI_value_correct[3]; // �������� �������� � �������������� �� ������

    double ADC_ION;
    float ADC_RESISTOR[3];
    int32_t PPM[3];

    // ����� �������� (�����������)
    int32_t MAX_LVL_char[2];     // ��������� ������������ ������ (1 - �� ������� | 2 - ����� �������)
    int32_t ZERO_LVL_char[2];    // ��������� ������������ ������ (1 - 0��/4��  | 2 - �� ������� | 3 - ����� �������)
    int32_t GVL_correct_char[2]; // ��������� ������� ����� (�������� +- �� �������� ��������) ������� �����

    

    char ADC_value_char[3][30];            // �������� ��� �� ������ 3 � ���� ������
    char ADC_Volts_char[3][30];            // ���������� �� ������� ����� � ���� ������
    char ADC_Current_char[3][30];          // ��� �� ������� ����� � ���� ������
    char ADC_SI_value_char[3][30];         // �������� �������� ��� ��������� � ���� ������
    char ADC_SI_value_correct_char[3][30]; // �������� �������� � ���������� � ���� ������

    uint32_t (*update_value)(void); // ������ �� ������� ���������� (������ ������ � ���)
  } ADC_MS5193T_item;

  typedef struct Internal_ADC
  {
    float ADC_AKB_volts;        // ���������� �� ���
    uint8_t ADC_AKB_Proc;       // ������� ������ �� ���
    double *Colibrate_koeff;    // ������������� �����������
    float MK_VBAT;              // ���������� �� CR2032
    float MK_temp;              // �����������, ���������� ��
    char MK_temp_char[6];       // ����������� �� � ���� ������
    char MK_vbat_char[6];       // ����������� �� � ���� ������
    char ADC_AKB_volts_char[6]; // ���������� �� ��� ������
    char ADC_AKB_Proc_char[6];  // ������� ������ �� ��� ������
  } Internal_ADC_item;
  extern Internal_ADC_item IntADC;
  ////////////////////////////////////////////////////////////////////////////////
  //     �������� ��������� GSM_STATUS
  ////////////////////////////////////////////////////////////////////////////////
#define GSM_RDY (1UL << 0)                      // ������ ���������
#define SIM_PRESENT (1UL << 1)                  // SIM-����� �����������
#define NETWORK_REGISTERED (1UL << 2)           // ���������� ���������������� � ����
#define SIGNAL_PRESENT (1UL << 3)               // ������ ���� ������������
#define OPERATOR_IDENTIFIED (1UL << 4)          // �������� ���� �������
#define SMS_SENT_SUCCESS (1UL << 5)             // �������� SMS �������
#define DATA_REQUEST_SUCCESS (1UL << 6)         // ������ ������ �������� �������
#define OPERATION_SEND_COMPLETED (1UL << 7)     // �������� ������� ����������
#define RESPONSE_RECEIVED (1UL << 8)            // ������� ����� �� �������
#define GPRS_CONNECTED (1UL << 9)               // GPRS ���������� �����������
#define GPRS_DISCONNECTED (1UL << 10)           // GPRS ���������� ���������
#define HTTP_SEND (1UL << 11)                   // ��������� http ������
#define MQTT_SEND (1UL << 12)                   // ��������� http ������
#define HTTP_READ (1UL << 13)                   // ��������� http ������ (������ ������)
#define SMS_SEND (1UL << 14)                    // ��������� SMS
#define HTTP_SEND_Successfully (1UL << 15)      //  http ������ �������
#define MQTT_SEND_Successfully (1UL << 16)      //  mqtt ������ �� �������� �������
#define HTTP_READ_Successfully (1UL << 17)      // http ������ (������ ������) �������
#define MQTT_READ_Successfully (1UL << 18)      // mqtt ������ (������ ������) �������
#define SMS_SEND_Successfully (1UL << 19)       //  SMS �������� ����������
#define NETWORK_REGISTERED_SET_HTTP (1UL << 20) // ���������� ���������������� � ���� ��� ����������� HTTP ��� ���������
#define DATA_READ (1UL << 21)                   // ����� ��������� ���������
#define STATUS_MQTT_CONN_ERROR        (1u<<22)
#define STATUS_MQTT_PUB_ERROR         (1u<<23)
#define STATUS_MQTT_SUB_ERROR         (1u<<24)
#define STATUS_MQTT_SERVER_COMM_ERROR (1u<<25)
#define STATUS_MQTT_AUTH_ERROR        (1u<<26)
#define STATUS_MQTT_CONN              (1u<<27)
#define STATUS_MQTT_SUB_SETTINGS      (1u<<28)

  typedef struct GSM_STATUS_item
  {
    uint32_t Status;           // ������ ������ � ���� ������� �����
    int8_t GSM_Signal_Level;   // ������� ������� GSM ��� -1, ���� ��� �����������
    int8_t GSM_Signal_Level_3; // ������� ������� GSM ��� ��� ����������� � ���� �������
    int8_t GSM_Signal_Errors;  // ������ ������� GSM ��� -1, ���� ��� �����������
    uint32_t Operator_code;    // ������ ������� GSM ��� -1, ���� ��� �����������

    char *GSM_status_char; // "OK", "ERR", "WAR", "ND" � ��������� ������� �����
    char *Modem_mode;
    char *GSM_SIMCARD_char;      // ��������, SIM_STATUS[0] ��� PRESENT ��� SIM_STATUS[1] ��� ABSENT
    char *GSM_status_ready_char; // ��������, GSM_READY_STATUS[0] ��� RDY
    char *GSM_status_reg_char;   // ��������, GSM_REG_STATUS[0] ��� REG
    char *GSM_region_char;       // ������, ������������ ������ (��������, ISO ��� ������)
    char *GSM_operator_char;     // �������� ��������� (����� �������� �� ����� ������)
    char GSM_signal_lvl_char[3]; // ��������� ������������� ������ �������
    char GSM_err_lvl_char[3];    // ��������� ������������� ������ �������
    char *GSM_gprs_on_char;      // ��������, GPRS_STATUS[0] ��� GPRS_STATUS[1]
    char GSM_sms_status[10];     // ������ �������� ���
    char GSM_site_status[10];    // ������ ������� �� ����
    char GSM_site_read_status[10];

    uint32_t GSM_LastResponseTime; // ����� ���������� ������ (�������)
    void (*update_value)(void);    // ������� ���������� ��������
  } GSM_STATUS_item;

  typedef struct GSM_Operator_item
  {
    const uint16_t code; // ��� ���������, ���������� �� ������
    const char name[10]; // �������� ��������� (��������, "MTS", "Beeline")
  } GSM_Operator_item;

  typedef struct Country_operator_item
  {
    const uint16_t mcc; // ��������� ��� ������ (MCC)
    const char iso[10]; // ISO 3166-1 alpha-3 ��� ������
  } Country_operator_item;

  ////////////////////////////////////////////////////////////////////////////////
  //  ��������� � �������
  ////////////////////////////////////////////////////////////////////////////////
  extern ADC_MS5193T_item ADC_data;
  extern GSM_STATUS_item GSM_data;
  extern ERRCODE_item ERRCODE;

  
  ////////////////////////////////////////////////////////////////////////////////
  // ���������� ���������� ������
  ////////////////////////////////////////////////////////////////////////////////
  extern uint8_t Mode;
  extern uint8_t Communication;
  extern uint8_t RS485_prot;
  extern uint8_t units_mes;
  extern uint8_t screen_sever_mode;
  extern uint8_t USB_mode;
  extern uint8_t USB_TERMINAL_STATUS;

  extern uint32_t time_counter;
#define TERMINAL_DISABLE 0
#define TERMINAL_CONNECTED 1

  extern char password[MAX_PASSWPRD_LEN];
  ////////////////////////////////////////////////////////////////////////////////
  // ���������� ���������� ������������
  ////////////////////////////////////////////////////////////////////////////////
  extern uint16_t time_update_display; // ����� ���������� ������ (��� ���������� ������� � �������)

  ////////////////////////////////////////////////////////////////////////////////
  // ���������� ���������� ��� ����������������
  ////////////////////////////////////////////////////////////////////////////////
  extern char Keyboard_press_code; // ��� ������� ������� �� ����������
  extern char error_code[4];       // ��� ���������� ������

  extern char EEPROM_status_char[10]; // ������ ����������� EEPROM
  extern char FLASH_status_char[10];  // ������ ����������� FLASH

  extern const char VERSION_PROGRAMM[20];
  extern char VERSION_BOOTLOADER[32];

  extern RTC_TimeTypeDef Time;       // ����
  extern RTC_DateTypeDef Date;       // �����
  extern RTC_TimeTypeDef Time_start; // ���� ������ ��
  extern RTC_DateTypeDef Date_start; // ����� ������ ��
  ////////////////////////////////////////////////////////////////////////////////
  // ���������� ���������
  ////////////////////////////////////////////////////////////////////////////////
  extern const char STATUS_CHAR[4][5]; // ��������� ������� �����
  extern const uint16_t Timer_key_one_press;
  extern const uint16_t Timer_key_press;
  extern const uint16_t Timer_key_press_fast;
  extern const char MODEM_STATUS[3][6];     // ��������� ������� ����� "GSM", "NBIOT", "ND"
  extern const char STATUS_CHAR[4][5];      // ��������� ������� ����� "OK", "ERR", "WAR", "ND"
  extern const char SIM_STATUS[3][8];       // ������ SIM-�����: PRESENT � �����������, ABSENT � �� �����������, UNKNOWN � ����������
  extern const char GSM_READY_STATUS[2][5]; // ������ ���������� GSM: RDY � �����, NRDY � �� �����
  extern const char GSM_REG_STATUS[3][8];   // ������ ����������� � ����: REG � ���������������, NREG � �� ���������������, UNKNOWN � ����������
  extern const char GPRS_STATUS[2][5];      // ������ GPRS ����������: CONNECTED � �����������, DISCONNECTED � ���������
  extern const int GSM_OperatorsCount;
  extern const int CountriesCount;
  extern const Country_operator_item Countries[];
  extern const GSM_Operator_item GSM_Operators[];

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
