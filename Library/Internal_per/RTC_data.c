#include "RTC_data.h"
#include "Display.h"

extern menuItem *selectedMenuItem;
extern int mode_redact;

RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;

extern char c_Time[]; // из settings
extern char c_Date[]; // из settings

void RTC_Init(void)
{
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
		Error_Handler();
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

	RTC_DateTypeDef DateToUpdate = {0};
	uint8_t data_d = ((uint8_t)c_Date[0] - '0') * 10 + ((uint8_t)c_Date[1] - '0');
	uint8_t date_m = ((uint8_t)c_Date[3] - '0') * 10 + ((uint8_t)c_Date[4] - '0');
	uint8_t date_y = ((uint8_t)c_Date[6] - '0') * 10 + ((uint8_t)c_Date[7] - '0');

	if (date_m>12) date_m  = date_m - 12 * (uint8_t)(date_m / 12); // коррекция месяца
	if (data_d>day_in_mount(date_m, date_y)) data_d = day_in_mount(date_m, date_y); //  коррекция дня

	DateToUpdate.WeekDay = 0;
	DateToUpdate.Month = date_m;
	DateToUpdate.Date = data_d;
	DateToUpdate.Year = date_y;

	if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	RTC_get_date();
	RTC_get_time();
}

void RTC_set_time()
{
	RTC_TimeTypeDef sTime = {0};

	uint8_t time_h = ((uint8_t)c_Time[0] - '0') * 10 + ((uint8_t)c_Time[1] - '0');
	uint8_t time_m = ((uint8_t)c_Time[3] - '0') * 10 + ((uint8_t)c_Time[4] - '0');

	if (time_h >= 24)
		time_h = time_h - 24 * (uint8_t)(time_h / 24);
	if (time_m >= 60)
		time_m -= 60;

	sTime.Hours = time_h;
	sTime.Minutes = time_m;
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

/*
	char time_h[2];
	char time_m[2];
	snprintf(time_h, 4, "%02d", Time.Hours);
	snprintf(time_m, 4, "%02d", Time.Minutes);
	for (int i = 0; i < 2; i++)
	{
		c_Time[i] = time_h[i];
	}
	c_Time[2] = ':';
	for (int i = 0; i < 2; i++)
	{
		c_Time[i + 3] = time_m[i];
	}
	*/
}

void RTC_get_date()
{
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);
/*
	char date_d[2];
	char date_m[2];
	char date_y[2];
	snprintf(date_d, 4, "%02d", Date.Date);
	snprintf(date_m, 4, "%02d", Date.Month);
	snprintf(date_y, 4, "%02d", Date.Year);
	for (int i = 0; i < 2; i++)
	{
		c_Date[i] = date_d[i];
	}
	c_Date[2] = '.';
	for (int i = 0; i < 2; i++)
	{
		c_Date[i + 3] = date_m[i];
	}
	c_Date[5] = '.';
	for (int i = 0; i < 2; i++)
	{
		c_Date[i + 6] = date_y[i];
	}
	*/
}
