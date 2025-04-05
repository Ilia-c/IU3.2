/* parser.c */
#include "Parser.h"

#define NUM_AT_COMMANDS (sizeof(atCommandList)/sizeof(atCommandList[0]))
static char command[1024] __attribute__((section(".ram2"))) =  {0};
char tempBuf[256]__attribute__((section(".ram2"))) = {0}; 

int SendCommandAndParse(const char *command_in, int (*parser)(), uint32_t timeout)
{
    memset(tempBuf, 0, sizeof(tempBuf));
    snprintf(tempBuf, sizeof(tempBuf), "SEND: %s", command_in);
    CDC_Transmit_FS((uint8_t *)tempBuf, strlen(tempBuf));

    // ����� �������� ������ ����� ���������
    SendSomeCommandAndSetFlag();  
    // �������� �������
    GSM_data.Status |= DATA_READ;
    osDelay(10);
    HAL_UART_Transmit(&huart4, (uint8_t *)command_in, strlen(command_in), 1000);

    // ���������� ������� �������, ���� �� ����
    xSemaphoreTake(UART_PARSER_semaphore, 0);

    // ��� ������� ������ (�� ����������) �� ��������� ��������
    if (xSemaphoreTake(UART_PARSER_semaphore, timeout) == pdFALSE)
    {
        // �� ��������� ���������� ����� � ������� timeout
        GSM_data.Status &= ~DATA_READ;
        return -1; 
    }

    // ����� parseBuffer ��� �������� �������� ������ (��������� ��� ��������)
    // �������� ������, ������� ���� OK/ERROR/��� ������
    int result = 0;
    if (parser != NULL) result = parser();  // ��� parse_ERROR_OK ��� ����� ������

    // ����� ������� ����� ��������� ����
    GSM_data.Status &= ~DATA_READ;
    
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
    // ���� � ������ ��������� "+CSQ:"
    char *p = strstr(parseBuffer, "+CSQ:");
    if (p != NULL)
    {
        // ������� ��������� �� "+CSQ:" � ���������� ��������� �������
        p += strlen("+CSQ:");
        while (*p == ' ' || *p == '\t')
        {
            p++;
        }
        
        // �������� ��������� ��� �������� �������� (16-������), ���������� �������
        if (sscanf(p, "%hd,%hd", &rssi, &ber) == 2)
        {
            //rssi - �������� ������� 0-31, 99
            //ber - ������� ������ 0-7, 99
            GSM_data.GSM_Signal_Level = rssi;
            GSM_data.GSM_Signal_Errors = ber;

            if (GSM_data.GSM_Signal_Level <= 5) ERRCODE.STATUS |= STATUS_GSM_SIGNEL_ERROR;
            else ERRCODE.STATUS &= ~STATUS_GSM_SIGNEL_ERROR;
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
        // ��������� �� �������
        return -1;
    }
    
    p += strlen("+CEREG:");
    
    // �������� ��������� ������ � �������:
    // <n>, <stat>, "<tac>", "<ci>", <act>
    // ������ ������: +CEREG: 0, 1,"02b3","cea80000",0
    if (sscanf(p, " %d , %d , \"%15[^\"]\" , \"%15[^\"]\" , %d", &n, &stat, tac, ci, &act) == 5)
    {
        //stat - X � act - Y
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
    char oper[32] = {0};  // ����� ��� ��������� (������������ ������� ��� �������� ������ ���������)
    
    // ���� � parseBuffer ��������� "+COPS:"
    char *p = strstr(parseBuffer, "+COPS:");
    if (p == NULL)
    {
        // ���� ��������� �� �������, ���������� -1
        return -1;
    }
    
    // ������� ��������� �� "+COPS:"
    p += strlen("+COPS:");
    
    // �������� ��������� ������ � �������: mode, format, "oper", act
    // ������ ������: +COPS: 0,2,"25001", 0
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
        return 0; // ����� "ERROR"
    }
    if (strstr(parseBuffer, "OK") != NULL)
    {
        return 1; // ����� "OK"
    }
    return -1;     // �� ����� �� "ERROR", �� "OK"
}

int waitForOKResponse()
{
    uint32_t timeout = 10000;
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    if (strstr(parseBuffer, "OK") != NULL)
    {
        return 1; // ������� ����� OK
    }
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0; // ������� ����� ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (strstr(parseBuffer, "OK") != NULL)
            {
                return 1; // ������� ����� OK
            }
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return 0; // ������� ����� ERROR
            }
        }
    }
    return -1; // �������: �� OK, �� ERROR �� ����������
}

