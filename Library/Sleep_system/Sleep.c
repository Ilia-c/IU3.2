#include "Sleep.h"

extern RTC_HandleTypeDef hrtc;

#include "stm32l4xx.h"
#include "stdint.h"


/**
 * @brief  ������� ������� ������ ������ � �������� ���� ������.
 * @return uint8_t: ��� ������, ���� �� RESET_CODE_XXX.
 *
 * ������� ��������� ����� � �������� RCC->CSR. ��������� �������� ����� ������
 * ���, ��� ���� ����������� ��������� ������, ���������� ������ �� �������.
 * ����� ����������� ������� ������� ����� (PINRST), ����� ����������� (SFTRST),
 * ����� ������ �� watchdog � �.�.
 */
uint8_t GetResetCode(void)
{
    uint32_t flags = RCC->CSR;
    uint8_t resetCode = RESET_CODE_UNKNOWN;

    if (flags & RCC_CSR_PINRSTF) {
        resetCode = RESET_CODE_PINRST;
    }
    else if (flags & RCC_CSR_SFTRSTF) {
        resetCode = RESET_CODE_SFTRST;
    }
    else if (flags & RCC_CSR_IWDGRSTF) {
        resetCode = RESET_CODE_IWDG;
    }
    else if (flags & RCC_CSR_WWDGRSTF) {
        resetCode = RESET_CODE_WWDG;
    }
    else if (flags & RCC_CSR_LPWRRSTF) {
        resetCode = RESET_CODE_LPWR;
    }
    else if (flags & RCC_CSR_BORRSTF) {
        resetCode = RESET_CODE_BOR;
    }

    // ������� ������ ������: ��������� ���� RMVF � �������� RCC->CSR
    RCC->CSR |= RCC_CSR_RMVF;

    return resetCode;
}



uint8_t IsLeapYear(uint8_t rtcYear)
{
    uint16_t fullYear = 2000 + rtcYear;
    if ((fullYear % 400) == 0)
        return 1;
    if ((fullYear % 100) == 0)
        return 0;
    if ((fullYear % 4) == 0)
        return 1;
    return 0;
}

// ���������� ���������� ���� � ������ (month=1..12) ��� ���� (20xx)
uint8_t DaysInMonth(uint8_t month, uint8_t rtcYear)
{
    switch (month)
    {
    case 1:
        return 31; // Jan
    case 2:
        return (IsLeapYear(rtcYear) ? 29 : 28); // Feb
    case 3:
        return 31; // Mar
    case 4:
        return 30; // Apr
    case 5:
        return 31; // May
    case 6:
        return 30; // Jun
    case 7:
        return 31; // Jul
    case 8:
        return 31; // Aug
    case 9:
        return 30; // Sep
    case 10:
        return 31; // Oct
    case 11:
        return 30; // Nov
    case 12:
        return 31; // Dec
    default:
        return 31; // �� ������ ������
    }
}

/**
 * @brief ���������� Alarm A �� "������� ����� + hours:minutes".
 *        �����������:
 *         - hours [0..99]
 *         - minutes [5..59]
 * @param hours   ���������� ����� ��� (0..99)
 * @param minutes ���������� ����� ��� (5..59)
 */
void RTC_SetAlarm_HoursMinutes(uint8_t hours, uint8_t minutes)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    // ����������� ����������
    if (minutes > 59)
        minutes = 59;
    if (hours > 99)
        hours = 99;

    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

    // ���������� ���������� ������� ����� � ����
    RTC_TimeTypeDef sTime = Time_start;
    RTC_DateTypeDef sDate = Date_start;

    // ��������� ������ �� ������ ��� �� �������� �������
    uint16_t startMinutes = sTime.Hours * 60 + sTime.Minutes;
    uint16_t deltaMinutes = (uint16_t)(hours * 60 + minutes);
    uint32_t totalMinutes = startMinutes + deltaMinutes;

    // ����������, ������� ���� ���� ���������, ���� ����� ��������� �� ������� �����
    uint32_t daysToAdd = totalMinutes / 1440;
    uint16_t futureDayMinutes = totalMinutes % 1440;
    uint8_t newHours = futureDayMinutes / 60;
    uint8_t newMinutes = futureDayMinutes % 60;
    // ��������� ������� �� �������� �������
    uint8_t newSeconds = sTime.Seconds;

    // ���������� ��� � ����, ���� ����������
    while (daysToAdd > 0)
    {
        sDate.Date++;
        uint8_t mdays = DaysInMonth(sDate.Month, sDate.Year);
        if (sDate.Date > mdays)
        {
            sDate.Date = 1;
            sDate.Month++;
            if (sDate.Month > 12)
            {
                sDate.Month = 1;
                sDate.Year++;
            }
        }
        daysToAdd--;
    }

    // ��������� ����� ����� ������ �� ������ ����� ��� ������ ������� ����������
    uint32_t computedAlarmSeconds = newHours * 3600 + newMinutes * 60 + newSeconds;

    // �������� ������� ����� � ���� ��� ��������
    RTC_TimeTypeDef nowTime;
    RTC_DateTypeDef nowDate;
    HAL_RTC_GetTime(&hrtc, &nowTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &nowDate, RTC_FORMAT_BIN);

    // ����, ������������, ��� ����������� ����� ��������� � ������� (0 � ���, 1 � ��)
    int alarmInPast = 0;
    if ((nowDate.Year > sDate.Year) ||
        (nowDate.Year == sDate.Year && nowDate.Month > sDate.Month) ||
        (nowDate.Year == sDate.Year && nowDate.Month == sDate.Month && nowDate.Date > sDate.Date))
    {
        alarmInPast = 1;
    }
    else if (nowDate.Year == sDate.Year && nowDate.Month == sDate.Month && nowDate.Date == sDate.Date)
    {
        uint32_t nowSeconds = nowTime.Hours * 3600 + nowTime.Minutes * 60 + nowTime.Seconds;
        if (computedAlarmSeconds <= nowSeconds)
            alarmInPast = 1;
    }

    // ���� ������������ ����� ��� ������, ���������� 30 �����
    if (alarmInPast == 1)
    {
        uint32_t newTotalSeconds = computedAlarmSeconds + (30 * 60);
        // ���� ����� ����� ������� �� ������� �����, ���������� ���������� �������������� ����
        uint32_t extraDays = newTotalSeconds / 86400;
        uint32_t secondsOfDay = newTotalSeconds % 86400;
        newHours   = secondsOfDay / 3600;
        newMinutes = (secondsOfDay % 3600) / 60;
        newSeconds = secondsOfDay % 60;
        while (extraDays > 0)
        {
            sDate.Date++;
            uint8_t mdays = DaysInMonth(sDate.Month, sDate.Year);
            if (sDate.Date > mdays)
            {
                sDate.Date = 1;
                sDate.Month++;
                if (sDate.Month > 12)
                {
                    sDate.Month = 1;
                    sDate.Year++;
                }
            }
            extraDays--;
        }
    }

    // ��������� ��������� ����������
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.Alarm = RTC_ALARM_A;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = sDate.Date;
    sAlarm.AlarmTime.Hours   = newHours;
    sAlarm.AlarmTime.Minutes = newMinutes;
    sAlarm.AlarmTime.Seconds = newSeconds;
    sAlarm.AlarmTime.SubSeconds = 0;
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;
    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}


