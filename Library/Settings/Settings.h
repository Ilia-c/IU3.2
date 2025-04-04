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



// define ��� ������ ������
#define REDACT_MODE 0          // ����� ��������������
#define CYCLE_MODE 1           // ����� �����
#define FIRMWARE_UPDATE_MODE 2 // ����� ���������� ��������

#define MY_USB_RX_BUFFER_SIZE 512 // ��������, ��� ����� ������� � USB (����)

#define BKP_REG_INDEX_RESET_PROG  RTC_BKP_DR1 // ������� ��� ��������, ��� ����� ��� ����� � ����� ����
#define DATA_RESET_PROG  0xDEADBEEF


#define RAM2_SECTION __attribute__((section(".ram2")))
  ////////////////////////////////////////////////////////////////////////////////
  //               �������� �������� EEPROM_Settings � Prgramm_version
  ////////////////////////////////////////////////////////////////////////////////
  extern char save_data[CMD_BUFFER_SIZE];

#define Mode_set 0
#define Mode_cycle 0
#define Mode_demo 0



  #define BKP_REG_INDEX_ERROR_CODE_1  RTC_BKP_DR2
  #define BKP_REG_INDEX_ERROR_CODE_2  RTC_BKP_DR3
  // ���� ��� ������ Fault'��:
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

  // ��������� ����������� � EEPROM
  typedef struct EEPROM_Settings
  {
    Prgramm_version_item version; // ������� ������ ����������
    char last_error_code[4];      // ��������� ��� ������
    uint32_t time_work_h;         // ����� ������ ���������� ����
    uint32_t time_work_m;         // ����� ������ ���������� ������

    /*-----------------*/
    // �������� ������ //
    /*-----------------*/
    uint16_t time_sleep_h; // ����� ��� ���������� ����
    uint16_t time_sleep_m; // ����� ��� ���������� ������

    char Phone[20];      // ��������� ��� ������
    //  ���  //
    double ADC_ION;     // ���������� ��� ���
    float ADC_RESISTOR; // ������������� ���������
    double GVL_correct; // ��������� ������� ����� (�������� +- �� �������� ��������) ���
    double GVL_correct_4m;    // �������� 4��
    double GVL_correct_20m;   // �������� 20��
    double k_koeff;     // ����������� ������� �������� �������������. (�� 2 ������, 20�� � 4��)
    double MAX_LVL;     // ������������ ������� (�������� 15 ������) ���
    double ZERO_LVL;    // ������� ��������     (�������� 0 ������)  ���
    // ��������� ����������� (��������) //
    double Crorrect_TEMP_A; // �������� ������� ����������� �����������
    double Colibrate_koeff;

    /*-----------------*/
    // select_bar      //
    /*-----------------*/
    uint8_t Mode;              // ������� ����� ������
    uint8_t Communication;     // ������� GSM ��� ���
    uint8_t RS485_prot;        // �������� RS-485
    uint8_t units_mes;         // �� ��������� �����, ������� ���������
    uint8_t screen_sever_mode; // �������� ��� ��� �������� ��� ���������
    uint8_t USB_mode;          // ����� ������ USB
    uint8_t Save_in;          // ����� ������ USB
    uint8_t len;               // ���� ����
    uint8_t mode_ADC;          // ����� ������ ���, 0 - 4-20��, 1 - 0-20��, 2 - ����
    uint8_t block;            // ���������� ����������
  } EEPROM_Settings_item;


  ////////////////////////////////////////////////////////////////////////////////
  //               �������� ��������� ERRCODE
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ERRCODE_
  {
    uint64_t STATUS;     // ���������� ������ (������ ��� - ��������� �����)
    char STATUSCHAR[20]; // ���������� ������ � ���� ������
  } ERRCODE_item;

  ////////////////////////////////////////////////////////////////////////////////
  //               �������� ��������� ADC_MS5193T
  ////////////////////////////////////////////////////////////////////////////////
  typedef struct ADC_MS5193T
  {
    char ADC_status_char[10];       // ������ GSM
    int32_t ADC_value;           // �������� ���
    double ADC_Volts;            // ���������� �� ������� �����
    double ADC_Current;          // ��� �� ������� �����
    double ADC_SI_value;         // �������� �������� ��� ��������� �� ������
    double ADC_SI_value_correct; // �������� �������� � �������������� �� ������
    uint8_t Status;              // ������ ������ ��� - 0 - ERR,  1 - WAR, 2 - OK, 3 - ����

    double *Temp_correct_A; // ��������� ����������� (��������)
    double *ADC_ION;
    float *ADC_RESISTOR;
    int32_t PPM;
    uint8_t *mode; // ����� ������ ���, 0 - 4-20��, 1 - 0-20��, 2 - ����
    // ������������� ���
    double *GVL_correct; // ��������� ������� ����� (�������� +- �� �������� ��������)
    double *k_koeff;     // ����������� ������� �������� �������������. (�� 2 ������, 20�� � 4��)
    // ����������
    double *MAX_LVL;  // ������������ ������� (�������� 15 ������) ���
    double *ZERO_LVL; // ������� ��������     (�������� 0 ������)  ���

    // ����� �������� (�����������)
    int32_t MAX_LVL_char[2];            // ��������� ������������ ������ (1 - �� ������� | 2 - ����� �������)
    int32_t ZERO_LVL_char[2];           // ��������� ������������ ������ (1 - 0��/4��  | 2 - �� ������� | 3 - ����� �������)
    int32_t GVL_correct_char[2];        // ��������� ������� ����� (�������� +- �� �������� ��������) ������� �����
    int32_t Temp_correct[2];        // ��������� ������� ����� ���������� (�������� +- �� �������� ��������) ������� �����

    char ADC_value_char[15];            // �������� ��� � ���� ������
    char ADC_Volts_char[15];            // ���������� �� ������� ����� � ���� ������
    char ADC_Current_char[15];          // ��� �� ������� ����� � ���� ������
    char ADC_SI_value_char[15];         // �������� �������� ��� ��������� � ���� ������
    char ADC_SI_value_correct_char[15]; // �������� �������� � ���������� � ���� ������

    double ADC_MS5193T_temp;        // ����������� �� ���������� �������
    char ADC_MS5193T_temp_char[15]; // ����������� �� ���������� ������� � ���� ������

    void (*update_value)(void); // ������ �� ������� ���������� (������ ������ � ���)
  } ADC_MS5193T_item;

  typedef struct Internal_ADC
  {
    float ADC_AKB_volts; // ���������� �� ���
    uint8_t ADC_AKB_Proc;    // ������� ������ �� ���
    double *Colibrate_koeff; // ������������� �����������
    uint16_t R1;
    uint16_t R2;
    float VBAT;          // ���������� �� CR2032
    char ADC_AKB_volts_char[6]; // ���������� �� ��� ������
    char ADC_AKB_Proc_char[6];  // ������� ������ �� ��� ������
  } Internal_ADC_item;
  extern Internal_ADC_item IntADC;
  ////////////////////////////////////////////////////////////////////////////////
  //     �������� ��������� GSM_STATUS
  ////////////////////////////////////////////////////////////////////////////////
