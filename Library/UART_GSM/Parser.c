/* parser.c */
#include "Parser.h"

#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))
static char command[1024] __attribute__((section(".ram2"))) =  {0};
char tempBuf[256]__attribute__((section(".ram2"))) = {0}; 

HAL_StatusTypeDef SendCommandAndParse(const char *command_in, int (*parser)(), uint32_t timeout)
{
    memset(tempBuf, 0, sizeof(tempBuf));
    snprintf(tempBuf, sizeof(tempBuf), "[DEBUG AT] Отправленная команда: %s", command_in);
    //CDC_Transmit_FS((uint8_t *)tempBuf, strlen(tempBuf));
    USB_DEBUG_MESSAGE(tempBuf, DEBUG_GSM, DEBUG_LEVL_4);

    // Сброс текущего буфера перед отправкой
    SendSomeCommandAndSetFlag();  
    // Отправка команды
    GSM_data.Status |= DATA_READ;
    osDelay(10);
    HAL_UART_Transmit(&huart4, (uint8_t *)command_in, strlen(command_in), 1000);
    HAL_StatusTypeDef result = parser(timeout);
    GSM_data.Status &= ~DATA_READ;
    return result;
}


HAL_StatusTypeDef waitForCPINResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (strstr(parseBuffer, "+CPIN:READY") != NULL)
            {
                GSM_data.Status |= SIM_PRESENT;
                ERRCODE.STATUS &= ~STATUS_GSM_NO_SIM;
                return HAL_OK;  // SIM карта присутствует и готова
            }
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                GSM_data.Status &= ~SIM_PRESENT;
                ERRCODE.STATUS |= STATUS_GSM_NO_SIM;
                return HAL_ERROR;
            }
        }
    }
    GSM_data.Status &= ~SIM_PRESENT;
    ERRCODE.STATUS |= STATUS_GSM_NO_SIM;
    return HAL_TIMEOUT;  // Таймаут
}


HAL_StatusTypeDef waitForCSQResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // Ищем в parseBuffer подстроку "+CSQ:"
            char *p = strstr(parseBuffer, "+CSQ:");
            if (p != NULL)
            {
                int16_t rssi, ber;
                p += strlen("+CSQ:");
                while (*p == ' ' || *p == '\t')
                {
                    p++;
                }
                if (sscanf(p, "%hd,%hd", &rssi, &ber) == 2)
                {
                    GSM_data.GSM_Signal_Level = rssi;
                    GSM_data.GSM_Signal_Errors = ber;
                    if (GSM_data.GSM_Signal_Level <= 5)
                    {
                        ERRCODE.STATUS |= STATUS_GSM_SIGNEL_ERROR;
                    }
                    else
                    {
                        ERRCODE.STATUS &= ~STATUS_GSM_SIGNEL_ERROR;
                    }

                    return HAL_OK; // Успешно получили уровень сигнала
                }
            }
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                return HAL_ERROR;
            }
        }
    }
    return HAL_TIMEOUT;
}


HAL_StatusTypeDef waitForCEREGResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    // Можно заранее сбрасывать/инициализировать GSM_data.Modem_mode
    // Но если это нежелательно, оставьте как есть

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // Ждем, пока парсер сообщит о свежих данных в parseBuffer
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // Ищем нужную подстроку
            char *p = strstr(parseBuffer, "+CEREG:");
            if (p != NULL)
            {
                p += strlen("+CEREG:");

                int n, stat, act;
                char tac[16] = {0};
                char ci[16]  = {0};

                // Пытаемся считать поля:
                // +CEREG: <n>, <stat>, "<tac>", "<ci>", <act>
                // Пример: +CEREG: 0,1,"02b3","cea80000",0
                if (sscanf(p, " %d , %d , \"%15[^\"]\" , \"%15[^\"]\" , %d", 
                           &n, &stat, tac, ci, &act) == 5)
                {
                    if (stat == 1 && act == 0)
                    {
                        GSM_data.Modem_mode = MODEM_STATUS[0];
                        return HAL_OK; 
                    }
                    if (stat == 1 && act == 9)
                    {
                        GSM_data.Modem_mode = MODEM_STATUS[1];
                        return HAL_OK; 
                    }
                    GSM_data.Modem_mode = MODEM_STATUS[2];
                    return HAL_BUSY;
                }
            }
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                return HAL_ERROR;
            }
        }
    }
    return HAL_TIMEOUT;
}