void GPIO_AnalogConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    GPIO_InitStruct.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_All;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
}



extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;

// ���� USB ����� �������� ��� � ������ Device, ��� � � ������ Host,
// �������� ��� ����������� (���� ��� ������������):
extern PCD_HandleTypeDef hpcd_USB_OTG_FS; // ��� ������ Device
extern HCD_HandleTypeDef hhcd_USB_OTG_FS; // ��� ������ Host

void Enter_StandbyMode(uint8_t hours, uint8_t minutes)
{
    vTaskSuspendAll();

    ERRCODE.STATUS = 0;
    uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
    uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
    

    HAL_ADC_DeInit(&hadc1);
    HAL_ADC_DeInit(&hadc3);
    __HAL_RCC_DMA2_CLK_DISABLE();
    HAL_PCD_DeInit(&hpcd_USB_OTG_FS);
    HAL_HCD_DeInit(&hhcd_USB_OTG_FS);
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE(); // �������� �������� ����, ����� ������������� �����
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12; // DM/DP
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    __HAL_RCC_GPIOA_CLK_DISABLE();
    DBGMCU->CR = 0x0;

    HAL_RCC_DeInit();
    HAL_SuspendTick();
    GPIO_AnalogConfig();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    PWR->SCR |= (PWR_SCR_CWUF1 | PWR_SCR_CWUF2 | PWR_SCR_CWUF3 | PWR_SCR_CWUF4 | PWR_SCR_CWUF5);
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);

    // 2) ������������ ���������� �wake-up� ����, ���� ��� ������ �� ������������
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);

    // 3) ���������� EXTI-�����, ��������� � RTC
    __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

    // 4) ���������� ��� ����� ����������� PWR
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    // (��� ���������� PWR->SCR |= �)

    RTC_SetAlarm_HoursMinutes(hours, minutes);
    HAL_PWR_EnterSTANDBYMode();
}

void Enter_StandbyMode_NoWakeup(void)
{
    // ������������ ���� ����� (���� ������������ FreeRTOS)
    vTaskSuspendAll();
    uint32_t errcode_low  = (uint32_t)(ERRCODE.STATUS & 0xFFFFFFFF);
    uint32_t errcode_high = (uint32_t)(ERRCODE.STATUS >> 32);
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_1, errcode_low);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INDEX_ERROR_CODE_2, errcode_high);
    // ��������������� ���������: ADC, USB � �.�.
    HAL_ADC_DeInit(&hadc1);
    HAL_ADC_DeInit(&hadc3);
    __HAL_RCC_DMA2_CLK_DISABLE();
    HAL_PCD_DeInit(&hpcd_USB_OTG_FS);
    HAL_HCD_DeInit(&hhcd_USB_OTG_FS);
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    // ������������� ����� USB � ����� ����������� �����
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    __HAL_RCC_GPIOA_CLK_DISABLE();

    // ���������� ��������� � ������ �������
    DBGMCU->CR = 0x0;
    HAL_RCC_DeInit();
    HAL_SuspendTick();
    GPIO_AnalogConfig();

    __HAL_RCC_PWR_CLK_ENABLE();
    // � Hardware WakeUp pins
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);
    // � RTC Alarm A/B
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_B);
    // � RTC WakeUp timer (���� �������������)
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
    // � ����� EXTI-������ ��� RTC
    __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
    __HAL_RTC_WAKEUPTIMER_EXTI_CLEAR_FLAG();

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // ������� ������ �����������
    PWR->SCR |= (PWR_SCR_CWUF1 | PWR_SCR_CWUF2 | PWR_SCR_CWUF3 | PWR_SCR_CWUF4 | PWR_SCR_CWUF5);
    HAL_PWR_EnterSTANDBYMode();
}

// ��� ������� FreeRTOS
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}