#define GSM_RDY (1UL << 0)              // ������ ���������
#define SIM_PRESENT (1UL << 1)          // SIM-����� �����������
#define NETWORK_REGISTERED (1UL << 2)   // ���������� ���������������� � ����
#define SIGNAL_PRESENT (1UL << 3)       // ������ ���� ������������
#define OPERATOR_IDENTIFIED (1UL << 4)  // �������� ���� �������
#define SMS_SENT_SUCCESS (1UL << 5)     // �������� SMS �������
#define DATA_REQUEST_SUCCESS (1UL << 6) // ������ ������ �������� �������
#define OPERATION_SEND_COMPLETED (1UL << 7)  // �������� ������� ���������� !!!
#define RESPONSE_RECEIVED (1UL << 8)    // ������� ����� �� �������
#define GPRS_CONNECTED (1UL << 9)       // GPRS ���������� �����������
#define GPRS_DISCONNECTED (1UL << 10)    // GPRS ���������� ���������
#define HTTP_SEND (1UL << 11)    // ��������� http ������
#define HTTP_READ (1UL << 12)    // ��������� http ������ (������ ������)
#define SMS_SEND (1UL << 13)    // ��������� SMS
#define HTTP_SEND_Successfully  (1UL << 14)    //  http ������ �������
#define HTTP_READ_Successfully (1UL << 15)    // http ������ (������ ������) �������
#define SMS_SEND_Successfully (1UL << 16)    //  SMS �������� ����������
#define NETWORK_REGISTERED_SET_HTTP (1UL << 17)   // ���������� ���������������� � ���� ��� ����������� HTTP ��� ���������
#define DATA_READ (1UL << 18) // ����� ��������� ���������

  typedef struct GSM_STATUS_item
  {
    uint32_t Status;                // ������ ������ � ���� ������� �����
    int8_t GSM_Signal_Level;        // ������� ������� GSM ��� -1, ���� ��� �����������
    int8_t GSM_Signal_Level_3;      // ������� ������� GSM ��� ��� ����������� � ���� �������
    int8_t GSM_Signal_Errors;       // ������ ������� GSM ��� -1, ���� ��� �����������
    uint32_t Operator_code;         // ������ ������� GSM ��� -1, ���� ��� �����������

    char *GSM_status_char;          // "OK", "ERR", "WAR", "ND" � ��������� ������� �����
    char *Modem_mode;
    char *GSM_SIMCARD_char;         // ��������, SIM_STATUS[0] ��� PRESENT ��� SIM_STATUS[1] ��� ABSENT
    char *GSM_status_ready_char;    // ��������, GSM_READY_STATUS[0] ��� RDY
    char *GSM_status_reg_char;      // ��������, GSM_REG_STATUS[0] ��� REG
    char *GSM_region_char;          // ������, ������������ ������ (��������, ISO ��� ������)
    char *GSM_operator_char;        // �������� ��������� (����� �������� �� ����� ������)
    char GSM_signal_lvl_char[3];    // ��������� ������������� ������ �������
    char GSM_err_lvl_char[3];       // ��������� ������������� ������ �������
    char *GSM_gprs_on_char;         // ��������, GPRS_STATUS[0] ��� GPRS_STATUS[1]
    char GSM_sms_status[10];         // ������ �������� ���
    char GSM_site_status[10];         // ������ ������� �� ����
    char GSM_site_read_status[10];

    uint32_t GSM_LastResponseTime;  // ����� ���������� ������ (�������)
    void (*update_value)(void);     // ������� ���������� ��������
  } GSM_STATUS_item;

