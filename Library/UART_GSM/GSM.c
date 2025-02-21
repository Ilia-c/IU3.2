#include "GSM.h"

extern xSemaphoreHandle UART_PARSER_semaphore;

extern UART_HandleTypeDef huart4;
extern EEPROM_Settings_item EEPROM;

// Однобайтовый буфер приёма по прерыванию
uint8_t gsmRxChar;

// Временный буфер для текущей строки (до \r\n)
__attribute__((section(".ram2")))
static uint8_t gsmLineBuffer[MAX_LINE_LEN];
static uint16_t gsmLineIndex = 0;

// Двойное буферирование для накопления и стабильной передачи по USB
__attribute__((section(".ram2")))
static char buffer1[CMD_BUFFER_SIZE];
__attribute__((section(".ram2")))
char buffer2[CMD_BUFFER_SIZE];

// Указатель на активный буфер для накопления и стабильный для USB
static char *accBuffer = buffer1;  // Активный для записи
char *usbBuffer = buffer2;         // Будет передан по USB
static uint16_t accIndex = 0;

static ParseState_t parseState = PARSE_IDLE;

/**
 * @brief  Анализирует сформированную строку, накапливая данные в накопительном буфере.
 *         Если строка не пуста и не равна только "\r\n", происходит подмена буферов,
 *         вызов семафора и передача по USB, после чего оба буфера очищаются.
 */
void ParseGsmLine(char *line)
{
    // Если строка нулевая – выходим
    if (line == NULL)
    {
        return;
    }
    
    // Если строка состоит только из "\r\n" или пуста, то ничего не делаем
    if ((line[0] == '\r' && line[1] == '\n' && line[2] == '\0') || (line[0] == '\0'))
    {
        return;
    }
    
    // Определяем длину полученной строки
    uint16_t lineLen = 0;
    while (line[lineLen] != '\0')
    {
        lineLen++;
    }
    
    // Добавляем строку в накопительный буфер (accBuffer)
    if (accIndex + lineLen < CMD_BUFFER_SIZE)
    {
        for (uint16_t i = 0; i < lineLen; i++)
        {
            accBuffer[accIndex + i] = line[i];
        }
        accIndex += lineLen;
    }
    else
    {
        // Переполнение буфера – сбрасываем накопление
        accIndex = 0;
        memset(accBuffer, 0, CMD_BUFFER_SIZE);
        return;
    }
    
    // Если в накопительном буфере что-то накопилось, выполняем подмену буферов и передачу по USB
    if (accIndex > 0)
    {
        // Подмена буферов: накопленные данные из accBuffer передаются через usbBuffer,
        // а для накопления назначается другой буфер.
        char *temp = accBuffer;
        accBuffer = usbBuffer;
        usbBuffer = temp;
        
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(UART_PARSER_semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        
        if (EEPROM.USB_mode == 1)
        {
            CDC_Transmit_FS((uint8_t *)usbBuffer, accIndex);
        }
        
        // Отчищаем буфер после передачи
        memset(accBuffer, 0, CMD_BUFFER_SIZE);
        // Сброс накопления для нового приема данных
        accIndex = 0;
        
    }
}

/**
 * @brief  Обрабатывает поступающие символы от UART.
 *         Данные накапливаются во временном буфере gsmLineBuffer до получения последовательности "\r\n".
 *         После формирования строки вызывается ParseGsmLine, после чего временный буфер очищается.
 */
void ProcessGsmChar(uint8_t c)
{
    switch (parseState)
    {
        case PARSE_IDLE:
            if (gsmLineIndex < (MAX_LINE_LEN - 1))
            {
                gsmLineBuffer[gsmLineIndex++] = c;
            }
            if (c == '\r')
            {
                parseState = PARSE_HAVE_CR;
                return;
            }
            return;

        case PARSE_HAVE_CR:
            if (gsmLineIndex < (MAX_LINE_LEN - 1))
            {
                gsmLineBuffer[gsmLineIndex++] = c;
            }
            else
            {
                gsmLineIndex = 0; // Переполнение — сброс
                memset(gsmLineBuffer, 0, MAX_LINE_LEN);
            }
            
            if (c == '\n')
            {
                // Завершаем строку последовательностью "\r\n"
                if (gsmLineIndex < MAX_LINE_LEN)
                {
                    gsmLineBuffer[gsmLineIndex] = '\0';
                }
                else
                {
                    gsmLineBuffer[MAX_LINE_LEN - 1] = '\0';
                }
                // Вызываем обработку сформированной строки
                ParseGsmLine((char *)gsmLineBuffer);
                // Сброс временного буфера для формирования следующей строки
                gsmLineIndex = 0;
                memset(gsmLineBuffer, 0, MAX_LINE_LEN);
                parseState = PARSE_IDLE;
                return;
            }
            parseState = PARSE_IDLE;
            return;
    }
}

/**
 * @brief  Колбэк, вызываемый по завершении приёма одного байта UART4.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        ProcessGsmChar(gsmRxChar);
        HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);
    }
}

void SendSomeCommandAndSetFlag()
{
    accIndex = 0;
    memset(accBuffer, 0, CMD_BUFFER_SIZE);
}

void PARSER(){}
