#include "RTC_data.h"
#include "Display.h"

extern menuItem *selectedMenuItem;
extern int mode_redact;

RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;

#define LSI_TIMEOUT 1000  // ������� � ������������� ��� �������� ���������� LSI

void RTC_Init(void)
{
    // �������������� ��������� RTC ��� �������� ��������� (��������, LSE)
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
        // ���� ������������� �� �������, ��������� �� ���������� �������� (LSI)
        __HAL_RCC_LSI_ENABLE();

        uint32_t tickstart = HAL_GetTick();
        // ��� ���������� LSI � ������� �� ��������� (timeout)
        while ((__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) == RESET) && ((HAL_GetTick() - tickstart) < LSI_TIMEOUT)){}
        if ((HAL_GetTick() - tickstart) >= LSI_TIMEOUT)
        {
            // ������� �������� LSI - �������� ���������� ������
            Error_Handler();
        }
        // ����������� RTC ��� ������ � LSI. ���������� ����� ���������� �� ������������ ��� LSE.
        hrtc.Init.AsynchPrediv = 127;
        hrtc.Init.SynchPrediv = 249;  // �������� ��� LSI ~32000 ��
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

	if (Date.Date>day_in_mount(Date.Month, Date.Year)) Date.Date = day_in_mount(Date.Month, Date.Year); //  ��������� ���

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

void PowerUP_counter()
{
    if (HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_INIT_FLAG) != BKP_MAGIC)
    {
        EEPROM.time_work++;
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_INIT_FLAG, BKP_MAGIC);
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CNT_POWERUP, EEPROM.time_work);
    }
    else
    {
        EEPROM.time_work = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_CNT_POWERUP);
        EEPROM.time_work++;
        HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_CNT_POWERUP, EEPROM.time_work);
    }
    sprintf(EEPROM.version.time_work_char, "%ld", EEPROM.time_work);
}
