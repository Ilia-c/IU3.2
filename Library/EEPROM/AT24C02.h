#ifndef __AT24C02_H
#define __AT24C02_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Settings.h"

/* ��������� EEPROM */
#define EEPROM_PAGE_SIZE        8U
#define EEPROM_TOTAL_SIZE       256U
#define EEPROM_START_ADDR       0x0000
#define EEPROM_SIGNATURE        0xDEADBEEF
#define EEPROM_I2C_ADDRESS      (0x50 << 1)

/* ���������, ����������� � EEPROM.
 * ������������� __attribute__((packed)) ��������� �������� ������� ������������,
 * ��� �������� ��� ������� ������������ ������� ������ EEPROM.
 */
typedef struct __attribute__((packed)) {
    uint32_t signature;        // ���������� ��������� ��� �������� ������� ���������� ������
    uint32_t formatVersion;    // ������ ������� ���������� ������
    uint32_t version;          // ����� ������/������� ����������
    EEPROM_Settings_item data; // ���������������� ��������� (������������ � "Settings.h")
    uint32_t crc;              // ����������� ����� (CRC)
} EepromRecord;

/* ������� ������ � EEPROM */

/**
 * @brief ��������� ��������� � EEPROM.
 * @param src ��������� �� ��������� � �����������.
 * @return true, ���� ������ ������ �������, ����� false.
 */
HAL_StatusTypeDef EEPROM_SaveSettings(const EEPROM_Settings_item *src);

/**
 * @brief ��������� ��������� �� EEPROM.
 * @param dst ��������� �� ���������, ���� ����� ��������� ���������.
 * @return true, ���� �������� ������ �������, ����� false.
 */
HAL_StatusTypeDef EEPROM_LoadSettings(EEPROM_Settings_item *dst);

/**
 * @brief ���������, ���������� �� �������� ������ � EEPROM.
 * @return true, ���� ������ ���������� � ���������, ����� false.
 */
HAL_StatusTypeDef EEPROM_IsDataExists(void);

/**
 * @brief ��������� ������������ ���������� ������ � EEPROM.
 *
 * ������� ��������� ���������� ����� ������ � ��������� ���������, ������ �������
 * � ����������� ����� (CRC). ���� ��� �������� ��������, ������ ��������� �����������.
 *
 * @return true, ���� ������ ���������, ����� false.
 */
HAL_StatusTypeDef EEPROM_CheckDataValidity(void);

#ifdef __cplusplus
}
#endif

#endif // __AT24C02_H
