#include "main.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "Settings.h"
#include "semphr.h"
#include "GSM.h"

extern UART_HandleTypeDef huart4;
extern GSM_STATUS_item GSM_data;
extern xSemaphoreHandle UART_PARSER_semaphore;
extern char *usbBuffer;

int CPIN(uint32_t Timeout);
int parse_CPIN();
int parse_RDY();
int parse_CTZV();
int parse_QNITZ();
int parse_CREG();
int ProcessATResponse();


typedef struct {
    const char command[20];          // Идентификатор команды (например, "ATI" или "+CSQ")
    int (*parser)(char *resp); // Функция для разбора ответа этой команды
} ATCommandEntry;

ATCommandEntry atCommandList[] = {
    {"+CREG:",   parse_CREG},  // +CREG: 1 регистрация в сети удалась
    {"+QNITZ:",  parse_QNITZ}, // +QNITZ:2025/2/21,20:22:17+12,0 время
    {"+CTZV:",   parse_CTZV},  // +CTZV:2025/2/21,20:22:17,+12 дата
    {"RDY",      parse_RDY}    // Готовность модуля к работе
};


int parse_RDY() {

    return -1;
}

int parse_CTZV() {

    return 0;
}

int parse_QNITZ() {

    return -1;
}

int parse_CREG() {

    return -1;
}

int CPIN(uint32_t Timeout){
    // Отправка комманды
    char command[] = "AT+CPIN?\r";
    HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    if (xSemaphoreTake(UART_PARSER_semaphore, 50000) == pdFALSE)
    {
        // Установить ошибку
        return -1;
    }
    if (parse_CPIN() == 1)
    {
        GSM_data.GSM_Signal_Level = -1;
        GSM_data.Status |= SIM_PRESENT; 
    }
    else{
        GSM_data.Status = 0x0000;
        ERRCODE.STATUS |= STATUS_GSM_NO_SIM;
        ERRCODE.STATUS &= ~(STATUS_GSM_REG_ERROR |
            STATUS_NBIOT_REG_ERROR |
            STATUS_GSM_NET_ERROR |
            STATUS_UART_SMS_SEND_ERROR |
            STATUS_UART_SERVER_COMM_ERROR |
            STATUS_UART_SERVER_UNAVAILABLE |
            STATUS_UART_NO_RESPONSE |
            STATUS_UART_WRONG_PASSWORD);
    }
    return 0;
}

int parse_CPIN() {
    if (usbBuffer == NULL) {
        return 0;
    }
    if (strstr(usbBuffer, "+CPIN:READY") != NULL) {
        return 1;
    }
    return 0;
}


// Только для поиска конца регистрации и сохранения даты и времени
#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))
// command_delay == 1 - следующий ответ это ответ на запрос
int ProcessATResponse() {
    for (size_t i = 0; i < NUM_AT_COMMANDS; i++) {
        size_t len = strlen(atCommandList[i].command);
        if (strncmp(usbBuffer, atCommandList[i].command, len) == 0) {
            return atCommandList[i].parser(usbBuffer);
        }
    }
    return -1;
}

