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

// ����� ��������� �������� (2 ��) � ����� 512 �� Flash
#define BACKUP_HARD_ADDRESS   ((uint32_t)0x0807F000U) // 512 �� Flash, 2 �� ��������� ��������
#define FLASH_PAGE_SIZE_BYTES ((uint32_t)0x800U)    // 2 KB
#define FAULTLOG_MAGIC        0xA5A5A5A5U           // ���������� �����
#define STACK_DUMP_WORDS      16                    // ������ ����� �����



extern RTC_HandleTypeDef hrtc;

// ��������� ����� ���������
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
    // ��������� ������ ����� ���������
    uint32_t r0, r1, r2, r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;

    uint32_t exc_return;      // ��� �������� ����������

    uint32_t timestamp;       // ������� �������

    // RTOS-��������:
    uint32_t task_id;         // ������������� ������� ������
    uint32_t sys_tick;        // �������� SysTick

    // ������ �����
    uint32_t stack[STACK_DUMP_WORDS];
} FaultLog_t;

// API �������
void FlashBackup_Save(void);
void FlashBackup_Clear(void);
uint8_t FlashBackup_IsEmpty(void);

// �������� ��������� �� ����
static inline FaultLog_t *FlashBackup_GetLog(void) {
    return (FaultLog_t *)BACKUP_HARD_ADDRESS;
}

#endif // HARD_FAULT_H