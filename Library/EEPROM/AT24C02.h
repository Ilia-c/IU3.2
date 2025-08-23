#ifndef __AT24C02_H
#define __AT24C02_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32l4xx_hal.h"
#include <string.h>
#include <stddef.h> 
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

// �������� �� ������ ��������� EepromRecord
#define STRUCT_SIZE             ((uint16_t)sizeof(EepromRecord))
#define BUFFER_ENTRY_SIZE     ((uint16_t)sizeof(uint32_t))
#define BUFFER_ENTRIES        (BUFFER_SIZE_BYTES / BUFFER_ENTRY_SIZE)
#define BUFFER_START_ADDR      ((uint16_t)STRUCT_SIZE)
#define BUFFER_SIZE_BYTES     ((uint16_t)(EEPROM_TOTAL_SIZE - BUFFER_START_ADDR))
typedef char array_check_eeprom_size[
    (sizeof(EepromRecord) <= 140) ? 1 : -1 
];


/**
 * @brief ��������� ��������� � EEPROM.
 * @param src ��������� �� ��������� � �����������.
 * @return true, ���� ������ ������ �������, ����� false.
 */
HAL_StatusTypeDef EEPROM_SaveSettings(EEPROM_Settings_item *src);

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
HAL_StatusTypeDef EEPROM_ReadData(uint16_t memAddr, uint8_t *pData, uint16_t size);
HAL_StatusTypeDef EEPROM_WriteData(uint16_t memAddr, const uint8_t *pData, uint16_t size);
HAL_StatusTypeDef WriteSlot(const EepromRecord *record);
HAL_StatusTypeDef ReadSlot(EepromRecord *record);

#ifdef __cplusplus
}
#endif

#endif // __AT24C02_H
