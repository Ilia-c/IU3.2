/* parser.c */
#include "GSM.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "string.h"
#include "usbd_cdc_if.h"
#include "AT24C02.h"
// Внешние переменные и объекты
extern UART_HandleTypeDef huart4;
extern xSemaphoreHandle UART_PARSER_semaphore;
extern EEPROM_Settings_item EEPROM;
extern char *parseBuffer;
extern GSM_STATUS_item GSM_data;



int SendCommandAndParse(const char *command, int (*parser)(), uint32_t timeout);
int parse_RDY();
int parse_CPIN();
int parse_CSQ(void);
int parse_CEREG(void);
int parse_COPS(void);
int parse_ERROR_OK(void);
int waitForOKResponse();
int waitForGreaterThanResponse(void);
int waitForHTTPResponse();
void ND();
int sendSMS(void);
int waitAndParseSiteResponse(void);
int sendHTTP(void);
int READ_Settings_sendHTTP(void);
int Send_data();
