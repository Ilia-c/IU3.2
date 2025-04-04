#ifndef W25Q128_NEW_H
#define W25Q128_NEW_H

#include "main.h"
#include "Status_codes.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SD.h"

extern uint32_t g_total_records_count;
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

// Таймаут на полное стирание
#define TIMEOUT_CHIP_ERASE_MS 40000

// ------------------ Параметры организации памяти ------------------
#define SECTOR_SIZE         4096
#define RECORD_SIZE         128
#define RECORDS_PER_SECTOR  (SECTOR_SIZE / RECORD_SIZE)

// Полный размер флеш
#define FLASH_TOTAL_SIZE    ((15 * 1024 * 1024) + 512*1024)
#define TOTAL_SECTORS       (FLASH_TOTAL_SIZE / SECTOR_SIZE)
#define TOTAL_RECORDS       (TOTAL_SECTORS * RECORDS_PER_SECTOR)

// Макросы для "меток"
#define EMPTY  0xFF  // обозначение "пусто / стёрто"
#define WRITE_START 0xF0  //  Запись в сектор начата
#define SET    0x00  // обозначение "установлено / занято"

// Смещения внутри блока
#define SECTOR_MARK             0  // [0]
#define SECTOR_MARK_SEND        1  // [1]
#define BLOCK_MARK_WRITE_START  2  // [2]
#define BLOCK_MARK_WRITE_STOP   3  // [3]
#define BLOCK_MARK_DATA_SEND    4  // [4]
#define BLOCK_MARK_LEN          5  // [5]

// ------------------ Структура записи (128 байт) ------------------
typedef struct {
    uint8_t Sector_mark;         // [0]  : Пометка сектора (только для блок=0), иначе может быть 0xFF
    uint8_t Sector_mark_send;    // [1]  : Пометка, что сектор отправлен
    uint8_t rec_status_start;    // [2]  : Начало записи
    uint8_t rec_status_end;      // [3]  : Конец записи
    uint8_t block_mark_send;     // [4]  : Данные отправлены?
    uint8_t length;              // [5]  : Длина полезных данных
    char    data[RECORD_SIZE - 6]; // [6..127]
} record_t;

// Экспортируемые переменные (если нужны)
extern uint32_t flash_end_ptr;

// Прототипы низкоуровневых функций
void w25_init(void);
int  W25_Read_ID(uint32_t *id);
int  W25_Reset(void);
int  W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz);
int  W25_Write_Data(uint32_t addr, uint8_t *data, uint32_t sz);
uint8_t W25_Read_Status(void);
int  W25_Write_Enable(void);
int  W25_WaitForReady(uint32_t timeout_ms);
int  W25_Erase_Sector(uint32_t addr);
int  W25_Chip_Erase(void);

// Прототипы основных функций
int32_t search_sector_empty(void);
int flash_append_record(const char *record_data, uint8_t sector_mark_send);
int mark_block_sent(int32_t addr_block);
int Save_one_to_USB(void);
int backup_records_to_external(void);

// При желании – функции для работы с именем файла, USB и т.д.
void createFilename(char *dest, size_t destSize);

#endif // W25Q128_NEW_H
