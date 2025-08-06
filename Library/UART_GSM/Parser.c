/* parser.c */
#include "Parser.h"

#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))
static char command[1024] __attribute__((section(".ram2"))) =  {0};
char tempBuf[256]__attribute__((section(".ram2"))) = {0}; 

HAL_StatusTypeDef SendCommandAndParse(const char *command_in, int (*parser)(), uint32_t timeout)
{
    memset(tempBuf, 0, sizeof(tempBuf));
    snprintf(tempBuf, sizeof(tempBuf), "[DEBUG AT] ������������ �������: %s", command_in);
    //CDC_Transmit_FS((uint8_t *)tempBuf, strlen(tempBuf));
    USB_DEBUG_MESSAGE(tempBuf, DEBUG_GSM, DEBUG_LEVL_4);

    // ����� �������� ������ ����� ���������
    SendSomeCommandAndSetFlag();  
    // �������� �������
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
                return HAL_OK;  // SIM ����� ������������ � ������
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
    return HAL_TIMEOUT;  // �������
}


HAL_StatusTypeDef waitForCSQResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // ���� � parseBuffer ��������� "+CSQ:"
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

                    return HAL_OK; // ������� �������� ������� �������
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

    // ����� ������� ����������/���������������� GSM_data.Modem_mode
    // �� ���� ��� ������������, �������� ��� ����

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // ����, ���� ������ ������� � ������ ������ � parseBuffer
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // ���� ������ ���������
            char *p = strstr(parseBuffer, "+CEREG:");
            if (p != NULL)
            {
                p += strlen("+CEREG:");

                int n, stat, act;
                char tac[16] = {0};
                char ci[16]  = {0};

                // �������� ������� ����:
                // +CEREG: <n>, <stat>, "<tac>", "<ci>", <act>
                // ������: +CEREG: 0,1,"02b3","cea80000",0
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
        // ���, ����� ������ ����� ������ (������� �� �������).
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // ���� � ������ ������ ���������
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
            
            // ������������� ����� ����������� "ERROR" / "+CME ERROR"
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                // ���������� 0 ��� ���� ��� � ������ ������
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
                USB_DEBUG_MESSAGE("[DEBUG AT] ����� OK", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_OK; // ������� ����� OK
            }
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                USB_DEBUG_MESSAGE("[ERROR AT] ����� ERROR", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_ERROR; // ������� ����� ERROR
            }
        }
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] ������� �������� ��", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_TIMEOUT; // �������: �� OK, �� ERROR �� ����������
}

HAL_StatusTypeDef waitForGreaterThanResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    // ���� � ������ ��������� ������ '>', �������, ��� ����������� ��������
    if (strstr(parseBuffer, ">") != NULL)
    {
        return HAL_OK; // ������� ������ '>'
    }
    // ���� ��������� ERROR, ���������� ������
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return HAL_ERROR; // ������� ����� ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // ���� � ������ ��������� ������ '>', �������, ��� ����������� ��������
            if (strstr(parseBuffer, ">") != NULL)
            {
                return HAL_OK; // ������� ������ '>'
            }
            // ���� ��������� ERROR, ���������� ������
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return HAL_ERROR; // ������� ����� ERROR
            }
        }
    }

    return HAL_TIMEOUT; // �������: ������ '>' �� ���������
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
    return HAL_TIMEOUT;  // �������
}


HAL_StatusTypeDef waitForHTTPResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    int32_t m, s, d;
    ERRCODE.STATUS &= ~STATUS_HTTP_WRONG_PASSWORD_ERROR;
    // �������� ��������� ������. ���� � ������ ������������ ������ ������, ��������,
    // ����� � ����� "162.", sscanf ������� ����� 3, ��� ��� ����� ����� ��������� ���������.
    if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
    {
        if (s == 200)
            return HAL_OK;
        return HAL_BUSY; // ������� ������� � �������� �����
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
            {
                if (s == 200){
                     USB_DEBUG_MESSAGE("[DEBUG AT] ����� 200", DEBUG_GSM, DEBUG_LEVL_4);
                    return HAL_OK;
                }
                USB_DEBUG_MESSAGE("[ERROR AT] ����� �� 200", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_BUSY; // ������� ������� � �������� �����
            }
            if (strstr(parseBuffer, "error") != NULL)
            {
                break;
            }
        }
    }
    //USB_DEBUG_MESSAGE(parseBuffer, DEBUG_GSM, DEBUG_LEVL_4);
    //CDC_Transmit_FS(parseBuffer, sizeof(parseBuffer));
    return HAL_TIMEOUT; // �������: ����� �� ������� ��� �� ������������� �������
}


#define MAX_SMS_ATTEMPTS 1