int waitForGreaterThanResponse(void)
{
    uint32_t timeout = 10000; // ������� � ��
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    // ���� � ������ ��������� ������ '>', �������, ��� ����������� ��������
    if (strstr(parseBuffer, ">") != NULL)
    {
        return 1; // ������� ������ '>'
    }
    // ���� ��������� ERROR, ���������� ������
    if (strstr(parseBuffer, "ERROR") != NULL)
    {
        return 0; // ������� ����� ERROR
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // ���� � ������ ��������� ������ '>', �������, ��� ����������� ��������
            if (strstr(parseBuffer, ">") != NULL)
            {
                return 1; // ������� ������ '>'
            }
            // ���� ��������� ERROR, ���������� ������
            if (strstr(parseBuffer, "ERROR") != NULL)
            {
                return 0; // ������� ����� ERROR
            }
        }
    }
    return -1; // �������: ������ '>' �� ���������
}

int waitForHTTPResponse()
{
    uint32_t timeout = 60000;
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    int32_t m, s, d;
    ERRCODE.STATUS &= ~STATUS_HTTP_WRONG_PASSWORD_ERROR;
    // �������� ��������� ������. ���� � ������ ������������ ������ ������, ��������,
    // ����� � ����� "162.", sscanf ������� ����� 3, ��� ��� ����� ����� ��������� ���������.
    if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
    {
        if (s == 200)
            return 1;
        return 0; // ������� ������� � �������� �����
    }
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            if (sscanf(parseBuffer, "%ld %ld %ld", &m, &s, &d) == 3)
            {
                if (s == 200)
                    return 1;
                return 0; // ������� ������� � �������� �����
            }
        }
    }
    CDC_Transmit_FS(parseBuffer, sizeof(parseBuffer));
    return -1; // �������: ����� �� ������� ��� �� ������������� �������
}


#define MAX_SMS_ATTEMPTS 3

int sendSMS(void)
{
    int attempt;
    uint8_t smsSent = 0; // ���� �������� �������� SMS
    if (strlen(save_data) + 3 < sizeof(save_data) )
    {
        strcat(save_data, "\x1A\r");
    }
    else{
        return -1;
    }

    if (SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000) != 1)
    {
    };
    if (SendCommandAndParse("AT+CGACT=0\r", waitForOKResponse, 60000) != 1)
    {
    };
    if (SendCommandAndParse("AT+CGDCONT=1\r", waitForOKResponse, 1000) != 1)
    {
    };

    for (attempt = 0; attempt < MAX_SMS_ATTEMPTS; attempt++)
    {
        // ������� ��� ������ ���������� ������
        if (SendCommandAndParse("AT+CMGF=1\r", waitForOKResponse, 1000) != 1)
        {
            osDelay(5000); // �������� 5 ������ ����� ��������� ��������
            continue;      // ��������� � ��������� �������
        }

        // ������� ��� ������ ��������� ��������
        if (SendCommandAndParse("AT+CSCS=\"GSM\"\r", waitForOKResponse, 1000) != 1)
        {
            osDelay(5000);
            continue;
        }

        // ������� ��� ������ �������� SMS
        // "AT+CMGS=\"+79150305966\"\r"
        memset(command, 0, sizeof(command));
        sprintf(command, "AT+CMGS=\"%s\"\r", EEPROM.Phone);
        strcat(command, save_data);
        if (SendCommandAndParse(command, waitForGreaterThanResponse, 1000) != 1)
        {
            osDelay(2000);
            continue;
        }
        HAL_UART_Transmit(&huart4, save_data, strlen(save_data), 1000);
        osDelay(30000);
        // ���� ��� ������� ��������� �������, �������, ��� SMS ����������
        smsSent = 1;
        break;
    }

    // ���� �� ���� �� ������� �� ���������� �������, ���������� ���� ������
    if (!smsSent) {
        ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
        return -1;
    }
    return 0;
}

#define MAX_HTTP_ATTEMPTS 3

int sendHTTP(void) {
    int attempt;
    uint8_t httpSent = 0; // ���� �������� �������� HTTP
    char send[512] = "AT+HTTPPARA=\"URL\",\"http://geosp-data.ru/api/save-data?data=";
    if ( (strlen(send) + strlen(save_data) + 3) < sizeof(send) )
    {
        strcat(send, save_data);
        strcat(send, "\"\r");
    }
    else{
        return -1;
    }

    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 1000) != 1) {
            goto http_error_1;
        }
        osDelay(100);
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error_1;
        }
        // �������� HTTP ������� (send - ������ � ��������� �������������� URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != 1) {
            goto http_error_1;
        }
        osDelay(100);
        // ���������� HTTPACTION, �������� ������
        int res = SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 30000);
        if (ERRCODE.STATUS & STATUS_HTTP_WRONG_PASSWORD_ERROR) {
            // �������� ������ ��� 403
            SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 12000);
            break;
        }
        if (res == 0){
            goto http_error_1;
        }

        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 12000);

        if (ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR) break;
        // ���� ������ ��� � �� ��������, ������� ������� ����������
        if (readResult != 1){
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
            goto http_error_1;
        }
        // ���� �� ���� �����, ������ ��� ������� ��������� �������
        httpSent = 1;
        break;

http_error_1:
    SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
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
        return -1;
    }
    return 0;
}

