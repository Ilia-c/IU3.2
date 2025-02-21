#include "main.h"
#include "string.h"     // для strlen, strstr, strcpy и т.д.
#include "usbd_cdc_if.h" // для CDC_Transmit_FS (если есть)
#define MAX_LINE_LEN    512
#define CMD_BUFFER_SIZE 512
// Машина состояний для CR/LF
typedef enum {
    PARSE_IDLE,   // Обычный режим (нет ожидания LF)
    PARSE_HAVE_CR // Видели '\r' и ждём следующий символ
} ParseState_t;

void ParseGsmLine(char *line);
void ProcessGsmChar(uint8_t c);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void SendSomeCommandAndSetFlag();
void EnableUsbCDC_UART();
void DisableUsbCDC_UART();