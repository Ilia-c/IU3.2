#include "AT24C02.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include <stddef.h> 

extern I2C_HandleTypeDef hi2c1;
_Static_assert(sizeof(EepromRecord) <= EEPROM_TOTAL_SIZE, "EepromRecord exceeds EEPROM size");

//=============================================================================
// Функция расчёта 32-битного CRC (POLY=0xEDB88320)
static uint32_t CalculateCRC(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc >>= 1;
        }
    }
    return ~crc;
}

//=============================================================================
// Функция записи данных во EEPROM с учётом постраничной записи (AT24C02)
static HAL_StatusTypeDef EEPROM_WriteData(uint16_t memAddr, const uint8_t *pData, uint16_t size) {
    uint16_t bytesWritten = 0;
    HAL_StatusTypeDef status;

    while (bytesWritten < size) {
        uint16_t currentAddr = memAddr + bytesWritten;
        uint8_t pageOffset   = currentAddr % EEPROM_PAGE_SIZE;
        uint8_t bytesToWrite = EEPROM_PAGE_SIZE - pageOffset;

        if (bytesToWrite > (size - bytesWritten)) {
            bytesToWrite = (size - bytesWritten);
        }

        if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_I2C_ADDRESS, 3, 100) != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_READY_ERROR;
            return HAL_ERROR;
        }
        ERRCODE.STATUS &= ~STATUS_EEPROM_READY_ERROR;
        status = HAL_I2C_Mem_Write(&hi2c1,
                                   EEPROM_I2C_ADDRESS,
                                   currentAddr,
                                   I2C_MEMADD_SIZE_8BIT, // Важно для AT24C02
                                   (uint8_t *)(pData + bytesWritten),
                                   bytesToWrite,
                                   1000);
        if (status == HAL_TIMEOUT)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_TIMEOUT_I2C_ERROR;
            return HAL_ERROR;
        }
        ERRCODE.STATUS &= ~STATUS_EEPROM_TIMEOUT_I2C_ERROR;

        if (status == HAL_BUSY)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_READY_ERROR;
            return HAL_ERROR;
        }
        ERRCODE.STATUS &= ~STATUS_EEPROM_READY_ERROR;
        if (status == HAL_ERROR)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
            return HAL_ERROR;
        }
        ERRCODE.STATUS &= ~STATUS_EEPROM_WRITE_ERROR;

        // Задержка для завершения внутренней записи EEPROM (обычно 5 мс)
        HAL_Delay(5);

        bytesWritten += bytesToWrite;
    }
    return HAL_OK;
}

//=============================================================================
// Функция чтения данных из EEPROM
static HAL_StatusTypeDef EEPROM_ReadData(uint16_t memAddr, uint8_t *pData, uint16_t size)
{
    // Проверяем, готово ли устройство (EEPROM) к работе
    if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_I2C_ADDRESS, 3, 10) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_EEPROM_READY_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_EEPROM_READY_ERROR;
    // Если устройство готово, выполняем чтение
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1,
                                                EEPROM_I2C_ADDRESS,
                                                memAddr,
                                                I2C_MEMADD_SIZE_8BIT,
                                                pData,
                                                size,
                                                1000);
    if (status == HAL_TIMEOUT) {
        ERRCODE.STATUS |= STATUS_EEPROM_TIMEOUT_I2C_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_EEPROM_TIMEOUT_I2C_ERROR;

    if (status == HAL_BUSY) {
        ERRCODE.STATUS |= STATUS_EEPROM_READY_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_EEPROM_READY_ERROR;
    if (status == HAL_ERROR) {
        ERRCODE.STATUS |= STATUS_EEPROM_READ_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS  &= ~STATUS_EEPROM_READ_ERROR;
    return HAL_OK;
}

//=============================================================================
// Внутренняя функция записи EepromRecord в EEPROM (начиная с EEPROM_START_ADDR)
static HAL_StatusTypeDef WriteSlot(const EepromRecord *record) {
    uint8_t buffer[sizeof(EepromRecord)];
    // Копируем всю структуру в буфер
    memcpy(buffer, record, sizeof(EepromRecord));
    // Обнуляем поле crc перед вычислением (находимся в конце структуры)
    memset(buffer + offsetof(EepromRecord, crc), 0, sizeof(record->crc));

    // Вычисляем CRC по всему буферу (где crc = 0)
    uint32_t crc = CalculateCRC(buffer, sizeof(EepromRecord));

    // Записываем CRC в буфер в Little-Endian порядке
    buffer[offsetof(EepromRecord, crc) + 0] = (uint8_t)(crc & 0xFF);
    buffer[offsetof(EepromRecord, crc) + 1] = (uint8_t)((crc >> 8) & 0xFF);
    buffer[offsetof(EepromRecord, crc) + 2] = (uint8_t)((crc >> 16) & 0xFF);
    buffer[offsetof(EepromRecord, crc) + 3] = (uint8_t)((crc >> 24) & 0xFF);

    if (EEPROM_WriteData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord)) != HAL_OK) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

//=============================================================================
// Внутренняя функция чтения EepromRecord из EEPROM (начиная с EEPROM_START_ADDR)
// Функция также проверяет сигнатуру, версию формата и CRC.
static HAL_StatusTypeDef ReadSlot(EepromRecord *record) {
    uint8_t buffer[sizeof(EepromRecord)];

    if (EEPROM_ReadData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord)) != HAL_OK) {
        return HAL_ERROR;
    }

    memcpy(record, buffer, sizeof(EepromRecord));

    if (record->signature != EEPROM_SIGNATURE) {
        return HAL_ERROR;
    }

    if (record->formatVersion != EEPROM_FORMAT_VERSION) {
        return HAL_ERROR;
    }

    uint32_t savedCrc = record->crc;
    // Обнуляем поле crc в буфере для вычисления контрольной суммы
    memset(buffer + offsetof(EepromRecord, crc), 0, sizeof(record->crc));
    uint32_t calcCrc = CalculateCRC(buffer, sizeof(EepromRecord));

    if (calcCrc != savedCrc) {
        // Если контрольная сумма не совпадает, данные некорректны
        ERRCODE.STATUS |= STATUS_EEPROM_CRC_ERROR;
        return HAL_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_EEPROM_CRC_ERROR; 
    return HAL_OK;
}