int READ_Settings_sendHTTP(void) {
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
        return -1;
    }
    


    for (attempt = 0; attempt < MAX_HTTP_ATTEMPTS; attempt++) {
        if (SendCommandAndParse("AT+HTTPINIT\r", waitForOKResponse, 1000) != 1) {
            goto http_error_2;
        }
        osDelay(100);
        
        if (SendCommandAndParse("AT+HTTPPARA=\"CID\",\"1\"\r", waitForOKResponse, 1000) != 1) {
            goto http_error_2;
        }
        osDelay(100);
        // �������� HTTP ������� (send - ������ � ��������� �������������� URL)
        if (SendCommandAndParse(send, waitForOKResponse, 20000) != 1) {
            goto http_error_2;
        }
        osDelay(100);
        // ���������� HTTPACTION, �������� ������
        int res = SendCommandAndParse("AT+HTTPACTION=0\r", waitForHTTPResponse, 30000);
        if (ERRCODE.STATUS & STATUS_HTTP_WRONG_PASSWORD_ERROR) {
            // �������� ������ ��� 403
            SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 120000);
            break;
        }
        if (res == 0){
            goto http_error_2;
        }
        osDelay(1000);
        int readResult = SendCommandAndParse("AT+HTTPREAD=0,200\r", waitAndParseSiteResponse, 1000);
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 120000);

        if (ERRCODE.STATUS & STATUS_HTTP_NO_BINDING_ERROR){
            httpSent = 1;
            break;
        }
        // ���� ������ ��� � �� ��������, ������� ������� ����������
        if (readResult != 1) {
            ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        }
        
        // ���� �� ���� �����, ������ ��� ������� ��������� �������
        httpSent = 1;
        break;

http_error_2:
        // � ������ ������ ��������� �������� ���������:
        SendCommandAndParse("AT+HTTPTERM\r", waitForOKResponse, 1000);
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
        osDelay(5000);  // �������� 5 ������ ����� ��������� ��������
    }

    // ���� �� ���� �� ������� �� ���������� �������, ���������� ���� ������
    if (!httpSent) {
        ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
        return -1;
    }
    return 0;
}

int Send_data(){
    Collect_DATA();
    if (sendHTTP() == 0) return 0;
    ERRCODE.STATUS |= STATUS_HTTP_SERVER_COMM_ERROR;
    if (sendSMS() == 0) return 0;
    ERRCODE.STATUS |= STATUS_GSM_SMS_SEND_ERROR;
    return -1;
}





int parse_site_response(void) {

    // ���� ����������� � ����������� ���������� ������
    char *start = strchr(parseBuffer, '[');
    char *end = strrchr(parseBuffer, ']');
    if (start == NULL || end == NULL || start > end) {
        return -1;
    }
    
    // ��������� ���������� ����� ��������
    size_t len = end - start - 1;
    if (len == 0 || len >= 128) {  // 128 � ������ ���������� ������, ����� ���������
        return -1;
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
        return -1;
    }

    // ���������, ��������� �� ���������� ����� ���������� � ���������
    if (strcmp(devNum, EEPROM.version.VERSION_PCB) != 0) {
        return -1;
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
    EEPROM.GVL_correct = correct;
    EEPROM.MAX_LVL = maxLvl;
    // �������� ����� ��������, ���������� ������� ������������ ����
    strncpy(EEPROM.Phone, phone, sizeof(EEPROM.Phone) - 1);
    EEPROM.Phone[sizeof(EEPROM.Phone) - 1] = '\0';

    
    if (EEPROM_SaveSettings(&EEPROM) != HAL_OK)
    {
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
    }

    return 1;
}

int waitAndParseSiteResponse(void)
{
    uint32_t timeout = 20000;  // ������� �������� � ��
    TickType_t startTick = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);
    
    if (strstr(parseBuffer, "NO_BINDING") != NULL) {
        ERRCODE.STATUS |= STATUS_HTTP_NO_BINDING_ERROR;
        return 0;
    }
    if (strstr(parseBuffer, "INCORRECT_PASSWORD") != NULL) {
        ERRCODE.STATUS |= STATUS_HTTP_WRONG_PASSWORD_ERROR;
        return 0;
    }
    char *start = strchr(parseBuffer, '[');
    if (start != NULL && strchr(start, ']') != NULL) {
        int res = parse_site_response();
        return res;  // 1 ��� ������, -1 ��� ������
    }
    
    // ���� �������� ����������� ������
    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // ������� ��������� ����� ������ � �������� �� 5 ������
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(5000)) == pdTRUE)
        {
            // ��� ������ ��������� ����� ������ ���������:
            if (strstr(parseBuffer, "NO_BINDING") != NULL) {
                ERRCODE.STATUS |= STATUS_HTTP_NO_BINDING_ERROR;
                return 0;
            }
            if (strstr(parseBuffer, "INCORRECT_PASSWORD") != NULL) {
                ERRCODE.STATUS |= STATUS_HTTP_WRONG_PASSWORD_ERROR;
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
    // ���� �� ����� �������� ������ ����� ��� � �� ���������, ���������� -1
    return -1;
}
