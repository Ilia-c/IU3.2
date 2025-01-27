#include "main.h"


static inline uint8_t IsLeapYear(uint8_t rtcYear);
static uint8_t DaysInMonth(uint8_t month, uint8_t rtcYear);
void RTC_SetAlarm_HoursMinutes(uint8_t hours, uint8_t minutes);
void GPIO_AnalogConfig(void);
void Enter_StandbyMode(uint8_t hours, uint8_t minutes);
uint8_t Check_Wakeup_Reason(void);
void DWT_Init(void);
void None_func();