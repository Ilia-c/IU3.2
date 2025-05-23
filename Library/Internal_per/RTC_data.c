#include "RTC_data.h"
#include "Display.h"

extern menuItem *selectedMenuItem;
extern int mode_redact;

RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;

#define LSI_TIMEOUT 1000  // Таймаут в миллисекундах для ожидания готовности LSI

void RTC_Init(void)
{
    // Первоначальная настройка RTC для внешнего источника (например, LSE)
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
		ERRCODE.STATUS |= ERROR_RTC;
        // Если инициализация не удалась, переходим на внутренний источник (LSI)
        __HAL_RCC_LSI_ENABLE();

        uint32_t tickstart = HAL_GetTick();
        // Ждём готовности LSI с защитой от зависания (timeout)
        while ((__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) && ((HAL_GetTick() - tickstart) < LSI_TIMEOUT)){}
        if ((HAL_GetTick() - tickstart) >= LSI_TIMEOUT)
        {
            // Таймаут ожидания LSI - вызываем обработчик ошибок
            Error_Handler();
        }
        // Настраиваем RTC для работы с LSI. Прескалеры могут отличаться от конфигурации для LSE.
        hrtc.Init.AsynchPrediv = 127;
        hrtc.Init.SynchPrediv = 249;  // Примерно для LSI ~32000 Гц
        if (HAL_RTC_Init(&hrtc) != HAL_OK)
        {
            Error_Handler();
        }
    }
}

int day_in_mount(int mount, int year)
{
	if (mount == 2)
	{
		if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
			return 29;
	}
	int day_m[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return (day_m[mount - 1]);
}

void RTC_set_date()
{
	HAL_PWR_EnableBkUpAccess();
	RTC_DateTypeDef DateToUpdate = {0};

	if (Date.Date>day_in_mount(Date.Month, Date.Year)) Date.Date = day_in_mount(Date.Month, Date.Year); //  коррекция дня

	DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
	DateToUpdate.Month = Date.Month;
	DateToUpdate.Date = Date.Date;
	DateToUpdate.Year = Date.Year;

	if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	RTC_get_date();
	RTC_get_time();
}

void RTC_set_time()
{
	HAL_PWR_EnableBkUpAccess();
	RTC_TimeTypeDef sTime = {0};

	sTime.Hours = Time.Hours;
	sTime.Minutes = Time.Minutes;
	sTime.Seconds = 0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	RTC_get_date();
	RTC_get_time();
}


extern menuItem Null_Menu;

void RTC_read()
{
	if ((mode_redact == 0) || ((selectedMenuItem->data_in == (void *)&Null_Menu)))
	{
		RTC_get_date();
	}
	if ((mode_redact == 0) || ((selectedMenuItem->data_in == (void *)&Null_Menu)))
	{
		RTC_get_time();
	}
}

void set_time_init(uint8_t hr, uint8_t min, uint8_t sec)
{
	RTC_TimeTypeDef sTime = {0};
	sTime.Hours = hr;
	sTime.Minutes = min;
	sTime.Seconds = sec;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

void RTC_get_time()
{
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
}

void RTC_get_date()
{
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
}






char *u64_to_str(uint64_t value, char *buf, size_t buf_size) {
    if (buf_size < 2) {
        if (buf_size >= 1) buf[0] = '\0';
        return buf;
    }

    // Сперва заполняем временный буфер "задом наперёд"
    char tmp[21];
    int  tmplen = 0;
    if (value == 0) {
        tmp[tmplen++] = '0';
    } else {
        while (value > 0 && tmplen < (int)sizeof(tmp)) {
            tmp[tmplen++] = '0' + (value % 10);
            value /= 10;
        }
    }

    // Проверяем, что хватит места (tmplen цифр + '\0')
    if ((size_t)tmplen + 1 > buf_size) {
        // Не влезает — обрежем старшие цифры
        tmplen = buf_size - 1;
    }

    // Записываем в правильном порядке
    for (int i = 0; i < tmplen; i++) {
        buf[i] = tmp[tmplen - 1 - i];
    }
    buf[tmplen] = '\0';
    return buf;
}

/* Конвертирует дату в количество дней с 2000-01-01 */
static uint32_t RTC_DateToDays(const RTC_DateTypeDef *d) {
    uint32_t y = 2000 + d->Year;
    uint32_t m = d->Month;
    uint32_t day = d->Date;
    // Считаем дни с 2000-01-01 по текущую дату
    // Учтём високосные годы
    uint32_t days = (y - 2000) * 365 
                  + (y - 2000 + 3) / 4   // високосные
                  - (y - 2000 + 99) / 100 // вековые не високосные
                  + (y - 2000 + 399) / 400;
    // дни в месяцах до m (не високосный)
    static const uint8_t mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    for (uint32_t mm = 1; mm < m; mm++) {
        days += mdays[mm];
        if (mm == 2 && ((y % 4 == 0 && y % 100) || y % 400 == 0))
            days += 1;  // февраль високосный
    }
    days += day - 1;
    return days;
}

uint64_t LoadAccumulated(void) {
    uint32_t lo = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_SEC_LOW);
    uint32_t hi = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_SEC_HIGH);
    return ((uint64_t)hi << 32) | lo;
}
void StoreAccumulated(uint64_t v) {
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_SEC_LOW,  (uint32_t)(v & 0xFFFFFFFF));
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_SEC_HIGH, (uint32_t)(v >> 32));
}

