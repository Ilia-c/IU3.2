#include "main.h"
#include <string.h>
#include <stdio.h>
#include "Settings.h"

void RTC_Init(void);
void set_time_init(uint8_t hr, uint8_t min, uint8_t sec);
void RTC_get_time();
void RTC_read();
void RTC_set_time();
void RTC_set_date();
void RTC_get_date();
HAL_StatusTypeDef PowerUP_counter(void);
HAL_StatusTypeDef EEPROM_LoadLastTimeWork(void);
HAL_StatusTypeDef EEPROM_ClearBuffer(void);
HAL_StatusTypeDef EEPROM_clear_time_init(void);

