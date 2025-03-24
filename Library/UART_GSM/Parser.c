/* parser.c */
#include "Parser.h"

#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))
static char command[50] __attribute__((section(".ram2"))) =  {0};

int SendCommandAndParse(const char *command, int (*parser)(), uint32_t timeout)
{
    // Сброс текущего буфера перед отправкой
    SendSomeCommandAndSetFlag();  
    // Отправка команды
    HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), 1000);

    // Включаем приём (если data_read=0, то ничего не придёт)
    data_read = 1;

    // Сбрасываем висящий семафор, если он есть
    xSemaphoreTake(UART_PARSER_semaphore, 0);

    // Ждём прихода данных (по прерыванию) до заданного таймаута
    if (xSemaphoreTake(UART_PARSER_semaphore, timeout) == pdFALSE)
    {
        // Не дождались завершения приёма в течение timeout
        data_read = 0;
        return -1; 
    }

    // Здесь parseBuffer уже содержит принятые данные (полностью или частично)
    // Вызываем парсер, который ищет OK/ERROR/что угодно
    int result = parser();  // ваш parse_ERROR_OK или любой другой

    // После разбора можно отключить приём
    data_read = 0;
    return result;
}

void ND(){
    osDelay(100);
}

int parse_CPIN()
{
    if (strstr(parseBuffer, "+CPIN:READY") != NULL){
        GSM_data.Status |= SIM_PRESENT;
        ERRCODE.STATUS &= ~STATUS_GSM_NO_SIM;
        return 1;
    }
    GSM_data.Status &= ~SIM_PRESENT;
    ERRCODE.STATUS |= STATUS_GSM_NO_SIM;
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
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0; // Нашли "ERROR"
    }
    if (strstr(parseBuffer, "OK") != NULL)
    {
        return 1; // Нашли "OK"
    }
    return -1;     // Не нашли ни "ERROR", ни "OK"
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


#define MAX_SMS_ATTEMPTS 3

int sendSMS(void)
{
    int attempt;
    uint8_t smsSent = 0; // Флаг успешной отправки SMS
    size_t len = strlen(save_data);
    if (len + 2 < CMD_BUFFER_SIZE)
    {
        save_data[len] = '\x1A';
        save_data[len + 1] = '\r';
        save_data[len + 2] = '\0';
    }

    if (SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000) != 1)
    {
    };
    if (SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 1000) != 1)
    {
    };
    if (SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000) != 1)
    {
    };

    for (attempt = 0; attempt < MAX_SMS_ATTEMPTS; attempt++)
    {
        // Команда для выбора текстового режима
        if (SendCommandAndParse("AT+CMGF=1\r", waitForOKResponse, 1000) != 1)
        {
            osDelay(5000); // Задержка 5 секунд перед повторной попыткой
            continue;      // Переходим к следующей попытке
        }

        // Команда для выбора кодировки символов
        if (SendCommandAndParse("AT+CSCS=\"GSM\"\r", waitForOKResponse, 1000) != 1)
        {
            osDelay(5000);
            continue;
        }

        // Команда для начала отправки SMS
        // "AT+CMGS=\"+79150305966\"\r"
        sprintf(command, "AT+CMGS=\"%s\"\r", EEPROM.Phone);
        if (SendCommandAndParse(command, waitForGreaterThanResponse, 1000) != 1)
        {
            osDelay(5000);
            continue;
        }
        HAL_UART_Transmit(&huart4, save_data, strlen(save_data), 1000);

        // Если все команды выполнены успешно, считаем, что SMS отправлена
        smsSent = 1;
        break;
    }

    // Если ни одна из попыток не увенчалась успехом, выставляем флаг ошибки
    if (!smsSent) {
        ERRCODE.STATUS |= STATUS_UART_SMS_SEND_ERROR;
        return -1;
    }
    return 0;
}

#define MAX_HTTP_ATTEMPTS 3

int sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // Флаг успешной отправки HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?data=";
    size_t len = strlen(save_data);
    if (len + 2 < 512) {
        save_data[len] = '"';
        save_data[len + 1] = '\r';
        save_data[len + 2] = '\0';
    }
    strcat(send, save_data);

    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        // Предварительная очистка состояния перед началом
        //SendCommandAndParse("AT+HTTPTERM\r", parse_ERROR_OK, 1000);
        //SendCommandAndParse("AT+CGACT=0\r", parse_ERROR_OK, 1000);
        //SendCommandAndParse("AT+CGDCONT=1\r", parse_ERROR_OK, 1000);
        // Начало настройки соединения
        if (SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        // Отправка HTTP запроса (send - строка с корректно сформированным URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != 1) {
            goto http_error;
        }
        // Выполнение HTTPACTION, проверка ответа
        if (SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 20000) != 1) {
            goto http_error;
        }
        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        if (readResult != 1) {
            readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        }
        osDelay(100);
        // После попыток, вне зависимости от результата, завершаем HTTP сессию
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
        // Если данные так и не получены, считаем попытку неуспешной
        if (readResult != 1) {
            ERRCODE.STATUS |= STATUS_UART_NO_RESPONSE;
        }

        // Если до сюда дошли, значит все команды выполнены успешно
        httpSent = 1;
        break;

