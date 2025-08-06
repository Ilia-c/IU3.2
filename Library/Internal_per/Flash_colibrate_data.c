#include "Flash_colibrate_data.h"
#include <string.h>

extern CRC_HandleTypeDef hcrc;  // HW CRC peripheral handle

/**
 * @brief  Подсчёт CRC32 аппаратным модулем (исключая поле crc32)
 */
static uint32_t CalcCRC(const Main_data_settings_item *pData)
{
    Main_data_settings_item tmp = *pData;
    tmp.crc32 = 0;
    uint32_t *ptr   = (uint32_t *)&tmp;
    uint32_t length = sizeof(Main_data_settings_item) / sizeof(uint32_t);
    return HAL_CRC_Calculate(&hcrc, ptr, length);
}

/**
 * @brief  Проверить, пуста ли область флеша (все байты == 0xFF)
 * @retval HAL_OK если область пуста, иначе HAL_ERROR
 */
HAL_StatusTypeDef Flash_IsCalibEmpty(void)
{
    uint8_t *addr = (uint8_t *)CALIB_DATA_ADDR;
    for (uint32_t i = 0; i < sizeof(Main_data_settings_item); ++i) {
        if (addr[i] != 0xFFU) {
            return HAL_ERROR;
        }
    }
    return HAL_OK;
}

/**
 * @brief  Включить защиту WRP на странице калибровки
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_ProtectCalibPage(void)
{
    FLASH_OBProgramInitTypeDef obInit = {0};

    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    HAL_FLASHEx_OBGetConfig(&obInit);

    obInit.OptionType     = OPTIONBYTE_WRP;
    obInit.WRPArea        = OB_WRPAREA_BANK1_AREAB;
    obInit.WRPStartOffset = CALIB_FLASH_PAGE;
    obInit.WRPEndOffset   = CALIB_FLASH_PAGE;

    if (HAL_FLASHEx_OBProgram(&obInit) != HAL_OK) goto error;
    if (HAL_FLASH_OB_Launch()    != HAL_OK) goto error;

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return HAL_OK;

error:
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return HAL_ERROR;
}

/**
 * @brief  Снять защиту WRP со всех страниц Bank1
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_UnprotectCalibPage(void)
{
    FLASH_OBProgramInitTypeDef obInit = {0};

    HAL_FLASH_Unlock();
    HAL_FLASH_OB_Unlock();
    HAL_FLASHEx_OBGetConfig(&obInit);

    obInit.OptionType     = OPTIONBYTE_WRP;
    obInit.WRPArea        = OB_WRPAREA_BANK1_AREAB;
    obInit.WRPStartOffset = 0;
    obInit.WRPEndOffset   = (FLASH_BANK_SIZE / FLASH_PAGE_SIZE) - 1;

    if (HAL_FLASHEx_OBProgram(&obInit) != HAL_OK) goto error;
    if (HAL_FLASH_OB_Launch()    != HAL_OK) goto error;

    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return HAL_OK;

error:
    HAL_FLASH_OB_Lock();
    HAL_FLASH_Lock();
    return HAL_ERROR;
}

/**
 * @brief  Записать калибровочные данные во Flash
 * @retval HAL_OK при успехе, HAL_ERROR при ошибке
 */
HAL_StatusTypeDef Flash_WriteCalib(const Main_data_settings_item *pData)
{
    if (Flash_UnprotectCalibPage() != HAL_OK) {
        return HAL_ERROR;
    }

    FLASH_EraseInitTypeDef eraseInit = {0};
    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Banks     = FLASH_BANK_1;
    eraseInit.Page      = CALIB_FLASH_PAGE;
    eraseInit.NbPages   = 1;
    uint32_t pageError;

    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Erase(&eraseInit, &pageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    Main_data_settings_item data = *pData;
    data.crc32 = CalcCRC(&data);

    const uint64_t *words = (const uint64_t *)&data;
    uint32_t count = sizeof(Main_data_settings_item) / sizeof(uint64_t);
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t addr = CALIB_DATA_ADDR + i * sizeof(uint64_t);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, words[i]) != HAL_OK) {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }
    HAL_FLASH_Lock();

    return Flash_ProtectCalibPage();
}

/**
 * @brief  Прочитать калибровочные данные из Flash и проверить CRC
 * @retval HAL_OK если данные корректны, HAL_ERROR при CRC mismatch
 */
HAL_StatusTypeDef Flash_ReadCalib(Main_data_settings_item *pData)
{
    memcpy(pData, (void *)CALIB_DATA_ADDR, sizeof(Main_data_settings_item));
    uint32_t crc = CalcCRC(pData);
    return (crc == pData->crc32) ? HAL_OK : HAL_ERROR;
}