#include "Hard_fault.h"

extern RTC_HandleTypeDef hrtc;

// ��������� ��������� �����
__attribute__((aligned(8))) // ������������ ��� ���������� ������
static FaultLog_t faultLog = {
    .magic = FAULTLOG_MAGIC
};

// �������� ������� ��������� ��������
uint8_t FlashBackup_IsEmpty(void) {
    return (*(volatile uint32_t*)BACKUP_HARD_ADDRESS == 0xFFFFFFFF);
}


// �������� ������ �������� �� DBANK: 2 �� (dual) ��� 4 �� (single)
static inline uint32_t Flash_PageSizeBytes(void) {
#ifdef FLASH_OPTR_DBANK
    return (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK) ? 0x800u : 0x1000u);
#else
    return 0x800u; // ������
#endif
}


static uint32_t Flash_GetPage(uint32_t addr)
{
    const uint32_t psz = Flash_PageSizeBytes();
#if defined(FLASH_BANK2) && defined(FLASH_BANK_SIZE)
#ifdef FLASH_OPTR_DBANK
    if (READ_BIT(FLASH->OPTR, FLASH_OPTR_DBANK)) {
        // dual-bank: 2 ����� �� FLASH_BANK_SIZE
        if (addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
            return (addr - FLASH_BASE) / psz;                   // ���� 1
        } else {
            return (addr - (FLASH_BASE + FLASH_BANK_SIZE)) / psz; // ���� 2
        }
    }
#endif
    // single-bank ��� ��� DBANK-�������
    return (addr - FLASH_BASE) / psz;
#else
    return (addr - FLASH_BASE) / psz;
#endif
}


// �������� ��������� ��������
HAL_StatusTypeDef FlashBackup_Clear(void) {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_PROGERR | FLASH_FLAG_WRPERR |
                           FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_PGSERR |
                           FLASH_FLAG_MISERR | FLASH_FLAG_FASTERR);  // �������� ������� ������
                           
    FLASH_EraseInitTypeDef eraseInfo = {0};
    uint32_t pageErr = 0;
    eraseInfo.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInfo.Banks     = FLASH_BANK_2;         
    eraseInfo.Page      = 0x7E;
    eraseInfo.NbPages   = 1;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInfo, &pageErr);
    volatile uint32_t first = *(volatile uint32_t*)BACKUP_HARD_ADDRESS;
    HAL_FLASH_Lock();
    return status;
}

// ������ ����� � Flash
static void FlashBackup_Write(void) {
    // �������� �� ������� ��������
    if (!FlashBackup_IsEmpty()) {
        return;
    }

    // �������� ��������
    FlashBackup_Clear();
    
    // �������� ����� ��������
    if (!FlashBackup_IsEmpty()) {
        return;
    }

    // ������ 64-������� ������� (��� STM32L4)
    HAL_FLASH_Unlock();
    
    const uint64_t *src = (const uint64_t *)&faultLog;
    uint64_t *dst = (uint64_t *)BACKUP_HARD_ADDRESS;
    uint32_t words = (sizeof(FaultLog_t) + 7) / 8; // ���������� ����� �� 64-������ ����
    
    for (uint32_t i = 0; i < words; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,  // ��� STM32L4 ������������ DOUBLEWORD
                         (uint32_t)&dst[i], 
                         src[i]);
    }
    
    HAL_FLASH_Lock();
}

static inline int is_code_addr(uint32_t a)
{
    a &= ~1u; // Thumb bit
    return (a >= FLASH_CODE_LO) && (a < FLASH_CODE_HI);
}

static size_t collect_bt_scan(uint32_t *sp, uint32_t *sp_limit, uint32_t *out, size_t max_out)
{
    size_t n = 0;

    // 1) ������� PC/LR � ������� � ������, ���� �������� �������������
    extern FaultLog_t faultLog;
    if (faultLog.pc && is_code_addr(faultLog.pc) && n < max_out) out[n++] = (faultLog.pc & ~1u);
    if (faultLog.lr && is_code_addr(faultLog.lr) && n < max_out) out[n++] = (faultLog.lr & ~1u);

    // 2) ������� ���� �� ����������� �����
    for (uint32_t *p = sp; p < sp_limit && n < max_out; ++p) {
        uint32_t w = *p;
        if (is_code_addr(w)) {
            uint32_t addr = (w & ~1u);
            if (n == 0 || out[n-1] != addr) out[n++] = addr; // ����-���� ������������
        }
    }
    return n;
}