http_error:
        // В случае ошибки выполняем «очистку» состояния:
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);
        osDelay(5000);  // Задержка 5 секунд перед следующей попыткой
    }

    // Если ни одна из попыток не увенчалась успехом, выставляем флаг ошибки
    if (!httpSent) {
        ERRCODE.STATUS |= STATUS_UART_SERVER_COMM_ERROR;
        return -1;
    }
    return 0;
}

int READ_Settings_sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // Флаг успешной отправки HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?request=";
    
    size_t len = strlen(save_data);
    if (len + 2 < 512) {
        save_data[len] = '"';
        save_data[len + 1] = '\r';
        save_data[len + 2] = '\0';
    }
    strcat(send, save_data);

    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        // Предварительная очистка состояния перед началом
        //SendCommandAndParse("AT+HTTPTERM\r", parse_ERROR_OK, 1000);
        //SendCommandAndParse("AT+CGACT=0\r", parse_ERROR_OK, 1000);
        //SendCommandAndParse("AT+CGDCONT=1\r", parse_ERROR_OK, 1000);
        // Начало настройки соединения
        if (SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error;
        }
        // Отправка HTTP запроса (send - строка с корректно сформированным URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != 1) {
            goto http_error;
        }
        // Выполнение HTTPACTION, проверка ответа
        if (SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 20000) != 1) {
            goto http_error;
        }
        //osDelay(2000);
        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        if (readResult != 1) {
            readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        }
        // После попыток, вне зависимости от результата, завершаем HTTP сессию
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
        // Если данные так и не получены, считаем попытку неуспешной
        if (readResult != 1) {
            ERRCODE.STATUS |= STATUS_UART_NO_RESPONSE;
        }
        
        // Если до сюда дошли, значит все команды выполнены успешно
        httpSent = 1;
        break;

http_error:
        // В случае ошибки выполняем «очистку» состояния:
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);
        osDelay(5000);  // Задержка 5 секунд перед следующей попыткой
    }

    // Если ни одна из попыток не увенчалась успехом, выставляем флаг ошибки
    if (!httpSent) {
        ERRCODE.STATUS |= STATUS_UART_SERVER_COMM_ERROR;
        return -1;
    }
    return 0;
}

int Send_data(){
    Collect_DATA();
    if (sendHTTP() == 0) return 0;
    ERRCODE.STATUS |= STATUS_UART_SERVER_COMM_ERROR;
    if (sendSMS() == 0) return 0;
    ERRCODE.STATUS |= STATUS_UART_SMS_SEND_ERROR;
    return -1;
}





int parse_site_response(void) {

    // Ищем открывающую и закрывающую квадратные скобки
    char *start = strchr(parseBuffer, '[');
    char *end = strrchr(parseBuffer, ']');
    if (start == NULL || end == NULL || start > end) {
        return -1;
    }
    
    // Извлекаем содержимое между скобками
    size_t len = end - start - 1;
    if (len == 0 || len >= 128) {  // 128 – размер временного буфера, можно настроить
        return -1;
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
        return -1;
    }

    // Проверяем, совпадает ли полученный номер устройства с ожидаемым
    if (strcmp(devNum, EEPROM.version.VERSION_PCB) != 0) {
        return -1;
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
    EEPROM.GVL_correct = correct;
    EEPROM.MAX_LVL = maxLvl;
    // Копируем номер телефона, гарантируя наличие завершающего нуля
    strncpy(EEPROM.Phone, phone, sizeof(EEPROM.Phone) - 1);
    EEPROM.Phone[sizeof(EEPROM.Phone) - 1] = '\0';

    if (!EEPROM_SaveSettings(&EEPROM))
    {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }

    return 1;
}

int waitAndParseSiteResponse(void)
{
    uint32_t timeout = 20000;  // Таймаут ожидания в мс
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    
    if (strstr(parseBuffer, "NO_BINDING") != NULL) {
        return 0;
    }
    char *start = strchr(parseBuffer, '[');
    if (start != NULL && strchr(start, ']') != NULL) {
        int res = parse_site_response();
        return res;  // 1 при успехе, -1 при ошибке
    }
    
    // Цикл ожидания поступления данных
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // Ожидаем появления новых данных с периодом до 5 секунд
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // При каждом получении новых данных проверяем:
            if (strstr(parseBuffer, "NO_BINDING") != NULL) {
                return 0;
            }
            start = strchr(parseBuffer, '[');
            if (start != NULL && strchr(start, ']') != NULL)
            {
                int res = parse_site_response();
                return res;
            }
        }
    }
    // Если за время ожидания нужный пакет так и не обнаружен, возвращаем -1
    return -1;
}
