#include "AT24C02.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include <stddef.h>  // Для offsetof

extern I2C_HandleTypeDef hi2c1;

// Убедимся, что размер EepromRecord не превышает размер EEPROM (256 байт)
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
static bool EEPROM_WriteData(uint16_t memAddr, const uint8_t *pData, uint16_t size) {
    uint16_t bytesWritten = 0;
    HAL_StatusTypeDef status;

    while (bytesWritten < size) {
        uint16_t currentAddr = memAddr + bytesWritten;
        uint8_t pageOffset   = currentAddr % EEPROM_PAGE_SIZE;
        uint8_t bytesToWrite = EEPROM_PAGE_SIZE - pageOffset;

        if (bytesToWrite > (size - bytesWritten)) {
            bytesToWrite = (size - bytesWritten);
        }

        status = HAL_I2C_Mem_Write(&hi2c1,
                                   EEPROM_I2C_ADDRESS,
                                   currentAddr,
                                   I2C_MEMADD_SIZE_8BIT, // Важно для AT24C02
                                   (uint8_t *)(pData + bytesWritten),
                                   bytesToWrite,
                                   1000);
        if (status != HAL_OK) {
            return false;
        }

        // Задержка для завершения внутренней записи EEPROM (обычно 5 мс)
        HAL_Delay(5);

        bytesWritten += bytesToWrite;
    }
    return true;
}

//=============================================================================
// Функция чтения данных из EEPROM
static bool EEPROM_ReadData(uint16_t memAddr, uint8_t *pData, uint16_t size) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c1,
                                                EEPROM_I2C_ADDRESS,
                                                memAddr,
                                                I2C_MEMADD_SIZE_8BIT, // Важно
                                                pData,
                                                size,
                                                1000);
    return (status == HAL_OK);
}

//=============================================================================
// Внутренняя функция записи EepromRecord в EEPROM (начиная с EEPROM_START_ADDR)
static bool WriteSlot(const EepromRecord *record) {
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

    if (!EEPROM_WriteData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord))) {
        return false;
    }
    return true;
}

//=============================================================================
// Внутренняя функция чтения EepromRecord из EEPROM (начиная с EEPROM_START_ADDR)
// Функция также проверяет сигнатуру, версию формата и CRC.
static bool ReadSlot(EepromRecord *record) {
    uint8_t buffer[sizeof(EepromRecord)];

    if (!EEPROM_ReadData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord))) {
        return false;
    }

    memcpy(record, buffer, sizeof(EepromRecord));

    if (record->signature != EEPROM_SIGNATURE) {
        return false;
    }

    if (record->formatVersion != EEPROM_FORMAT_VERSION) {
        return false;
    }

    uint32_t savedCrc = record->crc;
    // Обнуляем поле crc в буфере для вычисления контрольной суммы
    memset(buffer + offsetof(EepromRecord, crc), 0, sizeof(record->crc));
    uint32_t calcCrc = CalculateCRC(buffer, sizeof(EepromRecord));

    if (calcCrc != savedCrc) {
        return false;
    }

    return true;
}

//=============================================================================
// Функция проверки существования валидных данных в EEPROM
bool EEPROM_IsDataExists(void) {
    EepromRecord record;
    return ReadSlot(&record);
}

//=============================================================================
// Функция загрузки настроек из EEPROM
bool EEPROM_LoadSettings(EEPROM_Settings_item *dst) {
    if (!dst) {
        return false;
    }

    EepromRecord record;
    if (!ReadSlot(&record)) {
        return false;
    }

    memcpy(dst, &record.data, sizeof(EEPROM_Settings_item));
    return true;
}

//=============================================================================
// Функция сохранения настроек в EEPROM
bool EEPROM_SaveSettings(const EEPROM_Settings_item *src) {
    if (!src) {
        return false;
    }

    EepromRecord newRecord;
    memset(&newRecord, 0, sizeof(newRecord));

    newRecord.signature     = EEPROM_SIGNATURE;
    newRecord.formatVersion = EEPROM_FORMAT_VERSION;
    newRecord.version       = 1;  // Можно использовать как счётчик версий
    memcpy(&newRecord.data, src, sizeof(EEPROM_Settings_item));
    // Поле crc заполняется внутри WriteSlot

    return WriteSlot(&newRecord);
}

//=============================================================================
// Новая функция для проверки корректности сохранённых данных
bool EEPROM_CheckDataValidity(void) {
    uint8_t buffer[sizeof(EepromRecord)];

    // Считываем данные из EEPROM
    if (!EEPROM_ReadData(EEPROM_START_ADDR, buffer, sizeof(EepromRecord))) {
        return false;
    }

    // Проверяем сигнатуру и версию формата
    EepromRecord *record = (EepromRecord *)buffer;
    if (record->signature != EEPROM_SIGNATURE) {
        return false;
    }
    if (record->formatVersion != EEPROM_FORMAT_VERSION) {
        return false;
    }

    // Сохраняем записанный CRC
    uint32_t savedCrc = record->crc;
    // Обнуляем CRC в буфере перед вычислением
    memset(buffer + offsetof(EepromRecord, crc), 0, sizeof(record->crc));
    uint32_t calcCrc = CalculateCRC(buffer, sizeof(EepromRecord));

    // Если контрольная сумма не совпадает, данные некорректны
    return (calcCrc == savedCrc);
}
