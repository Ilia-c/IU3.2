#include "RTC_data.h"
#include "Menu_data.h"

extern menuItem *selectedMenuItem;
extern int mode_redact;

RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef Time;
extern RTC_DateTypeDef Date;
extern char time_work_char[15]; // ����� ������ � ���� ������ � �����
extern uint32_t time_work;         // ����� ������ ���������� � ��������

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




static uint32_t get_time_seconds(void)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

	/* ��������� ���� � ������ ���� �� 2000-01-01 */
	uint32_t year = 2000U + (sDate.Year); // RTC ������ ��� ��� offset �� 2000
	uint32_t month = sDate.Month;		  // 1..12
	uint32_t day = sDate.Date;			  // 1..31

	/* ������� ���� �� 2000-01-01 �� year-month-day (�� ������� ������� ����). */
	uint32_t days = 0;
	/* ������� �������� ��� �� ������ ����� */
	for (uint16_t y = 2000; y < year; y++)
	{
		/* ���������� ���, ���� (������� �� 4 � �� �� 100) ��� �� 400 */
		if (((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0))
		{
			days += 366;
		}
		else
		{
			days += 365;
		}
	}
	/* ������ ���� � ������� ��� ������������� ���� */
	static const uint8_t mdays_norm[12] = {
		31, 28, 31, 30, 31, 30,
		31, 31, 30, 31, 30, 31};
	/* ������ ���� � ������� ��� ����������� ���� */
	static const uint8_t mdays_leap[12] = {
		31, 29, 31, 30, 31, 30,
		31, 31, 30, 31, 30, 31};
	/* ���������, �������� �� ������� ��� ���������� */
	uint8_t is_leap = (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)) ? 1 : 0;
	/* �������� ��� � ���������� ������� �������� ���� */
	for (uint8_t m = 1; m < month; m++)
	{
		days += (is_leap ? mdays_leap[m - 1] : mdays_norm[m - 1]);
	}
	/* ���������� ��� �������� ������ (�� ������� ������� ����) */
	days += (day - 1);

	/* ������ ��������� � �������: */
	uint32_t secs = days * 86400U;
	secs += (uint32_t)sTime.Hours * 3600U;
	secs += (uint32_t)sTime.Minutes * 60U;
	secs += (uint32_t)sTime.Seconds;

	return secs;
}


int32_t write_idx = 0;
HAL_StatusTypeDef EEPROM_LoadLastTimeWork(void)
{
    HAL_StatusTypeDef status;
    uint32_t buffer_vals[BUFFER_ENTRIES];
    uint32_t max_val = 0;
    int64_t  max_idx = -1;
	write_idx = 0; 
	time_work = 0; 

    /* 1) ������� ��� uint32_t-�������� �� ������ EEPROM � ��������� ������ */
    for (uint32_t i = 0; i < BUFFER_ENTRIES; i++) {
        uint16_t addr = (uint16_t)(BUFFER_START_ADDR + i * BUFFER_ENTRY_SIZE);
        status = EEPROM_ReadData(addr,
                                 (uint8_t *)&buffer_vals[i],
                                 (uint16_t)BUFFER_ENTRY_SIZE);
        if (status != HAL_OK) {
			snprintf(time_work_char, sizeof(time_work_char), "ERR");
            return status;
        }
        /* ������������, ��� ������� ������ ���������������� ������ */
        if (buffer_vals[i] != 0U) {
            if ((max_idx < 0) || (buffer_vals[i] > max_val)) {
                max_val = buffer_vals[i];
                max_idx = (int64_t)i;
            }
        }
    }

    /* 2) ���� max_idx < 0, ��������: ��� ������ ����� (�������). */
    if (max_idx < 0) {
		snprintf(time_work_char, sizeof(time_work_char), "0�");
		return HAL_OK;
	}
	time_work = max_val;
	write_idx = (max_idx + 1) % (int32_t)BUFFER_ENTRIES;
	int32_t last_idx = (write_idx == 0)
						   ? (int32_t)(BUFFER_ENTRIES - 1)
						   : (write_idx - 1);
	snprintf(time_work_char, sizeof(time_work_char), "%lu�", (unsigned long)time_work/3600);
	return HAL_OK;
}

/*
 * ���� ������� ������
 * PowerUP_counter:
 * 1) �������������� Backup-������� BKP_REG_TIME_INIT ��������� BKP_MAGIC, ���� �� �� ���������������.
 * 2) ��������� ������� ���������� ����� (� ��������) ����� RTC.
 * 3) ��������� ���������� ����� �� BKP_REG_TIME.
 * 4) ��������� delta = now_secs � prev_secs.
 * 5) ���������� delta (uint32_t) � ������������ ����� EEPROM:
 *    � ������ ��� BUFFER_ENTRIES �������� (uint32_t) �� EEPROM �� �������
 *      BUFFER_START_ADDR + i*4;
 *    � ������� ������ ������ � ���������� ��������� (������������ uint32_t);
 *    � � ��������� ������ ( (max_index + 1) % BUFFER_ENTRIES ) ���������� delta.
 * 6) ��������� BKP_REG_TIME = now_secs.
 */
HAL_StatusTypeDef PowerUP_counter(void)
{
	HAL_StatusTypeDef status;
	uint32_t now_secs, prev_secs, delta;
	now_secs = get_time_seconds();
	HAL_PWR_EnableBkUpAccess();
	if (HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_TIME_INIT) != BKP_MAGIC)
	{
		HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_TIME, now_secs);
		HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_TIME_INIT, BKP_MAGIC);
		HAL_PWR_DisableBkUpAccess();
		return HAL_ERROR; // RTC �� ���������������, ������� � �������
	}
	prev_secs = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_TIME);
	delta = now_secs - prev_secs;
	if (delta < 300) return HAL_TIMEOUT;	// ���� ������ ������ 5 �����, �� ���� ���������

	HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_TIME, now_secs);
	if (EEPROM_LoadLastTimeWork() != HAL_OK)
	{
		snprintf(time_work_char, sizeof(time_work_char), "ERR");
		HAL_PWR_DisableBkUpAccess();
		return HAL_ERROR; // ������ ��� �������� ���������� ������� ������
	}
	delta += time_work;
	time_work = delta; 
	
	snprintf(time_work_char, sizeof(time_work_char), "%lu�", (unsigned long)time_work/3600);
	uint16_t write_addr = (uint16_t)(BUFFER_START_ADDR + write_idx * BUFFER_ENTRY_SIZE);
	status = EEPROM_WriteData(write_addr, (uint8_t *)&delta, (uint16_t)BUFFER_ENTRY_SIZE);
	if (status != HAL_OK)
	{
		HAL_PWR_DisableBkUpAccess();
		return HAL_ERROR;
	}
	HAL_PWR_DisableBkUpAccess();
	return HAL_OK;
}

// ��������� ������������� ������ EEPROM
HAL_StatusTypeDef EEPROM_ClearBuffer(void)
{
    HAL_StatusTypeDef status;
    /* ������ ����� ������ BUFFER_SIZE_BYTES */
    uint8_t zeros[BUFFER_SIZE_BYTES] = {0};
    status = EEPROM_WriteData(
        BUFFER_START_ADDR,
        zeros,
        (uint16_t)BUFFER_SIZE_BYTES
    );

    return status;
}

HAL_StatusTypeDef EEPROM_clear_time_init(void)
{
	HAL_PWR_EnableBkUpAccess();
	HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_TIME_INIT, 0);
	HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_TIME, 0);
	HAL_PWR_DisableBkUpAccess();
}


