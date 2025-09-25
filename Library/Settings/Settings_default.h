#ifndef SETTINGS_DEFAULT_H
#define SETTINGS_DEFAULT_H

// ����������
#define Debug_mode 1                                // ������� ��� �������� ����� �������

#define Version3_75 0xFAEE                       // ������ 3.75
#define Version3_80 0x0EA3                       // ������ 3.80
#define BOARD_VERSION Version3_80                // ��� ����� ������ �������� ������

#if BOARD_VERSION == Version3_80 
    #define i2cDisplay hi2c3
#elif BOARD_VERSION == Version3_75
    #define i2cDisplay hi2c2
#endif

// �������� �� ��������� ��� EEPROM
#define DEFAULT_VERSION_PROGRAMM       "Ver0.86"    // ������ ���������
#define DEFAULT_VERSION_PROGRAMM_UINT16_t  86       // ������ ���������
#define DEFAULT_VERSION_BOOTLOADER     "Uncnown"    // ������ ���������
#define BOOTLOADER_MAGIC                0xB00B1E00
#define EEPROM_FORMAT_VERSION           0x00000010U //? ������ ������� ������ EEPROM

#if BOARD_VERSION == Version3_80 
    #define DEFAULT_VERSION_PCB            "3.80-A000{" // ������ �������� �����
#elif BOARD_VERSION == Version3_75
    #define DEFAULT_VERSION_PCB            "3.75-A000{" // ������ �������� �����
#endif

#define DEFAULT_PASSWORD               "Defau"
#define DEFAULT_TIME_WORK_CHAR         "0"          // ����� ������ � ���� ������
#define DEFAULT_TIME_WORK_H            0

#define DEFAULT_TIME_SLEEP_H           1  // ����� ��� (����)
#define DEFAULT_TIME_SLEEP_M           0  // ����� ��� (������)

#define DEFAULT_PHONE                  "+79653003557"

// ��������� ���:
#define DEFAULT_ADC_ION                1.17f
#define DEFAULT_ADC_RESISTOR           49.99
#define DEFAULT_GVL_CORRECT_4M         0.004
#define DEFAULT_GVL_CORRECT_20M        0.02

// ����� 1
#define DEFAULT_GVL_CORRECT_1            0
#define DEFAULT_REAL_CURRENT_4MA_1      0.004
#define DEFAULT_REAL_CURRENT_20MA_1     0.02
#define DEFAULT_K_KOEFF_1                1
#define DEFAULT_B_KOEFF_1                0.0f
#define DEFAULT_MAX_LVL_1                15
#define DEFAULT_ZERO_LVL_1               0
// ����� 2
#define DEFAULT_GVL_CORRECT_2            0
#define DEFAULT_REAL_CURRENT_4MA_2      0.004
#define DEFAULT_REAL_CURRENT_20MA_2     0.02
#define DEFAULT_K_KOEFF_2                1
#define DEFAULT_B_KOEFF_2                0.0f
#define DEFAULT_MAX_LVL_2                15
#define DEFAULT_ZERO_LVL_2               0
// ����� 3
#define DEFAULT_GVL_CORRECT_3            0
#define DEFAULT_REAL_CURRENT_4MA_3      0.004
#define DEFAULT_REAL_CURRENT_20MA_3     0.02
#define DEFAULT_K_KOEFF_3                1
#define DEFAULT_B_KOEFF_3                0.0f
#define DEFAULT_MAX_LVL_3                15
#define DEFAULT_ZERO_LVL_3               0

#define DEFAULT_TIME_STABLIZED         10  // ����� ������������ ������� (���)


// ��������� �����������:
#define DEFAULT_CRORRECT_TEMP_A        0
#define DEFAULT_COLIBRATE_KOEFF        10.81

#define MQTT 1  // MQTT ��� HTTP 1- MQTT, 0 - HTTP
#define HTTP 0  // MQTT ��� HTTP 1- MQTT, 0 - HTTP

// �������� rs-485
#define RS485_OFF 0
#define RS485_ONLY 1

// ��������� select_bar:
#define DEFAULT_MODE                   0  // ����� ������: 0 - ������� ���������, 1 - �����������, 2 - ��������
#define DEFAULT_COMMUNICATION          1  // ������� GSM ��� ���
#define DEFAULT_COMMUNICATION_HTTP_MQTT 1  // MQTT ��� HTTP 1- MQTT, 0 - HTTP
#define DEFAULT_RS485_PROT             0  // �������� RS-485
#define DEFAULT_UNITS_MES              1  // ������� ��������� (�����)
#define DEFAULT_SCREEN_SEVER_MODE      1  // �������� �������� ��� ���������
#define DEFAULT_USB_MODE               0  // ����� ������ USB
#define DEFAULT_SAVE_IN                0  // ���� ���������: 0 - FLASH, 1 - SD, 2 - USB, 3 - ����
#define DEFAULT_LEN                    0  // ���� ����
#define DEFAULT_MODE_ADC               0  // ����� ������ ���: 0 - 4-20��, 1 - 0-20��, 2 - ����
#define DEFAULT_BLOCK                  0  // ����������: 0 - ��������������, 1 - �������������




#endif // SETTINGS_H