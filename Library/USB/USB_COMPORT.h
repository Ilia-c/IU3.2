#ifndef USB_COMPORT_H
#define USB_COMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Settings.h"
#include "Parser.h"
#include "cmsis_os.h"   // для TickType_t и vTaskDelay
#include <string.h>
#include <stdio.h>
#include <stdint.h>


void USB_Send_Status_Report(void);
void USB_COM(void);
void AT_SEND(void);
void DEBUG_USB(void);

#ifdef __cplusplus
}
#endif

#endif 