//=============================================================================
// Функция проверки существования валидных данных в EEPROM
HAL_StatusTypeDef EEPROM_IsDataExists(void) {
    if (ERRCODE.STATUS & STATUS_EEPROM_INIT_ERROR) return HAL_ERROR;
    EepromRecord record;
    return ReadSlot(&record);
}

//=============================================================================
// Функция загрузки настроек из EEPROM
HAL_StatusTypeDef EEPROM_LoadSettings(EEPROM_Settings_item *dst) {
    if (ERRCODE.STATUS & STATUS_EEPROM_INIT_ERROR) return HAL_ERROR;
    if (!dst) {
        return HAL_ERROR;
    }
    

    EepromRecord record;
    if (ReadSlot(&record) != HAL_OK) {
        return HAL_ERROR;
    }

    memcpy(dst, &record.data, sizeof(EEPROM_Settings_item));
    return HAL_OK;
}

//=============================================================================
// Функция сохранения настроек в EEPROM
HAL_StatusTypeDef EEPROM_SaveSettings(EEPROM_Settings_item *src) {
    if (ERRCODE.STATUS & STATUS_EEPROM_INIT_ERROR) return HAL_ERROR;
    if (!src) {
        return HAL_ERROR;
    }
    EEPROM_Settings_item currentSettings;
    if (EEPROM_LoadSettings(&currentSettings) == HAL_OK) {
        // Если данные совпадают, то повторную запись не выполняем
        if (memcmp(src, &currentSettings, sizeof(EEPROM_Settings_item)) == 0) {
            // Данные уже сохранены, записывать повторно не нужно
            return HAL_OK;
        }
    }


    EepromRecord newRecord;
    memset(&newRecord, 0, sizeof(newRecord));
    newRecord.signature     = EEPROM_SIGNATURE;
    newRecord.formatVersion = EEPROM_FORMAT_VERSION;
    newRecord.version       = 1; //! добавить подсчет ресурса
    memcpy(&newRecord.data, src, sizeof(EEPROM_Settings_item));
    // Поле crc заполняется внутри WriteSlot

    return WriteSlot(&newRecord);
}

//=============================================================================
// Новая функция для проверки корректности сохранённых данных
HAL_StatusTypeDef EEPROM_CheckDataValidity(void) {
    if (ERRCODE.STATUS & STATUS_EEPROM_INIT_ERROR) return HAL_ERROR;

    uint8_t buffer[sizeof(EepromRecord)];

    // Считываем данные из EEPROM
    if (EEPROM_ReadData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord)) != HAL_OK) {
        return HAL_ERROR;
    }

    // Проверяем сигнатуру и версию формата
    EepromRecord *record = (EepromRecord *)buffer;
    if (record->signature != EEPROM_SIGNATURE) {
        return HAL_ERROR;
    }
    if (record->formatVersion != EEPROM_FORMAT_VERSION) {
        return HAL_ERROR;
    }

    // Сохраняем записанный CRC
    uint32_t savedCrc = record->crc;
    // Обнуляем CRC в буфере перед вычислением
    memset(buffer + offsetof(EepromRecord, crc), 0, sizeof(record->crc));
    uint32_t calcCrc = CalculateCRC(buffer, sizeof(EepromRecord));

    // Если контрольная сумма не совпадает, данные некорректны
    if ((calcCrc != savedCrc)){
        return HAL_ERROR;
    }
    return HAL_OK;
}
