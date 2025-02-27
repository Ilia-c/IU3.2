/* parser.c */
#include "Parser.h"

#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))

/**
 * @brief Отправляет AT-команду, ожидает ответа и анализирует его.
 * @param command AT-команда для отправки.
 * @param timeout Таймаут ожидания ответа в миллисекундах.
 * @param parser Функция для разбора ответа.
 * @return Результат анализа ответа.
 */
int SendCommandAndParse(const char *command, int (*parser)(), uint32_t timeout)
{
    SendSomeCommandAndSetFlag();
    HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), 1000);
    if (xSemaphoreTake(UART_PARSER_semaphore, 0) == pdTRUE){}
    if (xSemaphoreTake(UART_PARSER_semaphore, timeout) == pdFALSE)
    {
        return -1; 
    }
    char *echo = strstr(parseBuffer, command);
    if (echo != NULL)
    {
        echo += strlen(command);
        while (*echo == '\r' || *echo == '\n' || *echo == ' ')
        {
            echo++;
        }
        memmove(parseBuffer, echo, strlen(echo) + 1);
        if (parser() == 1){
            return 1;
        }
        else{
            return 0;
        }
    }
    return -1;
}

void ND(){
    osDelay(100);
}

int parse_CPIN()
{
    if (strstr(parseBuffer, "+CPIN:READY") != NULL){
        GSM_data.Status |= SIM_PRESENT;
        return 1;
    }
    GSM_data.Status &= ~SIM_PRESENT;
    return 0;
}

int parse_CSQ(void)
{
    int16_t rssi, ber;
    // Ищем в буфере подстроку "+CSQ:"
    char *p = strstr(parseBuffer, "+CSQ:");
    if (p != NULL)
    {
        // Смещаем указатель за "+CSQ:" и пропускаем возможные пробелы
        p += strlen("+CSQ:");
        while (*p == ' ' || *p == '\t')
        {
            p++;
        }
        
        // Пытаемся разобрать два числовых значения (16-битных), разделённых запятой
        if (sscanf(p, "%hd,%hd", &rssi, &ber) == 2)
        {
            //rssi - качество сигнала 0-31, 99
            //ber - уровень ошибок 0-7, 99
            GSM_data.GSM_Signal_Level = rssi;
            GSM_data.GSM_Signal_Errors = ber;
            
            return 1;
        }
    }
    return -1;
}
int parse_CEREG(void)
{
    int n, stat, act;
    char tac[16] = {0};
    char ci[16] = {0};
    char *p = strstr(parseBuffer, "+CEREG:");
    if (p == NULL)
    {
        // Подстрока не найдена
        return -1;
    }
    
    p += strlen("+CEREG:");
    
    // Пытаемся разобрать строку в формате:
    // <n>, <stat>, "<tac>", "<ci>", <act>
    // Пример ответа: +CEREG: 0, 1,"02b3","cea80000",0
    if (sscanf(p, " %d , %d , \"%15[^\"]\" , \"%15[^\"]\" , %d", &n, &stat, tac, ci, &act) == 5)
    {
        //stat - X и act - Y
        if (stat == 1 && act == 0)
        {
            GSM_data.Modem_mode = MODEM_STATUS[0];
            return 1;
        }
        if (stat == 1 && act == 9)
        {
            GSM_data.Modem_mode = MODEM_STATUS[1];
            return 1;
        }
        GSM_data.Modem_mode = MODEM_STATUS[2];
        return 0;
    }
    return -1;
}
int parse_COPS(void)
{
    int mode, format, act;
    char oper[32] = {0};  // Буфер для оператора (достаточного размера для хранения строки оператора)
    
    // Ищем в parseBuffer подстроку "+COPS:"
    char *p = strstr(parseBuffer, "+COPS:");
    if (p == NULL)
    {
        // Если подстрока не найдена, возвращаем -1
        return -1;
    }
    
    // Смещаем указатель за "+COPS:"
    p += strlen("+COPS:");
    
    // Пытаемся разобрать строку в формате: mode, format, "oper", act
    // Пример ответа: +COPS: 0,2,"25001", 0
    if (sscanf(p, " %d , %d , \"%31[^\"]\" , %d", &mode, &format, oper, &act) == 4)
    {
        GSM_data.Operator_code = atoi(oper);
        return 1;
    }
    
    return -1;
}
int parse_ERROR_OK(void)
{

    // Если обнаружен код ERROR, возвращаем 0
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0;
    }
    // Если обнаружен код OK, возвращаем 1
    else if (strstr(parseBuffer, "OK") != NULL)
    {
        return 1;
    }
    // Если ни ERROR, ни OK не найдены, возвращаем -1 (неопределённое состояние)
    return -1;
}

int waitForOKResponse()
{
    uint32_t timeout = 40000;
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    if (strstr(parseBuffer, "OK") != NULL)
    {
        return 1; // Получен ответ OK
    }
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0; // Получен ответ ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (strstr(parseBuffer, "OK") != NULL)
            {
                return 1; // Получен ответ OK
            }
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return 0; // Получен ответ ERROR
            }
        }
    }
    return -1; // Таймаут: ни OK, ни ERROR не обнаружены
}

int waitForGreaterThanResponse(void)
{
    uint32_t timeout = 40000; // Таймаут в мс
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    // Если в буфере обнаружен символ '>', считаем, что приглашение получено
    if (strstr(parseBuffer, ">") != NULL)
    {
        return 1; // Получен символ '>'
    }
    // Если обнаружен ERROR, возвращаем ошибку
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0; // Получен ответ ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // Если в буфере обнаружен символ '>', считаем, что приглашение получено
            if (strstr(parseBuffer, ">") != NULL)
            {
                return 1; // Получен символ '>'
            }
            // Если обнаружен ERROR, возвращаем ошибку
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return 0; // Получен ответ ERROR
            }
        }
    }
    return -1; // Таймаут: символ '>' не обнаружен
}

int waitForHTTPResponse()
{
    uint32_t timeout = 60000;
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    int m, s, d;
    // Пытаемся разобрать строку. Если в ответе присутствует лишний символ, например,
    // точка в конце "162.", sscanf успешно вернёт 3, так как цифры будут разобраны корректно.
    if (sscanf(parseBuffer, "%d %d %d", &m, &s, &d) == 3)
    {
        if (s == 200)
            return 1;
        return 0; // Успешно получен и разобран ответ
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // Пытаемся разобрать строку. Если в ответе присутствует лишний символ, например,
            // точка в конце "162.", sscanf успешно вернёт 3, так как цифры будут разобраны корректно.
            if (sscanf(parseBuffer, "%d %d %d", &m, &s, &d) == 3)
            {
                if (s == 200)
                    return 1;
                return 0; // Успешно получен и разобран ответ
            }
        }
    }
    return -1; // Таймаут: ответ не получен или не соответствует формату
}
