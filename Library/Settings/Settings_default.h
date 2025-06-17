#ifndef SETTINGS_DEFAULT_H
#define SETTINGS_DEFAULT_H

// ����������
#define Debug_mode 1                                // ������� ��� �������� ����� �������

#define Version3_75 1                            // ������ 3.75
#define Version3_79 2                            // ������ 3.79
#define BOARD_VERSION Version3_75                // ��� ����� ������ �������� ������

// �������� �� ��������� ��� EEPROM
#define DEFAULT_VERSION_PROGRAMM       "Ver0.78"    // ������ ���������
#define EEPROM_FORMAT_VERSION           0x00000010U //? ������ ������� ������ EEPROM

#define DEFAULT_VERSION_PCB            "3.75-A000{" // ������ �������� �����
#define DEFAULT_PASSWORD               "Defau"
#define DEFAULT_TIME_WORK_CHAR         "0"          // ����� ������ � ���� ������

#define DEFAULT_LAST_ERROR_CODE        {0x00, 0x00, 0x00, 0x00}
#define DEFAULT_TIME_WORK_H            0

#define DEFAULT_TIME_SLEEP_H           1  // ����� ��� (����)
#define DEFAULT_TIME_SLEEP_M           0  // ����� ��� (������)

#define DEFAULT_PHONE                  "+79653003557"

// ��������� ���:
#define DEFAULT_ADC_ION                1.17
#define DEFAULT_ADC_RESISTOR           49.99
#define DEFAULT_GVL_CORRECT            0
#define DEFAULT_K_KOEFF                0
#define DEFAULT_MAX_LVL                15
#define DEFAULT_ZERO_LVL               0
#define DEFAULT_GVL_CORRECT_4M         0.004
#define DEFAULT_GVL_CORRECT_20M        0.02

// ��������� �����������:
#define DEFAULT_CRORRECT_TEMP_A        0
#define DEFAULT_COLIBRATE_KOEFF        10.81

// ��������� select_bar:
#define DEFAULT_MODE                   0  // ����� ������: 0 - ������� ���������, 1 - �����������, 2 - ��������
#define DEFAULT_COMMUNICATION          1  // ������� GSM ��� ���
#define DEFAULT_RS485_PROT             0  // �������� RS-485
#define DEFAULT_UNITS_MES              1  // ������� ��������� (�����)
#define DEFAULT_SCREEN_SEVER_MODE      1  // �������� �������� ��� ���������
#define DEFAULT_USB_MODE               0  // ����� ������ USB
#define DEFAULT_SAVE_IN                0  // ���� ���������: 0 - FLASH, 1 - SD, 2 - USB, 3 - ����
#define DEFAULT_LEN                    0  // ���� ����
#define DEFAULT_MODE_ADC               0  // ����� ������ ���: 0 - 4-20��, 1 - 0-20��, 2 - ����
#define DEFAULT_BLOCK                  0  // ����������: 0 - ��������������, 1 - �������������




#endif // SETTINGS_H