HAL_StatusTypeDef waitForCOPSResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // Ждём, когда придут новые данные (семафор от парсера).
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // Ищем в буфере нужную подстроку
            char *p = strstr(parseBuffer, "+COPS:");
            if (p != NULL)
            {
                p += strlen("+COPS:");

                int mode, format, act;
                char oper[32] = {0}; 
                if (sscanf(p, " %d , %d , \"%31[^\"]\" , %d", &mode, &format, oper, &act) == 4)
                {
                    GSM_data.Operator_code = atoi(oper);
                    return HAL_OK; 
                }
            }
            
            // Дополнительно можем отлавливать "ERROR" / "+CME ERROR"
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                // Возвращаем 0 или иной код в случае ошибки
                return HAL_ERROR;
            }
        }
    }
    return HAL_TIMEOUT;
}


HAL_StatusTypeDef waitForOKResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            if (strstr(parseBuffer, "OK") != NULL)
            {
                USB_DEBUG_MESSAGE("[DEBUG AT] Ответ OK", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_OK; // Получен ответ OK
            }
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                USB_DEBUG_MESSAGE("[ERROR AT] Ответ ERROR", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_ERROR; // Получен ответ ERROR
            }
        }
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] Таймаут ожидания ОК", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_TIMEOUT; // Таймаут: ни OK, ни ERROR не обнаружены
}

HAL_StatusTypeDef waitForGreaterThanResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    // Если в буфере обнаружен символ '>', считаем, что приглашение получено
    if (strstr(parseBuffer, ">") != NULL)
    {
        return HAL_OK; // Получен символ '>'
    }
    // Если обнаружен ERROR, возвращаем ошибку
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return HAL_ERROR; // Получен ответ ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // Если в буфере обнаружен символ '>', считаем, что приглашение получено
            if (strstr(parseBuffer, ">") != NULL)
            {
                return HAL_OK; // Получен символ '>'
            }
            // Если обнаружен ERROR, возвращаем ошибку
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return HAL_ERROR; // Получен ответ ERROR
            }
        }
    }

    return HAL_TIMEOUT; // Таймаут: символ '>' не обнаружен
}

HAL_StatusTypeDef waitForSMSResponse(uint32_t timeout)
{
    SendSomeCommandAndSetFlag();  
    GSM_data.Status |= DATA_READ;
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (strstr(parseBuffer, "+CMGS:") != NULL)
            {
                GSM_data.Status &= ~DATA_READ;
                return HAL_OK;  
            }
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                GSM_data.Status &= ~DATA_READ;
                return HAL_ERROR;
            }
        }
    }
    GSM_data.Status &= ~DATA_READ;
    return HAL_TIMEOUT;  // Таймаут
}


HAL_StatusTypeDef waitForHTTPResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    int32_t m, s, d;
    ERRCODE.STATUS &= ~STATUS_HTTP_WRONG_PASSWORD_ERROR;
    // Пытаемся разобрать строку. Если в ответе присутствует лишний символ, например,
    // точка в конце "162.", sscanf успешно вернёт 3, так как цифры будут разобраны корректно.
    if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
    {
        if (s == 200)
            return HAL_OK;
        return HAL_BUSY; // Успешно получен и разобран ответ
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
            {
                if (s == 200){
                     USB_DEBUG_MESSAGE("[DEBUG AT] Ответ 200", DEBUG_GSM, DEBUG_LEVL_4);
                    return HAL_OK;
                }
                USB_DEBUG_MESSAGE("[ERROR AT] Ответ не 200", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_BUSY; // Успешно получен и разобран ответ
            }
            if (strstr(parseBuffer, "error") != NULL)
            {
                break;
            }
        }
    }
    //USB_DEBUG_MESSAGE(parseBuffer, DEBUG_GSM, DEBUG_LEVL_4);
    //CDC_Transmit_FS(parseBuffer, sizeof(parseBuffer));
    return HAL_TIMEOUT; // Таймаут: ответ не получен или не соответствует формату
}


