#ifndef __AT24C02_H
#define __AT24C02_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Settings.h"

/* Параметры EEPROM */
#define EEPROM_PAGE_SIZE        8U
#define EEPROM_TOTAL_SIZE       256U
#define EEPROM_START_ADDR       0x0000
#define EEPROM_SIGNATURE        0xDEADBEEF
#define EEPROM_I2C_ADDRESS      (0x50 << 1)

/* Структура, сохраняемая в EEPROM.
 * Использование __attribute__((packed)) позволяет избежать лишнего выравнивания,
 * что критично для точного соответствия размера данных EEPROM.
 */
typedef struct __attribute__((packed)) {
    uint32_t signature;        // Магическая сигнатура для проверки наличия корректных данных
    uint32_t formatVersion;    // Версия формата сохранённых данных
    uint32_t version;          // Номер версии/счётчик обновлений
    EEPROM_Settings_item data; // Пользовательские настройки (определяются в "Settings.h")
    uint32_t crc;              // Контрольная сумма (CRC)
} EepromRecord;

/* Функции работы с EEPROM */

/**
 * @brief Сохраняет настройки в EEPROM.
 * @param src Указатель на структуру с настройками.
 * @return true, если запись прошла успешно, иначе false.
 */
HAL_StatusTypeDef EEPROM_SaveSettings(const EEPROM_Settings_item *src);

/**
 * @brief Загружает настройки из EEPROM.
 * @param dst Указатель на структуру, куда будут загружены настройки.
 * @return true, если загрузка прошла успешно, иначе false.
 */
HAL_StatusTypeDef EEPROM_LoadSettings(EEPROM_Settings_item *dst);

/**
 * @brief Проверяет, существуют ли валидные данные в EEPROM.
 * @return true, если данные существуют и корректны, иначе false.
 */
HAL_StatusTypeDef EEPROM_IsDataExists(void);

/**
 * @brief Проверяет корректность сохранённых данных в EEPROM.
 *
 * Функция выполняет считывание блока данных и проверяет сигнатуру, версию формата
 * и контрольную сумму (CRC). Если все проверки пройдены, данные считаются корректными.
 *
 * @return true, если данные корректны, иначе false.
 */
HAL_StatusTypeDef EEPROM_CheckDataValidity(void);

#ifdef __cplusplus
}
#endif

#endif // __AT24C02_H
