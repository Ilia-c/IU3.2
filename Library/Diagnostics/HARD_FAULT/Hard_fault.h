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

#define BT_DEPTH            32          // ������� ������� backtrace ���������
#undef  STACK_DUMP_WORDS
#define STACK_DUMP_WORDS    256         // 256 ���� = 1 ��

#ifndef FLASH_CODE_LO
  #define FLASH_CODE_LO     0x08000000U
#endif
#ifndef FLASH_CODE_HI
  #define FLASH_CODE_HI     0x08080000U // ����� 512�� ����� (�� ������������)
#endif

extern RTC_HandleTypeDef hrtc;

// ��������� ����� ���������
typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint32_t err_lo, err_hi;
    uint32_t cfsr, hfsr, dfsr, afsr, shcsr, icsr, mmfar, bfar;
    uint64_t state;

    uint32_t r0, r1, r2, r3;
    uint32_t r12, lr, pc, psr;

    uint32_t exc_return;

    uint32_t timestamp;

    // RTOS
    uint32_t task_id;
    uint32_t sys_tick;

    // +++ �����: ��������� ��������/��������� ������
    uint32_t sp_at_fault;   // ����� stk, ���������� � ������� (PSP/MSP �� �����)
    uint32_t msp;
    uint32_t psp;
    uint32_t control;
    uint32_t ccr;           // SCB->CCR �� ������ �����

    // +++ �����: �������� (����� ������ ��������)
    uint32_t bt_count;
    uint32_t bt[BT_DEPTH];

    // ������ �����
    uint32_t stack[STACK_DUMP_WORDS];
} FaultLog_t;

// API �������
void FlashBackup_Save(void);
HAL_StatusTypeDef FlashBackup_Clear(void);
uint8_t FlashBackup_IsEmpty(void);

// �������� ��������� �� ����
static inline volatile FaultLog_t *FlashBackup_GetLog(void) {
    return (volatile FaultLog_t *)BACKUP_HARD_ADDRESS;
}

#endif // HARD_FAULT_H