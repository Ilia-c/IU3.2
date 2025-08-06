// Flash_colibrate_data.h
#ifndef FLASH_COLIBRATE_DATA_H
#define FLASH_COLIBRATE_DATA_H

#include <stdbool.h>
#include <stdint.h>
#include "Settings.h"  // содержит определение Prgramm_version_item
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"

// Адреса для работы с FLASH
#define FLASH_BASE_ADDR 0x08000000U
#define CALIB_DATA_ADDR 0x0807F800U
#define FLASH_PAGE_SIZE_CALIBRATE_FLASH 0x800U /* 2 КБ на страницу */
#define CALIB_FLASH_PAGE ((CALIB_DATA_ADDR - FLASH_BASE_ADDR) / FLASH_PAGE_SIZE_CALIBRATE_FLASH)

/**
 * Структура калибровочных данных, сохраняемых во флэше
 */

// Проверить, пуста ли область (все байты == 0xFF)
HAL_StatusTypeDef    Flash_IsCalibEmpty(void);
// Снять защиту страницы калибровки (WRP disable)
HAL_StatusTypeDef     Flash_UnprotectCalibPage(void);
// Включить защиту страницы калибровки (WRP enable)
HAL_StatusTypeDef     Flash_ProtectCalibPage(void);
// Записать структуру во флэш (снятие/включение WRP внутри)
HAL_StatusTypeDef     Flash_WriteCalib(const Main_data_settings_item *pData);
// Прочитать + проверить CRC (0 — OK, <0 — ошибка)
HAL_StatusTypeDef     Flash_ReadCalib(Main_data_settings_item *pData);

#endif // FLASH_COLIBRATE_DATA_H