typedef struct GSM_Operator_item
{
  const uint16_t code;    // ��� ���������, ���������� �� ������
  const char name[10]; // �������� ��������� (��������, "MTS", "Beeline")
} GSM_Operator_item;

typedef struct Country_operator_item
{
  const uint16_t mcc;         // ��������� ��� ������ (MCC)
  const char iso[10]; // ISO 3166-1 alpha-3 ��� ������
}Country_operator_item;

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


extern char password[5];
////////////////////////////////////////////////////////////////////////////////
// ���������� ���������� ������������
////////////////////////////////////////////////////////////////////////////////
extern uint16_t time_update_display; // ����� ���������� ������ (��� ���������� ������� � �������)

////////////////////////////////////////////////////////////////////////////////
// ���������� ���������� ��� ����������������
////////////////////////////////////////////////////////////////////////////////
extern char Keyboard_press_code;   // ��� ������� ������� �� ����������
extern char error_code[4];         // ��� ���������� ������
extern double OneWire_temp;        // ����������� OneWire
extern char OneWire_temp_char[5];  // ����������� OneWire � ���� ������

extern char EEPROM_status_char[10]; // ������ ����������� EEPROM
extern char FLASH_status_char[10];  // ������ ����������� FLASH

extern RTC_TimeTypeDef Time; // ����
extern RTC_DateTypeDef Date; // �����
extern RTC_TimeTypeDef Time_start; // ���� ������ ��
extern RTC_DateTypeDef Date_start; // ����� ������ ��
////////////////////////////////////////////////////////////////////////////////
// ���������� ���������
////////////////////////////////////////////////////////////////////////////////
extern const char STATUS_CHAR[4][5]; // ��������� ������� �����
extern const uint16_t Timer_key_one_press;
extern const uint16_t Timer_key_press;
extern const uint16_t Timer_key_press_fast;
extern const char MODEM_STATUS[3][6]; // ��������� ������� ����� "GSM", "NBIOT", "ND"
extern const char STATUS_CHAR[4][5]; // ��������� ������� ����� "OK", "ERR", "WAR", "ND"
extern const char SIM_STATUS[3][8];// ������ SIM-�����: PRESENT � �����������, ABSENT � �� �����������, UNKNOWN � ���������� 
extern const char GSM_READY_STATUS[2][5];// ������ ���������� GSM: RDY � �����, NRDY � �� �����
extern const char GSM_REG_STATUS[3][8];// ������ ����������� � ����: REG � ���������������, NREG � �� ���������������, UNKNOWN � ����������
extern const char GPRS_STATUS[2][5]; // ������ GPRS ����������: CONNECTED � �����������, DISCONNECTED � ���������
extern const int GSM_OperatorsCount;
extern const int CountriesCount;
extern const Country_operator_item Countries[];
extern const GSM_Operator_item GSM_Operators[];

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H