// Naked-������ ��� ������� ����-������
__attribute__((naked))
void HardFault_Handler(void) {
    __asm volatile(
        "TST lr, #4        \n"
        "ITE EQ            \n"
        "MRSEQ r0, MSP     \n"
        "MRSNE r0, PSP     \n"
        "B HardFault_HandlerC"
    );
}



// C-����������
void HardFault_HandlerC(uint32_t *stk) {
    // 1. ��������� ���� ������ � RTC
    __disable_irq();
    ERRCODE.STATUS |= STATUS_HARDFAULT_OCCURRED;
    uint32_t lo = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFFU);
    uint32_t hi = (uint32_t)(ERRCODE.STATUS >> 32);
    
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, lo);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, hi);

    // 2. ��������� ���������������� ���������
    faultLog.state  = ((uint64_t)(EEPROM.Mode               & 0x0F) <<  (4 * 0));
    faultLog.state |= ((uint64_t)(EEPROM.Communication      & 0x0F) <<  (4 * 1));
    faultLog.state |= ((uint64_t)(EEPROM.RS485_prot         & 0x0F) <<  (4 * 2));
    faultLog.state |= ((uint64_t)(EEPROM.units_mes[0]       & 0x0F) <<  (4 * 3));
    faultLog.state |= ((uint64_t)(EEPROM.screen_sever_mode  & 0x0F) <<  (4 * 4));
    faultLog.state |= ((uint64_t)(EEPROM.USB_mode           & 0x0F) <<  (4 * 5));
    faultLog.state |= ((uint64_t)(EEPROM.Save_in            & 0x0F) <<  (4 * 6));
    faultLog.state |= ((uint64_t)(EEPROM.len                & 0x0F) <<  (4 * 7));
    faultLog.state |= ((uint64_t)(EEPROM.mode_ADC[0]        & 0x0F) <<  (4 * 8));
    faultLog.state |= ((uint64_t)(EEPROM.block              & 0x0F) <<  (4 * 9));

    // 3. ��������� �������� SCB
    faultLog.err_lo = lo;
    faultLog.err_hi = hi;
    faultLog.cfsr   = SCB->CFSR;
    faultLog.hfsr   = SCB->HFSR;
    faultLog.dfsr   = SCB->DFSR;
    faultLog.afsr   = SCB->AFSR;
    faultLog.shcsr  = SCB->SHCSR;
    faultLog.icsr   = SCB->ICSR;
    faultLog.mmfar  = SCB->MMFAR;
    faultLog.bfar   = SCB->BFAR;

    // 4. ������ ��������� �� ����-������
    faultLog.r0   = stk[0];
    faultLog.r1   = stk[1];
    faultLog.r2   = stk[2];
    faultLog.r3   = stk[3];
    faultLog.r12  = stk[4];
    faultLog.lr   = stk[5];
    faultLog.pc   = stk[6];
    faultLog.psr  = stk[7];

    faultLog.sp_at_fault = (uint32_t)stk;
    faultLog.msp = __get_MSP();
    faultLog.psp = __get_PSP();
    faultLog.control = __get_CONTROL();
    faultLog.ccr = SCB->CCR;

    // 5. ��� �������� ����������
    uint32_t excRet;
    __asm volatile("MOV %0, LR" : "=r" (excRet));
    faultLog.exc_return = excRet;

    // 6. RTOS � ������
    faultLog.task_id = (uint32_t)xTaskGetCurrentTaskHandle();
    faultLog.sys_tick = HAL_GetTick();
    faultLog.timestamp = HAL_GetTick();  // ��������� ������� �����

    // 7. ������ ����� (�����������)
    memcpy(faultLog.stack, stk, sizeof(faultLog.stack));

    uint32_t *sp = (uint32_t *)stk;
    uint32_t *sp_limit = sp + (sizeof(faultLog.stack) / sizeof(faultLog.stack[0]));
    faultLog.bt_count = (uint32_t)collect_bt_scan(sp, sp_limit, faultLog.bt, BT_DEPTH);

    // 8. �������� � ������ �����
    if (FlashBackup_IsEmpty()) {
        FlashBackup_Write();
    }
    // 9. ������������
    NVIC_SystemReset();
    while (1) {}
}

// ������� ��� ������� ����������
void FlashBackup_Save(void) {
    FlashBackup_Write();
}

