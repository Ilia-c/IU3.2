#ifndef W25Q128_H
#define W25Q128_H

#include "main.h"
#include "Status_codes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "Settings.h"
#include "stm32l4xx_hal.h"
#include "USB_FATFS_SAVE.h"


// Команды флеш-памяти
#define W25_GET_JEDEC_ID    0x9F
#define W25_ENABLE_RESET    0x66
#define W25_RESET           0x99
#define W25_READ            0x03
#define W25_WRITE_ENABLE    0x06
#define W25_SECTOR_ERASE    0x20
#define W25_CHIP_ERASE      0xC7
#define W25_READ_STATUS_REG 0x05
#define W25_PAGE_PROGRAM    0x02


#define TIMEOUT_CHIP_ERASE_MS 40000

// Параметры организации памяти
#define SECTOR_SIZE         4096            // Размер сектора (4 КБ)
#define SECTOR_HEADER_SIZE  2               // 2 байта в начале сектора – служебный заголовок
#define RECORD_SIZE         128             // Размер одной записи (128 байт)
#define RECORDS_PER_SECTOR  ((SECTOR_SIZE - SECTOR_HEADER_SIZE) / RECORD_SIZE)
#define FLASH_TOTAL_SIZE    (16 * 1024 * 1024)
#define TOTAL_SECTORS       (FLASH_TOTAL_SIZE / SECTOR_SIZE)
#define TOTAL_RECORDS       (TOTAL_SECTORS * RECORDS_PER_SECTOR)
#define RECORD_OUTPUT_SIZE  (RECORD_SIZE - 8 + 1)  // 121 байт, если RECORD_SIZE==128


// Определения состояний заголовка сектора (2 байта)
#define SECTOR_EMPTY   0xFFFF  // после стирания
#define SECTOR_INUSE   0x00FF  // сектор в использовании (первый фрагмент записан, сектор не заполнен)
#define SECTOR_FULL    0x0000  // сектор полностью заполнен


// Флаг завершённой записи (для поля rec_status_end)
#define RECORD_COMPLETE_FLAG 0xAA


extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
extern FATFS USBFatFs;    /* Объект файловой системы для USB диска */
extern FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */
extern EEPROM_Settings_item EEPROM;
// Структура записи (128 байт)
// Если фрагмент – первый в секторе, первые 2 байта содержат заголовок сектора.
#pragma pack(push, 1)
typedef struct {
    uint16_t sector_header;   // используется только в первой записи сектора
    uint16_t length;          // длина полезных данных (максимум 122 байта)
    uint8_t rec_status_start; // при начале записи устанавливается в 0x00
    uint8_t rec_status_end;   // изначально 0xFF, после завершения записи обновляется до RECORD_COMPLETE_FLAG (0xAA)
    char data[RECORD_SIZE - 8];  // 122 байта полезных данных
} record_t;
#pragma pack(pop)

// Глобальный указатель на следующий свободный фрагмент (номер записи по всему флешу)
extern uint32_t flash_end_ptr;

// Прототипы низкоуровневых функций:
int W25_Read_ID(uint32_t *id);
int W25_Reset(void);
int W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz);
int W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz);
uint8_t W25_Read_Status(void);
int W25_Write_Enable(void);
int W25_WaitForReady(uint32_t timeout_ms);
int W25_Erase_Sector(uint32_t addr);
int W25_Chip_Erase(void);

// Прототипы функций работы с флешем/записями:
void w25_init(void);
int update_flash_end_ptr(void);
int flash_append_record(const char *record_data);
int flash_read_record_by_index(uint32_t record_block, char *buffer);
int backup_records_to_external(void);

#endif // W25Q128_H