#define MAX_SMS_ATTEMPTS 1

HAL_StatusTypeDef sendSMS(void)
{
    int attempt;
    uint8_t smsSent = 0; // Флаг успешной отправки SMS
    if (strlen(save_data) + 3 < sizeof(save_data) )
    {
        strcat(save_data, "\x1A\r");
    }
    else{
        return HAL_ERROR;
    }
    
    SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
    SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 60000);
    SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);

    // Команда для выбора текстового режима
    if (SendCommandAndParse("AT+CMGF=1\r", waitForOKResponse, 1000) != HAL_OK)
    {
        return HAL_ERROR; // Ошибка при установке текстового режима
    }

    // Команда для выбора кодировки символов
    if (SendCommandAndParse("AT+CSCS=\"GSM\"\r", waitForOKResponse, 1000) != HAL_OK)
    {
        osDelay(1000);
        return HAL_ERROR; 
    }

    // Команда для начала отправки SMS
    // "AT+CMGS=\"+79150305966\"\r"
    memset(command, 0, sizeof(command));
    sprintf(command, "AT+CMGS=\"%s\"\r", EEPROM.Phone);
    // strcat(command, save_data);
    if (SendCommandAndParse(command, waitForGreaterThanResponse, 4000) != HAL_OK)
    {
        osDelay(1000);
        return HAL_ERROR; 
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] Отправлено сообщение по UART:", DEBUG_GSM, DEBUG_LEVL_3);
    USB_DEBUG_MESSAGE(save_data, DEBUG_GSM, DEBUG_LEVL_4);
    HAL_UART_Transmit(&huart4, save_data, strlen(save_data), 1000);
    
    if (waitForSMSResponse(90000) == HAL_OK){
        USB_DEBUG_MESSAGE("[DEBUG AT] SMS отправлено успешно", DEBUG_GSM, DEBUG_LEVL_4);
    } else {
        USB_DEBUG_MESSAGE("[ERROR AT] Ошибка при отправке SMS", DEBUG_GSM, DEBUG_LEVL_4);
        ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
        return HAL_ERROR; // Ошибка при ожидании ответа на SMS
    } 
    return HAL_OK; 
}

#define MAX_HTTP_ATTEMPTS 3

HAL_StatusTypeDef sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // Флаг успешной отправки HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?data=";
    if ( (strlen(send) + strlen(save_data) + 3) < sizeof(send) )
    {
        strcat(send, save_data);
        strcat(send, "\"\r");
    }
    else{
        return HAL_ERROR;
    }

    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 1000) != HAL_OK) {
            goto http_error_1;
        }
        osDelay(100);
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 2000) != HAL_OK) {
            goto http_error_1;
        }
        // Отправка HTTP запроса (send - строка с корректно сформированным URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != HAL_OK) {
            goto http_error_1;
        }
        osDelay(100);
        // Выполнение HTTPACTION, проверка ответа
        int res = SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 60000);
        if (ERRCODE.STATUS & STATUS_HTTP_WRONG_PASSWORD_ERROR) {
            // Неверный пароль код 403
            SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);
            break;
        }
        if (res != HAL_OK) {
            goto http_error_1;
        }

        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 2000);
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);

        if (ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR) break;
        // Если данные так и не получены, считаем попытку неуспешной
        if (readResult != HAL_OK){
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
            goto http_error_1;
        }
        // Если до сюда дошли, значит все команды выполнены успешно
        httpSent = 1;
        break;

