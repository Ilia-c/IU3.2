/* parser.c */
#include "GSM.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "string.h"
#include "usbd_cdc_if.h"
#include "AT24C02.h"
#include "SD.h"
#include "Settings.h"
// Внешние переменные и объекты
extern UART_HandleTypeDef huart4;
extern xSemaphoreHandle UART_PARSER_semaphore;
extern EEPROM_Settings_item EEPROM;
extern char *parseBuffer;
extern GSM_STATUS_item GSM_data;



int SendCommandAndParse(const char *command, int (*parser)(), uint32_t timeout);
int parse_RDY();
int waitForCPINResponse(uint32_t timeout);
int waitForCSQResponse(uint32_t timeout);
int waitForCEREGResponse(uint32_t timeout);
int waitForCOPSResponse(uint32_t timeout);
int waitForOKResponse(uint32_t timeout);
int waitForGreaterThanResponse(uint32_t timeout);
int waitForHTTPResponse(uint32_t timeout);
int sendSMS(void);
int waitAndParseSiteResponse(uint32_t timeout);
int sendHTTP(void);
int READ_Settings_sendHTTP(void);
int Send_data();
