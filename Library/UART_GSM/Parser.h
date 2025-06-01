/* parser.c */
#include "GSM.h"
#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "string.h"
#include "usbd_cdc_if.h"
#include "AT24C02.h"
#include "Data_collect.h"
#include "Settings.h"
// Внешние переменные и объекты
extern UART_HandleTypeDef huart4;
extern xSemaphoreHandle UART_PARSER_semaphore;
extern EEPROM_Settings_item EEPROM;
extern char *parseBuffer;
extern GSM_STATUS_item GSM_data;



HAL_StatusTypeDef SendCommandAndParse(const char *command, int (*parser)(), uint32_t timeout);
HAL_StatusTypeDef parse_RDY();
HAL_StatusTypeDef waitForCPINResponse(uint32_t timeout);
HAL_StatusTypeDef waitForCSQResponse(uint32_t timeout);
HAL_StatusTypeDef waitForCEREGResponse(uint32_t timeout);
HAL_StatusTypeDef waitForCOPSResponse(uint32_t timeout);
HAL_StatusTypeDef waitForOKResponse(uint32_t timeout);
HAL_StatusTypeDef waitForGreaterThanResponse(uint32_t timeout);
HAL_StatusTypeDef waitForHTTPResponse(uint32_t timeout);
HAL_StatusTypeDef sendSMS(void);
HAL_StatusTypeDef waitAndParseSiteResponse(uint32_t timeout);
HAL_StatusTypeDef sendHTTP(void);
HAL_StatusTypeDef READ_Settings_sendHTTP(void);
HAL_StatusTypeDef Send_data();
