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

// ------------------ Параметры организации памяти ------------------
#define SECTOR_SIZE         4096            // Размер сектора (4 КБ)
#define SECTOR_HEADER_SIZE  2               // 2 байта в начале сектора – служебный заголовок
#define RECORD_SIZE         128             // Размер одной записи (128 байт)
#define RECORDS_PER_SECTOR  ((SECTOR_SIZE - SECTOR_HEADER_SIZE) / RECORD_SIZE)
#define FLASH_TOTAL_SIZE    (16 * 1024 * 1024)
#define TOTAL_SECTORS       (FLASH_TOTAL_SIZE / SECTOR_SIZE)
#define TOTAL_RECORDS       (TOTAL_SECTORS * RECORDS_PER_SECTOR)

// Изменили overhead на 10 байт, итого полезных данных на 2 байта меньше
#define RECORD_OUTPUT_SIZE  (RECORD_SIZE - 10 + 1)  // для вывода (были -8+1, теперь -10+1)

// ------------------ Определения состояний заголовка сектора (2 байта) ------------------
#define SECTOR_EMPTY   0xFFFF  // после стирания
#define SECTOR_INUSE   0x00FF  // сектор в использовании (первый фрагмент записан)
#define SECTOR_FULL    0x0000  // сектор полностью заполнен

// Флаг завершённой записи (для поля rec_status_end)
#define RECORD_COMPLETE_FLAG 0xAA

// ------------------ Структура записи (128 байт) ------------------
/*
   Поля:
   - sector_header (2 байта)        : используется только в ПЕРВОМ блоке сектора.
   - length (2 байта)              : длина полезных данных (максимум 120)
   - rec_status_start (1 байт)     : 0x00 при начале записи
   - rec_status_end (1 байт)       : 0xFF, после завершения -> RECORD_COMPLETE_FLAG(0xAA)
   - sent_marker_sector (1 байт)   : 0xFF=сектор не отправлен, 0x00=весь сектор отправлен (значим только в block=0)
   - sent_marker_block (1 байт)    : 0xFF=блок не отправлен, 0x00=этот блок отправлен
   - data[RECORD_SIZE - 10]        : полезные данные (118 байт)
*/

extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
extern FATFS USBFatFs;    /* Объект файловой системы для USB диска */
extern FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */
extern EEPROM_Settings_item EEPROM;

#pragma pack(push, 1)
typedef struct {
    uint16_t sector_header;        // [0..1]
    uint16_t length;               // [2..3]
    uint8_t  rec_status_start;     // [4]
    uint8_t  rec_status_end;       // [5]
    uint8_t  sent_marker_sector;   // [6]
    uint8_t  sent_marker_block;    // [7]
    char     data[RECORD_SIZE - 10]; // [8..127], 118 байт
} record_t;
#pragma pack(pop)

// Глобальный указатель на следующий свободный фрагмент (номер записи по всему флешу)
extern uint32_t flash_end_ptr;

// ------------------ Прототипы низкоуровневых функций ------------------
int W25_Read_ID(uint32_t *id);
int W25_Reset(void);
int W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz);
int W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz);
uint8_t W25_Read_Status(void);
int W25_Write_Enable(void);
int W25_WaitForReady(uint32_t timeout_ms);
int W25_Erase_Sector(uint32_t addr);
int W25_Chip_Erase(void);

// ------------------ Прототипы основных функций ------------------
void w25_init(void);
int update_flash_end_ptr(void);
int flash_append_record(const char *record_data);
int flash_read_record_by_index(uint32_t record_block, char *buffer);

// ------------------ Прототип резервного копирования ------------------
int backup_records_to_external(void);

// ------------------ Новая функция ------------------
// Помечает указанный блок как отправленный (sent_marker_block=0x00).
// Если все блоки сектора отправлены, sent_marker_sector=0x00 в нулевом блоке сектора.
int mark_block_sent_and_check_sector(uint32_t record_block);
extern uint32_t flash_end_ptr;
#endif // W25Q128_H
