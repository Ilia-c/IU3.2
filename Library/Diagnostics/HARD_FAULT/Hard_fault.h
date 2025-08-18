#ifndef HARD_FAULT_H
#define HARD_FAULT_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_flash.h"
#include "stm32l4xx_hal_flash_ex.h"
#include "RTC_data.h"
#include "Settings.h"
#include <stdint.h>
#include <string.h>
#include "stm32l4xx.h"
#include "FreeRTOS.h"
#include "task.h"

// Адрес резервной страницы (2 КБ) в конце 512 КБ Flash
#define BACKUP_HARD_ADDRESS   ((uint32_t)0x0807F000U) // 512 КБ Flash, 2 КБ резервной страницы
#define FLASH_PAGE_SIZE_BYTES ((uint32_t)0x800U)    // 2 KB
#define FAULTLOG_MAGIC        0xA5A5A5A5U           // Магическое число
#define STACK_DUMP_WORDS      16                    // Размер дампа стека



extern RTC_HandleTypeDef hrtc;

// Структура дампа состояния
typedef struct __attribute__((packed)) {
    uint32_t magic;           // 0xA5A5A5A5
    uint32_t err_lo, err_hi;  // ERRCODE.STATUS
    uint32_t cfsr;            // Configurable Fault Status Register
    uint32_t hfsr;            // HardFault Status Register
    uint32_t dfsr;            // Debug Fault Status Register
    uint32_t afsr;            // Auxiliary Fault Status Register
    uint32_t shcsr;           // System Handler Control and State Register
    uint32_t icsr;            // Interrupt Control and State Register
    uint32_t mmfar;           // MemManage Fault Address Register
    uint32_t bfar;            // BusFault Address Register
    uint64_t state;
    // Сохраняем полный набор регистров
    uint32_t r0, r1, r2, r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;

    uint32_t exc_return;      // код возврата исключения

    uint32_t timestamp;       // отметка времени

    // RTOS-контекст:
    uint32_t task_id;         // идентификатор текущей задачи
    uint32_t sys_tick;        // значение SysTick

    // Снимок стека
    uint32_t stack[STACK_DUMP_WORDS];
} FaultLog_t;

// API функций
void FlashBackup_Save(void);
void FlashBackup_Clear(void);
uint8_t FlashBackup_IsEmpty(void);

// Получить указатель на дамп
static inline FaultLog_t *FlashBackup_GetLog(void) {
    return (FaultLog_t *)BACKUP_HARD_ADDRESS;
}

#endif // HARD_FAULT_H