#include "Diagnostics.h"
extern uint32_t time_work;

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
    USB_DEBUG_MESSAGE("[INFO] Начата диагностика", DEBUG_OTHER, DEBUG_LEVL_3);
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
    char status_char[30];
    sprintf(status_char, "[DEBUG FLASH] ID W25Q: 0x%06lX", id);
    USB_DEBUG_MESSAGE(status_char, DEBUG_FLASH, DEBUG_LEVL_3);
    if (id != 0xef4018)
    {
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

    if (ERRCODE.STATUS & CRITICAL_ERRORS_MASK) strcpy(ERRCODE.Diagnostics_char, "ERROR");
    else strcpy(ERRCODE.Diagnostics_char, "OK");

    base62_encode(ERRCODE.STATUS, ERRCODE.STATUSCHAR, sizeof(ERRCODE.STATUSCHAR));
    strcpy(ADC_data.ADC_status_char, (ERRCODE.STATUS & STATUS_ADC_ERRORS) ? "ERROR" : "OK");
    strcpy(EEPROM_status_char,   (ERRCODE.STATUS & STATUS_EEPROM_ERRORS) ? "ERROR" : "OK");
    strcpy(FLASH_status_char,    (ERRCODE.STATUS & STATUS_FLASH_ERRORS) ? "ERROR" : "OK");
    USB_DEBUG_MESSAGE("[INFO] Диагностика завершена", DEBUG_OTHER, DEBUG_LEVL_3);
}       



uint32_t EEPROM_Test(void) {
    uint32_t error_count = 0;

    EepromRecord record;
    if (ReadSlot(&record) != HAL_OK) {
        USB_DEBUG_MESSAGE("[ERROR EEPROM] Ошибка чтения данных из EEPROM при тесте", DEBUG_EEPROM, DEBUG_LEVL_2);
        return HAL_ERROR;
    }
    EEPROM_LoadLastTimeWork();
    


    HAL_StatusTypeDef status;
    uint8_t buffer = 0xFF;
    for (uint32_t i = 0; i < EEPROM_TOTAL_SIZE; ++i)
    {
        buffer = 0xFF;
        status = EEPROM_WriteData(i, &buffer, 1);
        if (status != HAL_OK) {
            USB_DEBUG_MESSAGE("[ERROR EEPROM] Ошибка записи едениц в EEPROM", DEBUG_EEPROM, DEBUG_LEVL_2);
            error_count++;
            continue;
        }
        status = EEPROM_ReadData(i, &buffer, 1);
        if (status != HAL_OK) {
            USB_DEBUG_MESSAGE("[ERROR EEPROM] Ошибка чтения едениц в EEPROM", DEBUG_EEPROM, DEBUG_LEVL_2);
            error_count++;
            continue;
        }
        if (buffer != 0xFF) {
            error_count++;
        }
        buffer = 0x00;
        status = EEPROM_WriteData(i, &buffer, 1);
        if (status != HAL_OK) {
            USB_DEBUG_MESSAGE("[ERROR EEPROM] Ошибка записи едениц в EEPROM", DEBUG_EEPROM, DEBUG_LEVL_2);
            error_count++;
            continue;
        }
        status = EEPROM_ReadData(i, &buffer, 1);
        if (status != HAL_OK) {
            USB_DEBUG_MESSAGE("[ERROR EEPROM] Ошибка чтения едениц в EEPROM", DEBUG_EEPROM, DEBUG_LEVL_2);
            error_count++;
            continue;
        }
        if (buffer != 0x00) {
            error_count++;
        }

    }
    status = EEPROM_WriteData(BUFFER_START_ADDR, (uint8_t *)&time_work, (uint16_t)BUFFER_ENTRY_SIZE);
	if (status != HAL_OK)
	{
		HAL_PWR_DisableBkUpAccess();
		return HAL_ERROR;
	}
    WriteSlot(&record);
    return error_count;
}
