// Flash_colibrate_data.h
#ifndef FLASH_COLIBRATE_DATA_H
#define FLASH_COLIBRATE_DATA_H

#include <stdbool.h>
#include <stdint.h>
#include "Settings.h"  // содержит определение Prgramm_version_item
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include "stm32l4xx.h" 
#include "stm32l4xx_hal.h"

// Адреса для работы с FLASH
#define FLASH_BASE_ADDR 0x08000000U
#define CALIB_DATA_ADDR 0x0807F800U
#define FLASH_PAGE_SIZE_CALIBRATE_FLASH 0x800U /* 2 КБ на страницу */
#define CALIB_FLASH_PAGE ((CALIB_DATA_ADDR - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE_CALIBRATE_FLASH)
#define CALIB_BANK_PAGE 127

/**
 * Структура калибровочных данных, сохраняемых во флэше
 */

// Проверить, пуста ли область (все байты == 0xFF)
static uint32_t CalcCRC_Main(const Main_data_settings_item *p);
static uint32_t CalcCRC_Factory(const Factory_data_item *pf);
uint8_t Flash_IsCalibEmpty(void);
bool Flash_IsCalibProtected(void);
HAL_StatusTypeDef Flash_ProtectCalibPage(void);
HAL_StatusTypeDef Flash_UnprotectCalibPage(void);
HAL_StatusTypeDef Flash_WriteCalib(const Main_data_settings_item *pData);
HAL_StatusTypeDef Flash_ReadCalib(Main_data_settings_item *out);




#endif // FLASH_COLIBRATE_DATA_H