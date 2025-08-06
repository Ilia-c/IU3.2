// Flash_colibrate_data.h
#ifndef FLASH_COLIBRATE_DATA_H
#define FLASH_COLIBRATE_DATA_H

#include <stdbool.h>
#include <stdint.h>
#include "Settings.h"  // �������� ����������� Prgramm_version_item
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"

// ������ ��� ������ � FLASH
#define FLASH_BASE_ADDR 0x08000000U
#define CALIB_DATA_ADDR 0x0807F800U
#define FLASH_PAGE_SIZE_CALIBRATE_FLASH 0x800U /* 2 �� �� �������� */
#define CALIB_FLASH_PAGE ((CALIB_DATA_ADDR - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE_CALIBRATE_FLASH)

/**
 * ��������� ������������� ������, ����������� �� �����
 */

// ���������, ����� �� ������� (��� ����� == 0xFF)
HAL_StatusTypeDef    Flash_IsCalibEmpty(void);
// ����� ������ �������� ���������� (WRP disable)
HAL_StatusTypeDef     Flash_UnprotectCalibPage(void);
// �������� ������ �������� ���������� (WRP enable)
HAL_StatusTypeDef     Flash_ProtectCalibPage(void);
// �������� ��������� �� ���� (������/��������� WRP ������)
HAL_StatusTypeDef     Flash_WriteCalib(const Main_data_settings_item *pData);
// ��������� + ��������� CRC (0 � OK, <0 � ������)
HAL_StatusTypeDef     Flash_ReadCalib(Main_data_settings_item *pData);

#endif // FLASH_COLIBRATE_DATA_H