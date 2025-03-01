#include "main.h"
#include "stm32l4xx_hal.h"
#include "Settings.h"
#include "FreeRTOS.h"
#include "semphr.h"



void ret_keyboard();
char ScanKeypad(void);
void exit_key_no_press();
void Keyboard(void);
