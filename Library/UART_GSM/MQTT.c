#include "MQTT.h"

// ===== ���������� ������ ��������� =====
static MqttSm g_mqtt;

// ===== ��������� ������� (�� � ����� SendCommandAndParse) =====

static HAL_StatusTypeDef waitForConnectResult(uint32_t timeout)
{
    TickType_t startTick    = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        // ���, ���� ������ ������� ����� � parseBuffer � �������� �������
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            if (!parseBuffer) {
                continue;
            }

            // �������� �����������
            if (strstr(parseBuffer, "CONNECT OK") != NULL) {
                USB_DEBUG_MESSAGE("[DEBUG AT] CONNECT OK", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_OK;
            }

            // ����� ������
            if (strstr(parseBuffer, "CONNECT FAIL") != NULL) {
                USB_DEBUG_MESSAGE("[ERROR AT] CONNECT FAIL", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_ERROR;
            }
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                USB_DEBUG_MESSAGE("[ERROR AT] CONNECT ERROR", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_ERROR;
            }

            // ���� ����� �� ��� � ���������� �� ��������
        }
    }

    USB_DEBUG_MESSAGE("[DEBUG AT] ������� �������� CONNECT OK/FAIL", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_TIMEOUT;
}


// --- ��������� ������ "+MQTTSTATU:<n>" ����� AT+MQTTSTATU ---
static int  s_last_mqtt_status = -1;  // 0..4
static HAL_StatusTypeDef waitForMQTTSTATU(uint32_t timeout)
{
    TickType_t startTick    = xTaskGetTickCount();
    TickType_t timeoutTicks = pdMS_TO_TICKS(timeout);

    s_last_mqtt_status = -1;

    while ((xTaskGetTickCount() - startTick) < timeoutTicks)
    {
        if (xSemaphoreTake(UART_PARSER_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            if (!parseBuffer) continue;

            // ���� ����� ������� ������ � ����� �������
            if (strstr(parseBuffer, "ERROR") != NULL ||
                strstr(parseBuffer, "+CME ERROR") != NULL)
            {
                USB_DEBUG_MESSAGE("[ERROR AT] MQTTSTATU: ERROR", DEBUG_GSM, DEBUG_LEVL_4);
                return HAL_ERROR;
            }

            // ���� ��������� ������� "+MQTTSTATU:" (�� ������ ���������� ����� � ������)
            char *p = parseBuffer;
            char *last = NULL;
            while ((p = strstr(p, "+MQTTSTATU:")) != NULL) {
                last = p;
                p += strlen("+MQTTSTATU:");
            }

            if (last) {
                p = last + strlen("+MQTTSTATU:");
                while (*p == ' ' || *p == '\t') p++;

                char *endp = p;
                long val = strtol(p, &endp, 10); // ������� 0..4
                if (endp != p && val == 2) {
                    USB_DEBUG_MESSAGE("[DEBUG AT] MQTTSTATU �������", DEBUG_GSM, DEBUG_LEVL_4);
                    return HAL_OK;
                } else {
                    return HAL_ERROR;
                    // �������� ������/�������� � ���������� ����� �� ��������
                }
            }
        }
    }

    USB_DEBUG_MESSAGE("[DEBUG AT] ������� �������� +MQTTSTATU:<n>", DEBUG_GSM, DEBUG_LEVL_4);
    return HAL_TIMEOUT;
}

// ������ +MQTTSTATU:<n> �� parseBuffer (������� ����� SendCommandAndParse("AT+MQTTSTATU",...))
static int mqtt_parse_status_from_buffer(uint8_t *st_out) {
    if (!parseBuffer) return -1;
    char *p = strstr(parseBuffer, "+MQTTSTATU:");
    if (!p) return -1;
    p += (int)strlen("+MQTTSTATU:");
    while (*p==' ' || *p=='\t') p++;
    int st = -1;
    if (sscanf(p, "%d", &st) == 1 && st >= 0) {
        if (st_out) *st_out = (uint8_t)st;
        return 0;
    }
    return -1;
}

// ����� ������� ����������
static HAL_StatusTypeDef MQTT_QueryStatus(uint8_t *st)
{
    s_last_mqtt_status = -1;
    if (SendCommandAndParse("AT+MQTTSTATU\r", waitForMQTTSTATU, 3000) != HAL_OK)
        return HAL_ERROR;

    if (s_last_mqtt_status < 0)
        return HAL_ERROR;
    if (st)
        *st = (uint8_t)s_last_mqtt_status;
    return HAL_OK;
}

// ������ ���������� ����������� ����� AT+MCONNECT
// �������, ��� � parseBuffer ���� "CONNECT OK" (�����) ��� "CONNECT FAIL"/"ERROR"
static HAL_StatusTypeDef mqtt_after_connect_check(void)
{
    if (!parseBuffer)
        return HAL_ERROR;
    if (strstr(parseBuffer, "CONNECT OK"))
        return HAL_OK;
    if (strstr(parseBuffer, "CONNECT FAIL"))
        return HAL_ERROR;
    if (strstr(parseBuffer, "ERROR"))
        return HAL_ERROR;
    return HAL_OK;
}

void MQTT_BuildCredentials(MqttCredentials *out)
{
    memset(out, 0, sizeof(*out));
    strncpy(out->clientId, Main_data.version.VERSION_PCB, sizeof(out->clientId)-1);
    strncpy(out->username, Main_data.version.VERSION_PCB, sizeof(out->username)-1);
    strncpy(out->password, Main_data.version.password,   sizeof(out->password)-1);
    remove_braces_inplace(out->clientId);
    remove_braces_inplace(out->username);
    remove_braces_inplace(out->password);
}

// ===== ������������� SM =====
void MQTT_InitGlobal(void)
{
    memset(&g_mqtt, 0, sizeof(g_mqtt));
    g_mqtt.st            = MQTT_ST_IDLE;
    g_mqtt.port          = MQTT_BROKER_PORT;
    g_mqtt.keepalive_s   = MQTT_KEEPALIVE_S;
    g_mqtt.clean_session = MQTT_CLEAN_SESSION;
    strncpy(g_mqtt.host, MQTT_BROKER_ADDR, sizeof(g_mqtt.host)-1);
    MQTT_BuildCredentials(&g_mqtt.cred);
    g_mqtt.t_last_stat_ms = 0;
}

// ===== ���� ��� ����������� (������ ��� � ����� ���� SendCommandAndParse) =====
static HAL_StatusTypeDef MQTT_SmStepConnect(MqttSm *sm)
{
    switch (sm->st)
    {
    case MQTT_ST_IDLE: {
        // ������������ PDP (�� ������, �� � ��� ������ ����������)
        if (SendCommandAndParse("AT+CGACT=1,1\r", waitForOKResponse, 60000) != HAL_OK) {
            ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR;
            sm->st = MQTT_ST_ERROR; return HAL_ERROR;
        }
        sm->st = MQTT_ST_PDP_UP;
        return HAL_BUSY;
    }

    case MQTT_ST_PDP_UP: {
        // ��������� clientId/user/pass ����� ���������
        char cmd[200];
        snprintf(cmd, sizeof(cmd), "AT+MCONFIG=\"%s\",\"%s\",\"%s\"\r",
                 sm->cred.clientId, sm->cred.username, sm->cred.password);
        osDelay(1000); // ��������� �����
        if (SendCommandAndParse(cmd, waitForOKResponse, 10000) != HAL_OK) {
            ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR;
            sm->st = MQTT_ST_ERROR; return HAL_ERROR;
        }
        sm->st = MQTT_ST_CFG;
        return HAL_BUSY;
    }

    case MQTT_ST_CFG:
    {
        char cmd[200];
        snprintf(cmd, sizeof(cmd), "AT+MCONNECT=\"%s\",%d,%d,%u\r",
                 sm->host, sm->port, sm->clean_session, (unsigned)sm->keepalive_s);

        // �̨� CONNECT OK / CONNECT FAIL
        if (SendCommandAndParse(cmd, waitForConnectResult, 60000) != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR;
            sm->st = MQTT_ST_ERROR;
            return HAL_ERROR;
        }
        sm->connected = 1;
        sm->st = MQTT_ST_CONNECTED;
        ERRCODE.STATUS |= STATUS_MQTT_CONN;

        //MQTT_PublishSaveData(save_data, 60000); // � ������ �����
        //MQTT_PublishEmptySecond(60000);         // ������ ������ �� ������ �����

        return HAL_OK;
    }

    case MQTT_ST_CONNECTED:
        // ����������� � ������ � �����������
        if (MQTT_Subscribe(TOPIC_SETTINGS_FMT, MQTT_QOS_SUB, 5000) == HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_MQTT_SUB_SETTINGS;
            MQTT_PublishEmptySecond(10000); // ����� �������� ���������
            osDelay(1000); // ������� ��� �� ���� ����������� �������� ��������� (��� �����)
        }
        else{
            sm->st = MQTT_ST_ERROR;
            ERRCODE.STATUS &= ~STATUS_MQTT_SUB_SETTINGS;
            return HAL_ERROR;
        }
        return HAL_OK;

    default:
    case MQTT_ST_ERROR:
        ERRCODE.STATUS &= ~STATUS_MQTT_CONN;
        return HAL_ERROR;
    }
}

// ===== ��������� ���������� � �������� ��������� (100�500 ��) =====
void MQTT_Process(uint32_t now_ms)
{
    // ���� �� ���������� � ������ ��������� ��� �����������
    if (!g_mqtt.connected) {
        (void)MQTT_SmStepConnect(&g_mqtt);
        return;
    }

    // ������������� ������� �������
    if (now_ms - g_mqtt.t_last_stat_ms > 15000u) {
        g_mqtt.t_last_stat_ms = now_ms;
        uint8_t st = 0xFF;
        if (MQTT_QueryStatus(&st) == HAL_OK) {
            if (st != 2) { // not connected
                g_mqtt.connected = 0;
                g_mqtt.st = MQTT_ST_IDLE;
            }
        }
    }
}

// ===== ����������� ��������� ���������� �� �������� =====
HAL_StatusTypeDef MQTT_EnsureConnected(uint32_t deadline_ms)
{
    if (g_mqtt.connected && g_mqtt.st == MQTT_ST_CONNECTED) return HAL_OK;

    while (HAL_GetTick() < deadline_ms) {
        HAL_StatusTypeDef r = MQTT_SmStepConnect(&g_mqtt);
        if (r == HAL_OK && g_mqtt.connected) return HAL_OK;
        if (r == HAL_ERROR) return HAL_ERROR;
        osDelay(100);
    }
    ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR;
    return HAL_TIMEOUT;
}

// ===== ���������� (�����) � �� ����� ���� SendCommandAndParse, ����� ������ ����� =====
HAL_StatusTypeDef MQTT_Publish(const char *topic,
                               const char *payload,
                               int qos, int retain,
                               uint32_t timeout_ms)
{
    if (!g_mqtt.connected) { ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR; return HAL_ERROR; }

    char cmd[512+128];
    if (strlen(topic) + strlen(payload) + 64 >= sizeof(cmd)) {
        ERRCODE.STATUS |= STATUS_MQTT_PUB_ERROR; return HAL_ERROR;
    }

    // ��������� M*: AT+MPUB="<topic>",<qos>,<retain>,"<message>"
    snprintf(cmd, sizeof(cmd), "AT+MPUB=\"%s\",%d,%d,\"%s\"\r",
             topic, qos, retain, payload);

    if (SendCommandAndParse(cmd, waitForOKResponse, timeout_ms) != HAL_OK) {
        ERRCODE.STATUS |= STATUS_MQTT_PUB_ERROR;
        if (parseBuffer) {
            if (strstr(parseBuffer, "AUTH") || strstr(parseBuffer, "auth"))
                ERRCODE.STATUS |= STATUS_MQTT_AUTH_ERROR;
        }
        return HAL_ERROR;
    }
    // Best-effort: ������������ ������
    uint8_t st = 0xFF;
    if (MQTT_QueryStatus(&st) != HAL_OK || st != 2) {
        g_mqtt.connected = 0;
        g_mqtt.st = MQTT_ST_IDLE;
    }

    return HAL_OK;
}

// ===== ���������� � ����� =====
HAL_StatusTypeDef MQTT_PublishSaveData(const char *save_data, uint32_t timeout_ms)
{
    char topic[96];
    // ����� = username = Version (��� ������)
    snprintf(topic, sizeof(topic), TOPIC_PUB1_FMT, g_mqtt.cred.username);
    HAL_StatusTypeDef status = MQTT_Publish(topic, save_data, MQTT_QOS_PUB, MQTT_RETAIN, timeout_ms);
    if (status != HAL_OK) {
        ERRCODE.STATUS |= STATUS_MQTT_PUB_ERROR;
    }
    else{
        ERRCODE.STATUS &= ~STATUS_MQTT_PUB_ERROR;
    }

    return status;
}

// ===== ���������� ������ ������ � ����� �������� =====
HAL_StatusTypeDef MQTT_PublishEmptySecond(uint32_t timeout_ms)
{
    char topic[96];
    snprintf(topic, sizeof(topic), TOPIC_PUB2_FMT, g_mqtt.cred.username);
    return MQTT_Publish(topic, "", MQTT_QOS_PUB, MQTT_RETAIN, timeout_ms);
}

// ===== �������� =====
HAL_StatusTypeDef MQTT_Subscribe(const char *topic, int qos, uint32_t timeout_ms)
{
    if (!g_mqtt.connected) { ERRCODE.STATUS |= STATUS_MQTT_CONN_ERROR; return HAL_ERROR; }
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+MSUB=\"%s\",%d\r", topic, qos);
    if (SendCommandAndParse(cmd, waitForOKResponse, timeout_ms) != HAL_OK) {
        ERRCODE.STATUS |= STATUS_MQTT_SUB_ERROR;
        return HAL_ERROR;
    }
    // SUBACK ������ ���� �������� � ���� �� ������ � ���� ����� ������, ��������� ������ parseBuffer �����.
    return HAL_OK;
}

// ===== ���������� =====
HAL_StatusTypeDef MQTT_Disconnect(uint32_t timeout_ms)
{
    if (SendCommandAndParse("AT+MDISCONNECT\r", waitForOKResponse, timeout_ms) != HAL_OK) {
        return HAL_ERROR;
    }
    g_mqtt.connected = 0;
    g_mqtt.st = MQTT_ST_IDLE;
    return HAL_OK;
}
