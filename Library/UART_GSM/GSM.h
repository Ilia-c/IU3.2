#include "main.h"
#include "string.h"     // для strlen, strstr, strcpy и т.д.
#include "usbd_cdc_if.h" // для CDC_Transmit_FS (если есть)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include <string.h>
#include <stdio.h>
#include "Settings.h"

#define RX_TIMEOUT_MS   300
#define CMD_BUFFER_SIZE 512

extern uint8_t data_read;

void Update_Data(void);
int determineRegionAndOperator(void);
void SendSomeCommandAndSetFlag(void);
static void GSM_TimerCallback(TimerHandle_t xTimer);
