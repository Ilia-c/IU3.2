#include "Diagnostics.h"


void Diagnostics()
{
    // EEPROM
    if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_I2C_ADDRESS, 2, 100) == HAL_OK)
    {
        if ((EEPROM_IsDataExists()))
        {
            // Сброс ошибок EEPROM (кроме записи)
            ERRCODE.STATUS &= ~STATUS_EEPROM_INIT_ERROR;
            ERRCODE.STATUS &= ~STATUS_EEPROM_READ_ERROR;
            ERRCODE.STATUS &= ~STATUS_EEPROM_CRC_ERROR;
        }
        else
        {
            ERRCODE.STATUS |= STATUS_EEPROM_CRC_ERROR;
        }
    }
    else
        ERRCODE.STATUS |= STATUS_EEPROM_INIT_ERROR;
    osDelay(300);
    // FLASH
    unsigned int id = W25_Read_ID();
    if (id != 0xef4018)
    {
        // Сброс ошибоки Инициализации (потом проверить чтение)
        ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR;
    }
    else
    {
        ERRCODE.STATUS &= ~STATUS_EEPROM_INIT_ERROR;
    }
    // SD
    //SD_check();

    // ADC
    osDelay(300);
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // Ожидается 0x0B
    if ((deviceID & 0x0F) != 0x0B)
    {
        // Ошибка инициализации
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
    else
        ERRCODE.STATUS &= ~STATUS_ADC_EXTERNAL_INIT_ERROR;

    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));
    strcpy(ADC_data.ADC_status_char, (ERRCODE.STATUS & STATUS_ADC_ERRORS) ? "ERROR" : "OK");
    strcpy(EEPROM_status_char,   (ERRCODE.STATUS & STATUS_EEPROM_ERRORS) ? "ERROR" : "OK");
    strcpy(FLASH_status_char,    (ERRCODE.STATUS & STATUS_SD_ERRORS) ? "ERROR" : "OK");
    strcpy(SD_status_char,       (ERRCODE.STATUS & STATUS_FLASH_ERRORS) ? "ERROR" : "OK");
}