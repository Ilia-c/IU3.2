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
#include "Driver_rs485.h"

//#include "USB_COMPORT.h"


#define RX_TIMEOUT_MS   300
#define CMD_BUFFER_SIZE 512

extern uint8_t data_read;
void collect_message(char message[]);

void GSM_Init(void);
void Update_Data(void);
int determineRegionAndOperator(void);
void SendSomeCommandAndSetFlag(void);
void GSM_TimerCallback(TimerHandle_t xTimer);
void USB_DEBUG_MESSAGE(const char message[], uint8_t category, uint8_t debugLVL);
void USB_TransmitWithTimeout(uint8_t *buf, uint16_t len);
