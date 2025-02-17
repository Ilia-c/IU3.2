#include "Sleep.h"

extern RTC_HandleTypeDef hrtc;

#include "stm32l4xx.h"
#include "stdint.h"


/**
 * @brief  Функция анализа флагов сброса и возврата кода сброса.
 * @return uint8_t: код сброса, один из RESET_CODE_XXX.
 *
 * Функция проверяет флаги в регистре RCC->CSR. Приоритет проверки можно задать
 * так, что если установлено несколько флагов, выбирается первый по порядку.
 * Здесь проверяется сначала внешний сброс (PINRST), затем программный (SFTRST),
 * потом сбросы от watchdog и т.д.
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

    // Очистка флагов сброса: установка бита RMVF в регистре RCC->CSR
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

// Возвращает количество дней в месяце (month=1..12) для года (20xx)
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
        return 31; // на всякий случай
    }
}

/**
 * @brief Установить Alarm A на "текущее время + hours:minutes".
 *        Ограничения:
 *         - hours [0..99]
 *         - minutes [5..59]
 * @param hours   Количество часов сна (0..99)
 * @param minutes Количество минут сна (5..59)
 */
void RTC_SetAlarm_HoursMinutes(uint8_t hours, uint8_t minutes)
{
    //if (minutes < 5)
    //    minutes = 5;
    if (minutes > 59)
        minutes = 59;
    if (hours > 99)
        hours = 99; 

    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    uint16_t currentDayMinutes = sTime.Hours * 60 + sTime.Minutes;
    uint16_t addMinutes = (uint16_t)(hours * 60 + minutes);
    uint32_t totalMinutes = (uint32_t)currentDayMinutes + addMinutes;

    uint32_t daysToAdd = totalMinutes / 1440;
    uint16_t futureDayMinutes = totalMinutes % 1440;

    uint8_t newHours = (uint8_t)(futureDayMinutes / 60);
    uint8_t newMinutes = (uint8_t)(futureDayMinutes % 60);

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

    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.Alarm = RTC_ALARM_A;

    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = sDate.Date;

    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;

    sAlarm.AlarmTime.Hours = newHours;
    sAlarm.AlarmTime.Minutes = newMinutes;
    sAlarm.AlarmTime.Seconds = 0;
    sAlarm.AlarmTime.SubSeconds = 0;
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT_24;

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

    //GPIO_InitStruct.Pin = GPIO_PIN_All;
    //HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //GPIO_InitStruct.Pin = GPIO_PIN_All;
    //HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
}



extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_sdmmc1;

// Если USB может работать как в режиме Device, так и в режиме Host,
// объявите оба дескриптора (если они используются):
extern PCD_HandleTypeDef hpcd_USB_OTG_FS; // для режима Device
extern HCD_HandleTypeDef hhcd_USB_OTG_FS; // для режима Host

void Enter_StandbyMode(uint8_t hours, uint8_t minutes)
{
    vTaskSuspendAll();

    HAL_RCC_DeInit();
    HAL_SuspendTick();
    GPIO_AnalogConfig();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    PWR->SCR |= (PWR_SCR_CWUF1 | PWR_SCR_CWUF2 | PWR_SCR_CWUF3 | PWR_SCR_CWUF4 | PWR_SCR_CWUF5);
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);


    //RTC_SetAlarm_HoursMinutes(hours, minutes);
    RTC_SetAlarm_HoursMinutes(0, 2);
    HAL_PWR_EnterSTANDBYMode();
}

// Для отладки FreeRTOS
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}