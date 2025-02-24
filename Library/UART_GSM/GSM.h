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

#define RX_TIMEOUT_MS   200
#define MAX_LINE_LEN    512
#define CMD_BUFFER_SIZE 512

static void GSM_TimerCallback(TimerHandle_t xTimer);
void Update_Data(void);
int determineRegionAndOperator(void);
