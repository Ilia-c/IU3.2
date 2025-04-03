#include "Diagnostics.h"


HAL_StatusTypeDef EEPROM_CHECK(){
    if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_I2C_ADDRESS, 2, 100) == HAL_OK)
    {
        ERRCODE.STATUS &= ~STATUS_EEPROM_INIT_ERROR;
        EEPROM_IsDataExists();
        return HAL_OK;
    }
    ERRCODE.STATUS |= STATUS_EEPROM_INIT_ERROR;
    return HAL_ERROR;
}

void Diagnostics()
{
    // OLED
    OLED_Diagnostics();

    // EEPROM
    if (EEPROM_CHECK() == HAL_OK)
    {
        EEPROM_IsDataExists();
    }

    osDelay(300);
    // FLASH
    uint32_t id = 0;
    W25_Read_ID(&id);
    if (id != 0xef4018)
    {
        // Сброс ошибоки Инициализации (потом проверить чтение)
        ERRCODE.STATUS |= STATUS_FLASH_INIT_ERROR;
    }
    else
    {
        ERRCODE.STATUS &= ~STATUS_FLASH_INIT_ERROR;
    }

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
    strcpy(FLASH_status_char,    (ERRCODE.STATUS & STATUS_FLASH_ERRORS) ? "ERROR" : "OK");
}