http_error_1:
    SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);
    osDelay(500);
    SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 60000);
    osDelay(500);
    SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);
    osDelay(500);
    SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000);
    osDelay(500);
    SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
    osDelay(500);
    SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
    osDelay(5000); // Задержка 5 секунд перед следующей попыткой
    }

    // Если ни одна из попыток не увенчалась успехом, выставляем флаг ошибки
    if (!httpSent) {
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef READ_Settings_sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // Флаг успешной отправки HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?request=";
    
    // Проверяем, поместится ли дополнение: длина send + длина save_data + 2 символа (для " и \r) + завершающий \0
    if ( (strlen(send) + strlen(save_data) + 3) < sizeof(send) )
    {
        strcat(send, save_data);
        strcat(send, "\"\r");
    }
    else{
        return HAL_ERROR;
    }
    


    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 2000) != HAL_OK) {
            goto http_error_2;
        }
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ инициализирован", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 2000) != HAL_OK) {
            goto http_error_2;
        }
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ профиль сессии установлен успешно", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        // Отправка HTTP запроса (send - строка с корректно сформированным URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != HAL_OK) {
            goto http_error_2;
        }
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ отправлен", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        // Выполнение HTTPACTION, проверка ответа
        int res = SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 60000);
        if (res != HAL_OK) {
            goto http_error_2;
        }
        osDelay(1000);
        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 2000);
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);

        if (ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR){
            httpSent = 1;
            break;
        }
        // Если данные так и не получены, считаем попытку неуспешной
        if (readResult != HAL_OK) {
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        }
        
        // Если до сюда дошли, значит все команды выполнены успешно
        httpSent = 1;
        break;

http_error_2:
        USB_DEBUG_MESSAGE("[ERROR AT] Ошибка READ запроса", DEBUG_GSM, DEBUG_LEVL_3);
        // В случае ошибки выполняем «очистку» состояния:
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);
        SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 60000);
        SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000); 
        SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
        SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
        osDelay(2000);  // Задержка2 секунды перед следующей попыткой
    }

    // Если ни одна из попыток не увенчалась успехом, выставляем флаг ошибки
    if (!httpSent) {
        USB_DEBUG_MESSAGE("[ERROR AT] Выставлена - Ошибка связи с сервером", DEBUG_GSM, DEBUG_LEVL_3);
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        return HAL_ERROR;
    }
    return HAL_OK; // Успешно отправлен HTTP запрос и получен ответ
}