HAL_StatusTypeDef sendSMS(void)
{
    int attempt;
    uint8_t smsSent = 0; // ���� �������� �������� SMS
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

    // ������� ��� ������ ���������� ������
    if (SendCommandAndParse("AT+CMGF=1\r", waitForOKResponse, 1000) != HAL_OK)
    {
        return HAL_ERROR; // ������ ��� ��������� ���������� ������
    }

    // ������� ��� ������ ��������� ��������
    if (SendCommandAndParse("AT+CSCS=\"GSM\"\r", waitForOKResponse, 1000) != HAL_OK)
    {
        osDelay(1000);
        return HAL_ERROR; 
    }

    // ������� ��� ������ �������� SMS
    // "AT+CMGS=\"+79150305966\"\r"
    memset(command, 0, sizeof(command));
    sprintf(command, "AT+CMGS=\"%s\"\r", EEPROM.Phone);
    // strcat(command, save_data);
    if (SendCommandAndParse(command, waitForGreaterThanResponse, 4000) != HAL_OK)
    {
        osDelay(1000);
        return HAL_ERROR; 
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] ���������� ��������� �� UART:", DEBUG_GSM, DEBUG_LEVL_3);
    USB_DEBUG_MESSAGE(save_data, DEBUG_GSM, DEBUG_LEVL_4);
    HAL_UART_Transmit(&huart4, save_data, strlen(save_data), 1000);
    
    if (waitForSMSResponse(90000) == HAL_OK){
        USB_DEBUG_MESSAGE("[DEBUG AT] SMS ���������� �������", DEBUG_GSM, DEBUG_LEVL_4);
    } else {
        USB_DEBUG_MESSAGE("[ERROR AT] ������ ��� �������� SMS", DEBUG_GSM, DEBUG_LEVL_4);
        ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
        return HAL_ERROR; // ������ ��� �������� ������ �� SMS
    } 
    return HAL_OK; 
}

#define MAX_HTTP_ATTEMPTS 3

HAL_StatusTypeDef sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // ���� �������� �������� HTTP
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
        // �������� HTTP ������� (send - ������ � ��������� �������������� URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != HAL_OK) {
            goto http_error_1;
        }
        osDelay(100);
        // ���������� HTTPACTION, �������� ������
        int res = SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 60000);
        if (ERRCODE.STATUS & STATUS_HTTP_WRONG_PASSWORD_ERROR) {
            // �������� ������ ��� 403
            SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);
            break;
        }
        if (res != HAL_OK) {
            goto http_error_1;
        }

        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 2000);
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);

        if (ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR) break;
        // ���� ������ ��� � �� ��������, ������� ������� ����������
        if (readResult != HAL_OK){
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
            goto http_error_1;
        }
        // ���� �� ���� �����, ������ ��� ������� ��������� �������
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
    osDelay(5000); // �������� 5 ������ ����� ��������� ��������
    }

    // ���� �� ���� �� ������� �� ���������� �������, ���������� ���� ������
    if (!httpSent) {
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef READ_Settings_sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // ���� �������� �������� HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?request=";
    
    // ���������, ���������� �� ����������: ����� send + ����� save_data + 2 ������� (��� " � \r) + ����������� \0
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
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ ���������������", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 2000) != HAL_OK) {
            goto http_error_2;
        }
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ ������� ������ ���������� �������", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        // �������� HTTP ������� (send - ������ � ��������� �������������� URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != HAL_OK) {
            goto http_error_2;
        }
        USB_DEBUG_MESSAGE("[DEBUG AT] HTTP READ ���������", DEBUG_GSM, DEBUG_LEVL_4);
        osDelay(100);
        // ���������� HTTPACTION, �������� ������
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
        // ���� ������ ��� � �� ��������, ������� ������� ����������
        if (readResult != HAL_OK) {
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        }
        
        // ���� �� ���� �����, ������ ��� ������� ��������� �������
        httpSent = 1;
        break;

http_error_2:
        USB_DEBUG_MESSAGE("[ERROR AT] ������ READ �������", DEBUG_GSM, DEBUG_LEVL_3);
        // � ������ ������ ��������� �������� ���������:
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 2000);
        SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 60000);
        SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000);
        SendCommandAndParse("AT+CGDCONT=1,\"IP\",\"internet.mts.ru\"\r", waitForOKResponse, 1000); 
        SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000);
        SendCommandAndParse("AT+CDNSCFG=\"8.8.8.8\",\"77.88.8.8\"\r", waitForOKResponse, 1000);
        osDelay(2000);  // ��������2 ������� ����� ��������� ��������
    }

    // ���� �� ���� �� ������� �� ���������� �������, ���������� ���� ������
    if (!httpSent) {
        USB_DEBUG_MESSAGE("[ERROR AT] ���������� - ������ ����� � ��������", DEBUG_GSM, DEBUG_LEVL_3);
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        return HAL_ERROR;
    }
    return HAL_OK; // ������� ��������� HTTP ������ � ������� �����
}



