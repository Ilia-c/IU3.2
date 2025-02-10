#include "main.h"
// Определяем коды сброса
#define RESET_CODE_BOR      0x01  // Сброс из-за понижения напряжения (Brown-Out Reset)
#define RESET_CODE_PINRST   0x02  // Внешний сброс (сброс по пину NRST)
#define RESET_CODE_SFTRST   0x03  // Программный сброс (Software Reset)
#define RESET_CODE_IWDG     0x04  // Сброс от независимого сторожевого таймера (IWDG Reset)
#define RESET_CODE_WWDG     0x05  // Сброс от оконного сторожевого таймера (WWDG Reset)
#define RESET_CODE_LPWR     0x06  // Сброс из-за условий низкого напряжения (Low-Power Reset)
#define RESET_CODE_UNKNOWN  0xFF  // Неизвестная причина сброса


uint8_t GetResetCode(void);
static inline uint8_t IsLeapYear(uint8_t rtcYear);
static uint8_t DaysInMonth(uint8_t month, uint8_t rtcYear);
void RTC_SetAlarm_HoursMinutes(uint8_t hours, uint8_t minutes);
void GPIO_AnalogConfig(void);
void Enter_StandbyMode(uint8_t hours, uint8_t minutes);
uint8_t Check_Wakeup_Reason(void);
void DWT_Init(void);
void None_func();