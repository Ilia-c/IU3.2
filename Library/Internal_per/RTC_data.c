#include "RTC_data.h"

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef Time = {0};
RTC_DateTypeDef Date = {0};

extern char c_Time[]; // из settings
extern char c_Date[]; // из settings

void RTC_Init(void)
{

	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

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


void set_time(uint8_t hr, uint8_t min, uint8_t sec)
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

	char time_h[2];
	char time_m[2];
	snprintf(time_h, 3, "%02d", Time.Hours);
    snprintf(time_m, 3, "%02d", Time.Minutes);
	for (int i = 0; i<2; i++){
		c_Time[i] = time_h[i];
	}
	c_Time[2] = ':';
	for (int i = 0; i<2; i++){
		c_Time[i+3] = time_m[i];
	}
	
	char date_d[2];
	char date_m[2];
	char date_y[2];
	snprintf(date_d, 3, "%02d", Date.Date);
    snprintf(date_m, 3, "%02d", Date.Month);
	snprintf(date_y, 3, "%02d", Date.Year);
	for (int i = 0; i<2; i++){
		c_Date[i] = date_d[i];
	}
	c_Date[2] = '.';
	for (int i = 0; i<2; i++){
		c_Date[i+3] = date_m[i];
	}
	c_Date[5] = '.';
	for (int i = 0; i<2; i++){
		c_Date[i+6] = date_y[i];
	}	
	
}
