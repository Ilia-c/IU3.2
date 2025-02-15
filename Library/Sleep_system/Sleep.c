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



static inline uint8_t IsLeapYear(uint8_t rtcYear)
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
static uint8_t DaysInMonth(uint8_t month, uint8_t rtcYear)
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
    // 1) Подстрахуемся: если minutes < 5 — ставим 5,
    //    чтобы не было "меньше 5 минут"
    if (minutes < 5)
        minutes = 5;
    if (minutes > 59)
        minutes = 59;
    if (hours > 99)
        hours = 99; // на всякий случай

    // 2) Деактивируем Alarm A на случай, если он был установлен
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

    // 3) Считываем ТЕКУЩИЕ время и дату
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // 4) Преобразуем текущее время в «минуты с начала суток»
    //    и прибавим желаемое количество минут
    uint16_t currentDayMinutes = sTime.Hours * 60 + sTime.Minutes;
    uint16_t addMinutes = (uint16_t)(hours * 60 + minutes);
    uint32_t totalMinutes = (uint32_t)currentDayMinutes + addMinutes;

    // 5) Сколько дней прибавляется (каждые 1440 мин = сутки)
    uint32_t daysToAdd = totalMinutes / 1440;
    uint16_t futureDayMinutes = totalMinutes % 1440;

    // Выделяем новые часы и минуты в будущем
    uint8_t newHours = (uint8_t)(futureDayMinutes / 60);
    uint8_t newMinutes = (uint8_t)(futureDayMinutes % 60);

    // 6) Прибавляем daysToAdd к текущей дате (sDate.Date)
    //    с учётом перехода месяцев и лет (в 20xx диапазоне)
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
                // Если Year==100 => 2100 год => календарь RTC уже не совсем корректен,
                // но в данной задаче предположим, что столько не спим :)
            }
        }
        daysToAdd--;
    }

    // 7) Заполняем структуру Alarm A
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.Alarm = RTC_ALARM_A;

    // AlarmDateWeekDaySel = DATE => указываем число месяца
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = sDate.Date;

    // Маски нет => сравниваем по дате, часам, минутам, секундам
    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;

    // Заполняем время
    sAlarm.AlarmTime.Hours = newHours;
    sAlarm.AlarmTime.Minutes = newMinutes;
    sAlarm.AlarmTime.Seconds = 0;
    sAlarm.AlarmTime.SubSeconds = 0;
    // Если RTC у вас в 24-часовом формате, TimeFormat=0
    // (в STM32L4 обычно так)
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;

    // 8) Устанавливаем Alarm с прерыванием
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

void Enter_StandbyMode(uint8_t hours, uint8_t minutes)
{
    // Отключим всё лишнее, остановим таймер SysTick и т.д.
    HAL_RCC_DeInit();
    HAL_SuspendTick();

    // Переводим GPIO в аналоговый режим
    GPIO_AnalogConfig();

    // Обязательно включить тактирование PWR и доступ к Backup-домену
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // Очистим все возможные флаги пробуждения (WUFx),
    // чтобы "старые" не выдернули нас из Standby сразу же
    PWR->SCR |= (PWR_SCR_CWUF1 | PWR_SCR_CWUF2 | PWR_SCR_CWUF3 |
                 PWR_SCR_CWUF4 | PWR_SCR_CWUF5);

    // Настраиваем Alarm A на «hours:minutes» от текущего момента
    RTC_SetAlarm_HoursMinutes(hours, minutes);

    // Входим в Standby
    HAL_PWR_EnterSTANDBYMode();
    // После этого МК «засыпает», а проснётся (Reset) при срабатывании Alarm
}

uint8_t Check_Wakeup_Reason(void)
{
    // 1) Включаем клок PWR, разрешаем доступ к Backup (если RTC)
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // 2) Проверяем, установлен ли флаг Standby?
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
    {
        // Да, мы вышли из Standby

        // Теперь проверяем флаги пробуждения
        if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF2) != RESET)
        {
            // Пробуждение по RTC Alarm (или WKUP2, но чаще Alarm)
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF2);
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
            return 1; // «проснулись» из Standby по RTC
        }
        else if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF1) != RESET)
        {
            // Возможно, сработал WKUP1
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF1);
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
            return 2; // «проснулись» из Standby по WKUP1
        }
        else
        {
            // Ни WUF1, ни WUF2... Может WUF3..WUF5
            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
            return 3; // Некий другой вариант
        }
    }
    else
    {
        // SB=0 => это обычный холодный сброс
        return 0;
    }
}

// Для отладки FreeRTOS
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void None_func() {}