HAL_StatusTypeDef parse_site_response(void) {

    // Ищем открывающую и закрывающую квадратные скобки
    char *start = strchr(parseBuffer, '[');
    char *end = strrchr(parseBuffer, ']');
    if (start == NULL || end == NULL || start > end) {
        USB_DEBUG_MESSAGE("[ERROR AT] Неверный формат ответа с сайта", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] Структура настроек с сайта верная", DEBUG_GSM, DEBUG_LEVL_4);
    // Извлекаем содержимое между скобками
    size_t len = end - start - 1;
    if (len == 0 || len >= 128) {  // 128 – размер временного буфера, можно настроить
        USB_DEBUG_MESSAGE("[ERROR AT] ТПревышен размер буфера для настроек с сайта", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    char dataStr[128] = {0};
    memcpy(dataStr, start + 1, len);
    dataStr[len] = '\0';

    /* Разбиваем строку на поля, разделённые символом ';'
       Ожидается 7 полей:
         1. Номер устройства (строка)
         2. Режим работы (целое число)
         3. Режим работы сна (целое число)
         4. Время сна (целое число)
         5. Корректировка (double)
         6. Максимальная глубина измерений (double)
         7. Номер телефона (строка)
    */
   #define BUFFER_SIZE 128
    char dataCopy[BUFFER_SIZE];
    strncpy(dataCopy, dataStr, BUFFER_SIZE - 1);
    dataCopy[BUFFER_SIZE - 1] = '\0';

    char devNum[32] = {0};
    unsigned int mode = 0;
    unsigned int sleepMode = 0;
    unsigned int sleepTime = 0;
    double correct = 0.0;
    double maxLvl = 0.0;
    char phone[20] = {0};

    int tokenCount = 0;
    char *token = strtok(dataCopy, ";");
    while (token != NULL)
    {
        tokenCount++;
        switch (tokenCount)
        {
        case 1:
            if (token[0] == '[') token++;
            strncpy(devNum, token, sizeof(devNum) - 1);
            devNum[sizeof(devNum) - 1] = '\0';
            break;
        case 2:
            mode = (unsigned int)strtoul(token, NULL, 10);
            break;
        case 3:
            sleepMode = (unsigned int)strtoul(token, NULL, 10);
            break;
        case 4:
            sleepTime = (unsigned int)strtoul(token, NULL, 10);
            break;
        case 5:
            correct = strtod(token, NULL);
            break;
        case 6:
            maxLvl = strtod(token, NULL);
            break;
        case 7:
            strncpy(phone, token, sizeof(phone) - 1);
            phone[sizeof(phone) - 1] = '\0';
            break;
        default:
            break;
        }
        token = strtok(NULL, ";");
    }

    if (tokenCount != 7)
    {
        // Если число токенов отличается от 7, данные считаются неверными
        USB_DEBUG_MESSAGE("[ERROR AT] Число токенов больше 7 - недопустимо", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }

    // Проверяем, совпадает ли полученный номер устройства с ожидаемым

    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);

    if (strcmp(devNum, Version) != 0) {
        USB_DEBUG_MESSAGE("[ERROR AT] Не совпадает номер усройства и номер на сайте", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    
    // Сохраняем разобранные данные в EEPROM
    if (EEPROM.Mode != 0) EEPROM.Mode = (uint8_t)mode;
    // Если режим сна равен 0, время сна трактуем как минуты, иначе как часы
    if (sleepMode == 1) {
        EEPROM.time_sleep_m = (uint16_t)sleepTime;
        EEPROM.time_sleep_h = 0;
    } else {
        EEPROM.time_sleep_h = (uint16_t)sleepTime;
        EEPROM.time_sleep_m = 0;
    }
    EEPROM.GVL_correct[0] = correct; 
    EEPROM.MAX_LVL[0] = maxLvl;
    // Копируем номер телефона, гарантируя наличие завершающего нуля
    strncpy(EEPROM.Phone, phone, sizeof(EEPROM.Phone) - 1);
    EEPROM.Phone[sizeof(EEPROM.Phone) - 1] = '\0';

    
    if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] Настройки с сайта успешно применены", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_OK;
}

HAL_StatusTypeDef waitAndParseSiteResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    

    char *start = 0;
    
    // Цикл ожидания поступления данных
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // Ожидаем появления новых данных с периодом до 5 секунд
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // При каждом получении новых данных проверяем:
            if (strstr(parseBuffer, "NO_BINDING") != NULL) {
                USB_DEBUG_MESSAGE("[WARNING AT] Устройство не привязано", DEBUG_GSM, DEBUG_LEVL_4);
                ERRCODE.STATUS |= STATUS_HTTP_NO_BINDING_ERROR;
                return HAL_ERROR;
            }
            if (strstr(parseBuffer, "INCORRECT_PASSWORD") != NULL) {
                USB_DEBUG_MESSAGE("[WARNING AT] Неверный пароль", DEBUG_GSM, DEBUG_LEVL_4);
                ERRCODE.STATUS |= STATUS_HTTP_WRONG_PASSWORD_ERROR;
                return HAL_ERROR;
            }
            start = strchr(parseBuffer, '[');
            if (start != NULL && strchr(start, ']') != NULL)
            {
                int res = parse_site_response();
                return res;
            }
            if (strstr(parseBuffer, "Error") != NULL)
            {
                USB_DEBUG_MESSAGE("[ERROR AT] Неизвестная ошибка", DEBUG_GSM, DEBUG_LEVL_4);
                break;
            }
        }
    }
    USB_DEBUG_MESSAGE("[ERROR AT] Таймаут чтения настроек с сайта", DEBUG_GSM, DEBUG_LEVL_4);
    // Если за время ожидания нужный пакет так и не обнаружен, возвращаем -1
    return HAL_TIMEOUT;
}
