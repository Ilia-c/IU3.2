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
#include "GSM.h"
#include "Parser.h"
#include "MQTT.h"

extern xSemaphoreHandle Display_semaphore;
extern osThreadId_t  UART_PARSER_taskHandle;

void M2M_disable();
void M2M_enable();
void M2M_check_POWER();
void M2M_init();
void M2M_text_status_Update();
void SMS_send();
void HTTP_read();
void HTTP_send();
HAL_StatusTypeDef _MQTT_send();
HAL_StatusTypeDef HTTP_or_SMS_send();
void _sendSMS();