void GetRTCTimestamp(uint32_t *outDays, uint32_t *outSecOfDay) {
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);
    uint32_t sec = t.Hours * 3600U + t.Minutes * 60U + t.Seconds;
    *outDays = RTC_DateToDays(&d);
    *outSecOfDay = sec;
}
/**
 * Добавляет в накопленный счётчик то, что прошло
 * от последней точки (checkpoint), и обновляет checkpoint на «сейчас».
 * Вызывать перед засыпанием, при пробуждении или в любой момент,
 * когда нужно «снять мерку».
 */
void Uptime_AccumulateFromCheckpoint(void)
{
    uint32_t nowDays, nowSec;
    GetRTCTimestamp(&nowDays, &nowSec);

    uint32_t oldDays = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_CP_DAYS);
    uint32_t oldSec  = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_CP_SEC);

    // разница в секундах
    int64_t delta = (int64_t)(nowDays - oldDays) * 86400LL + (int64_t)(nowSec - oldSec);
    if (delta < 0) delta = 0;  // на случай «прыжка» времени вниз
    uint64_t acc = LoadAccumulated();
    acc += (uint64_t)delta;
    StoreAccumulated(acc);
	EEPROM.time_work = acc/3600;
	u64_to_str(EEPROM.time_work, EEPROM.version.time_work_char, sizeof(EEPROM.version.time_work_char));
	
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_DAYS, nowDays);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_SEC,  nowSec);
}

void Uptime_ResetCheckpointOnTimeChange(void)
{
    uint32_t days, sec;
    GetRTCTimestamp(&days, &sec);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_DAYS, days);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_SEC,  sec);
}

uint64_t Uptime_GetAccumulatedSeconds(void)
{
    return LoadAccumulated();
}

void Uptime_CheckpointInit(void)
{
    //! Проверить на наличие сохраненного времени в EEPROM
    HAL_PWR_EnableBkUpAccess();
    if (HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INIT_FLAG) != BKP_MAGIC) {
        // первый старт после обесточивания Backup-домена

        // Если в EEPROM уже есть ненулевое время работы — загружаем его,
        // иначе обнуляем накопитель
        if (EEPROM.time_work != 0) {
            StoreAccumulated(EEPROM.time_work);
        } else {
            StoreAccumulated(0);
        }

        // Ставим флаг инициализации
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INIT_FLAG, BKP_MAGIC);

        // Устанавливаем начальную контрольную точку «сейчас»
        uint32_t days, sec;
        GetRTCTimestamp(&days, &sec);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_DAYS, days);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_SEC,  sec);
   }
}

void Uptime_FullReset(void)
{
	HAL_PWR_EnableBkUpAccess();
    uint32_t days, sec;
    
    // 1) Обнуляем накопленный счётчик в Backup-регистрах
    StoreAccumulated(0);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INIT_FLAG, BKP_MAGIC);
    GetRTCTimestamp(&days, &sec);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_DAYS, days);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CP_SEC,  sec);
	StoreAccumulated(0);

    // 4) Очищаем сохранённое в EEPROM время работы
    EEPROM.time_work = 0;
	EEPROM.version.time_work_char[0] = '0';
    EEPROM.version.time_work_char[1] = '\0';

 	HAL_PWR_DisableBkUpAccess();
}