HAL_StatusTypeDef parse_site_response(void) {

    // ���� ����������� � ����������� ���������� ������
    char *start = strchr(parseBuffer, '[');
    char *end = strrchr(parseBuffer, ']');
    if (start == NULL || end == NULL || start > end) {
        USB_DEBUG_MESSAGE("[ERROR AT] �������� ������ ������ � �����", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] ��������� �������� � ����� ������", DEBUG_GSM, DEBUG_LEVL_4);
    // ��������� ���������� ����� ��������
    size_t len = end - start - 1;
    if (len == 0 || len >= 128) {  // 128 � ������ ���������� ������, ����� ���������
        USB_DEBUG_MESSAGE("[ERROR AT] ��������� ������ ������ ��� �������� � �����", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    char dataStr[128] = {0};
    memcpy(dataStr, start + 1, len);
    dataStr[len] = '\0';

    /* ��������� ������ �� ����, ���������� �������� ';'
       ��������� 7 �����:
         1. ����� ���������� (������)
         2. ����� ������ (����� �����)
         3. ����� ������ ��� (����� �����)
         4. ����� ��� (����� �����)
         5. ������������� (double)
         6. ������������ ������� ��������� (double)
         7. ����� �������� (������)
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
        // ���� ����� ������� ���������� �� 7, ������ ��������� ���������
        USB_DEBUG_MESSAGE("[ERROR AT] ����� ������� ������ 7 - �����������", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }

    // ���������, ��������� �� ���������� ����� ���������� � ���������

    char Version[20] = {0};
    strncpy(Version, Main_data.version.VERSION_PCB, sizeof(Version)-1);
    remove_braces_inplace(Version);

    if (strcmp(devNum, Version) != 0) {
        USB_DEBUG_MESSAGE("[ERROR AT] �� ��������� ����� ��������� � ����� �� �����", DEBUG_GSM, DEBUG_LEVL_4);
        return HAL_ERROR;
    }
    
    // ��������� ����������� ������ � EEPROM
    if (EEPROM.Mode != 0) EEPROM.Mode = (uint8_t)mode;
    // ���� ����� ��� ����� 0, ����� ��� �������� ��� ������, ����� ��� ����
    if (sleepMode == 1) {
        EEPROM.time_sleep_m = (uint16_t)sleepTime;
        EEPROM.time_sleep_h = 0;
    } else {
        EEPROM.time_sleep_h = (uint16_t)sleepTime;
        EEPROM.time_sleep_m = 0;
    }
    EEPROM.GVL_correct[0] = correct; 
    EEPROM.MAX_LVL[0] = maxLvl;
    // �������� ����� ��������, ���������� ������� ������������ ����
    strncpy(EEPROM.Phone, phone, sizeof(EEPROM.Phone) - 1);
    EEPROM.Phone[sizeof(EEPROM.Phone) - 1] = '\0';

    
    if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }
    USB_DEBUG_MESSAGE("[DEBUG AT] ��������� � ����� ������� ���������", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_OK;
}

HAL_StatusTypeDef waitAndParseSiteResponse(uint32_t timeout)
{
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    

    char *start = 0;
    
    // ���� �������� ����������� ������
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // ������� ��������� ����� ������ � �������� �� 5 ������
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            // ��� ������ ��������� ����� ������ ���������:
            if (strstr(parseBuffer, "NO_BINDING") != NULL) {
                USB_DEBUG_MESSAGE("[WARNING AT] ���������� �� ���������", DEBUG_GSM, DEBUG_LEVL_4);
                ERRCODE.STATUS |= STATUS_HTTP_NO_BINDING_ERROR;
                return HAL_ERROR;
            }
            if (strstr(parseBuffer, "INCORRECT_PASSWORD") != NULL) {
                USB_DEBUG_MESSAGE("[WARNING AT] �������� ������", DEBUG_GSM, DEBUG_LEVL_4);
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
                USB_DEBUG_MESSAGE("[ERROR AT] ����������� ������", DEBUG_GSM, DEBUG_LEVL_4);
                break;
            }
        }
    }
    USB_DEBUG_MESSAGE("[ERROR AT] ������� ������ �������� � �����", DEBUG_GSM, DEBUG_LEVL_4);
    // ���� �� ����� �������� ������ ����� ��� � �� ���������, ���������� -1
    return HAL_TIMEOUT;
}
