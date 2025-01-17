#include "GSM.h"

#define MAX_LINE_LEN 256
#define CMD_BUFFER_SIZE 1024

extern UART_HandleTypeDef huart4;
// Флаг включённого USB: 0 — не передавать, 0xFF — передавать
static uint8_t usbEnabled = 0x00;

// Флаг «была отправлена команда»
static uint8_t cmdSent = 0;

// Однобайтовый буфер приёма по прерыванию
uint8_t gsmRxChar;

// Временный буфер для текущей строки (до \r\n)
__attribute__((section(".ram2"))) 
static uint8_t gsmLineBuffer[MAX_LINE_LEN];
static uint16_t gsmLineIndex = 0;

// Дополнительный накопительный буфер для всех строк,
// полученных после отправки команды, до прихода OK/ERROR.
__attribute__((section(".ram2"))) 
static char cmdBuffer[CMD_BUFFER_SIZE];
static uint16_t cmdIndex = 0;


__attribute__((section(".ram2"))) 
char tmpBuf[CMD_BUFFER_SIZE];
// Указатель на колбэк-функцию, которую вызовем при завершении команды
static void (*commandDoneCallback)(const char *resp) = NULL;

static ParseState_t parseState = PARSE_IDLE;

/**
 * @brief  Проверяет одну законченную строку (line) на "RDY", "OK", "ERROR" и т.д.
 */
void ParseGsmLine(char *line)
{
    // Если пустая строка — просто ничего не делаем
    if (line == NULL || *line == '\0')
    {
        return;
    }

    // Если пришла "RDY", то всегда передаём по USB (если включен),
    // причём даже если cmdSent = 0
    if (strncmp(line, "RDY", 3) == 0)
    {
        if (usbEnabled == 0xFF)
        {
            CDC_Transmit_FS((uint8_t *)line, strlen(line));
        }
        return;
    }
    if (cmdSent == 0)
    {
        return;
    }

    // Теперь cmdSent = 1, значит мы аккумулируем все строки в cmdBuffer
    // и после OK или ERROR всё отправим.
    // Проверяем, влезает ли строка в наш cmdBuffer (с запасом под \r\n и '\0')
    uint16_t lineLen = strlen(line);
    if (cmdIndex + lineLen + 2 < CMD_BUFFER_SIZE - 1)
    {
        strcpy(&cmdBuffer[cmdIndex], line);
        cmdIndex += lineLen;
        // Добавим "\r\n" в конец, чтобы сохранить структуру построчно
        //cmdBuffer[cmdIndex++] = '\r';
        //cmdBuffer[cmdIndex++] = '\n';
        //cmdBuffer[cmdIndex] = '\0';
    }

    // Проверяем, есть ли в строке "OK" или "ERROR"
    // Важно: "ERROR" может быть и "+CME ERROR: 50" — т.е. substr "ERROR"
    // всё равно найдётся.
    // Аналогичная проверка "ERROR" (ищем подстроку "ERROR")
    if ((strstr(line, "ERROR") != NULL) || (strstr(line, "OK") != NULL) || (strstr(line, "COMMAND NO RESPONSE!") != NULL))
    {

        strcpy(tmpBuf, cmdBuffer);
        if (usbEnabled == 0xFF)
        {
            CDC_Transmit_FS((uint8_t *)tmpBuf, strlen(tmpBuf));
        }
        if (commandDoneCallback != NULL)
        {
            commandDoneCallback(tmpBuf);
        }
        cmdIndex = 0;
        cmdBuffer[0] = '\0';
        cmdSent = 0;
        return;
    }

    
}

/**
 * @brief  Обрабатывает очередной полученный символ (c) от UART4
 */
void ProcessGsmChar(uint8_t c)
{
    if (c == '>')
    {
        if (usbEnabled == 0xFF)
        {
            gsmLineBuffer[gsmLineIndex++] = c;
            gsmLineBuffer[gsmLineIndex] = '\0';
            CDC_Transmit_FS((uint8_t *)gsmLineBuffer, 1);
            cmdIndex = 0;
            cmdBuffer[0] = '\0';
            cmdSent = 0;
            gsmLineIndex = 0;
        }
        return;
    }

    switch (parseState)
    {

    case PARSE_IDLE:
        if (gsmLineIndex < (MAX_LINE_LEN - 1))
        {
            gsmLineBuffer[gsmLineIndex++] = c;
        }
        if (c == '\r')
        {   
            // Видим '\r' -> ждём следующий символ
            parseState = PARSE_HAVE_CR;
            return;
        }
        if (c == '\n')
        {
            // Одиночный '\n': завершаем строку
            if (gsmLineIndex < (MAX_LINE_LEN - 1))
            {
                gsmLineBuffer[gsmLineIndex] = '\0';
            }
            else
            {
                gsmLineBuffer[MAX_LINE_LEN - 1] = '\0';
            }

            // Передаём «готовую строку» на парсинг
            ParseGsmLine((char *)gsmLineBuffer);

            // Сбрасываем индекс
            gsmLineIndex = 0;
            return;
        }
        return;

    case PARSE_HAVE_CR:
        // Иначе это был одиночный '\r', а теперь пошёл обычный символ
        if (gsmLineIndex < (MAX_LINE_LEN - 1))
        {
            gsmLineBuffer[gsmLineIndex++] = c;
        }
        else
        {
            gsmLineIndex = 0; // Переполнение — сброс
        }
        
        if (c == '\n')
        {
            // Окончание строки: "\r\n"
            if (gsmLineIndex < MAX_LINE_LEN)
            {
                gsmLineBuffer[gsmLineIndex] = '\0';
            }
            else
            {
                gsmLineBuffer[MAX_LINE_LEN - 1] = '\0';
            }

            ParseGsmLine((char *)gsmLineBuffer);

            gsmLineIndex = 0;
            parseState = PARSE_IDLE;
            return;
        }
        parseState = PARSE_IDLE;
        return;
    }
}

/**
 * @brief  Колбэк, вызываемый по завершении приёма одного байта UART4
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        // Обрабатываем пришедший байт
        ProcessGsmChar(gsmRxChar);

        // Снова запускаем приём 1 байта
        HAL_UART_Receive_IT(&huart4, &gsmRxChar, 1);
    }
}

void SendSomeCommandAndSetFlag()
{
    cmdSent = 1;
    cmdIndex = 0;
    cmdBuffer[0] = '\0';
}

// Если хотите включить USB:
void EnableUsbCDC_UART(void (*cb)(const char *resp))
{
    usbEnabled = 0xFF;
    commandDoneCallback = cb;
}

// Если хотите выключить USB:
void DisableUsbCDC_UART()
{
    usbEnabled = 0;
}
