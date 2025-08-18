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

// �������� ��������� ��������
void FlashBackup_Clear(void) {
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef eraseInfo = {0};
    eraseInfo.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInfo.Page = (BACKUP_HARD_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE_BYTES;
    eraseInfo.NbPages = 1;
    uint32_t pageErr = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseInfo, &pageErr);
    HAL_FLASH_Lock();
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
    faultLog.state |= ((uint64_t)(EEPROM.units_mes          & 0x0F) <<  (4 * 3));
    faultLog.state |= ((uint64_t)(EEPROM.screen_sever_mode  & 0x0F) <<  (4 * 4));
    faultLog.state |= ((uint64_t)(EEPROM.USB_mode           & 0x0F) <<  (4 * 5));
    faultLog.state |= ((uint64_t)(EEPROM.Save_in            & 0x0F) <<  (4 * 6));
    faultLog.state |= ((uint64_t)(EEPROM.len                & 0x0F) <<  (4 * 7));
    faultLog.state |= ((uint64_t)(EEPROM.mode_ADC[0]           & 0x0F) <<  (4 * 8));
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

