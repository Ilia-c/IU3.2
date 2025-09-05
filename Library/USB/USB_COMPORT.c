
#include "USB_COMPORT.h"

#define RX_BUFFER_SIZE 512
extern uint8_t UserRxBufferFS[RX_BUFFER_SIZE];
extern uint16_t UserRxLength;
extern UART_HandleTypeDef huart4;



void TrimCommand(char *command)
{
    // ������� ������ ����������� '\n' (���� ����)
    int len = strlen(command);
    if (len > 0 && command[len - 1] == '\n')
    {
        command[len - 1] = '\0';
    }
}
extern uint8_t g_myRxBuffer[MY_USB_RX_BUFFER_SIZE]; 
extern uint8_t UserRxBufferFS[MY_USB_RX_BUFFER_SIZE];
extern uint16_t g_myRxCount;
void USB_COM(void)
{
    if (EEPROM.USB_mode != USB_DEBUG) return;
    if (EEPROM.DEBUG_Mode == USB_SNIFFING){
        DEBUG_USB();
    }

    if (EEPROM.DEBUG_Mode == USB_AT_DEBUG){
        AT_SEND();
    }
}
typedef struct {
    const char *cmd;
    uint32_t     flag;
    const char *msg_on;
    const char *msg_off;
} DebugCmd_t;

static const DebugCmd_t debugCmds[] = {
    { "DEBUG_GSM",          DEBUG_GSM,          "DEBUG_GSM ON",          "DEBUG_GSM OFF"          },
    { "AT_COMMANDS",        AT_COMMANDS,        "AT_COMMANDS ON",        "AT_COMMANDS OFF"        },
    { "DEBUG_RS485",        DEBUG_RS485,        "DEBUG_RS485 ON",        "DEBUG_RS485 OFF"        },
    { "DEBUG_ADC",          DEBUG_ADC,          "DEBUG_ADC ON",          "DEBUG_ADC OFF"          },
    { "DEBUG_ADC_IN",       DEBUG_ADC_IN,       "DEBUG_ADC_IN ON",       "DEBUG_ADC_IN OFF"       },
    { "DEBUG_FLASH",        DEBUG_FLASH,        "DEBUG_FLASH ON",        "DEBUG_FLASH OFF"        },
    { "DEBUG_EEPROM",       DEBUG_EEPROM,       "DEBUG_EEPROM ON",       "DEBUG_EEPROM OFF"       },
    { "DEBUG_OTHER",        DEBUG_OTHER,        "DEBUG_OTHER ON",        "DEBUG_OTHER OFF"        }
};
#define DEBUG_CMD_COUNT  (sizeof(debugCmds) / sizeof(debugCmds[0]))

void DEBUG_USB(void)
{
    g_myRxCount = 0;
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);

    // ��������� DEBUG_* ������
    for (size_t i = 0; i < DEBUG_CMD_COUNT; ++i)
    {
        const DebugCmd_t *d = &debugCmds[i];
        size_t cmdLen = strlen(d->cmd);
        if (strncmp(command, d->cmd, cmdLen) == 0)
        {
            // �������� ������ �� �����, ������� ������ �������
            memset(command, 0, cmdLen + 1);

            // �������� � ������������ ����
            uint8_t is_set = (EEPROM.DEBUG_CATEG & d->flag) ? 1 : 0;
            if (is_set)
                EEPROM.DEBUG_CATEG &= ~d->flag;
            else
                EEPROM.DEBUG_CATEG |= d->flag;

            // �������� ������
            char resp[64];
            int len = snprintf(resp, sizeof(resp),
                               "------------ %s ------------\r\n",
                               is_set ? d->msg_off : d->msg_on);
            collect_message(resp);                 
            //while (CDC_Transmit_FS((uint8_t *)resp, len) == USBD_BUSY){}
            return;
        }
    }

    // ��������� ������ �������: ����� 1�5
    if (strlen(command) <= 2 && command[0] >= '1' && command[0] <= '5')
    {
        uint8_t lvl = command[0] - '0';
        EEPROM.DEBUG_LEVL = lvl - 1;
        char resp[64];
        int len = snprintf(resp, sizeof(resp), "------------ ����� ������� �������: %u ------------\r\n", lvl);
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        command[0] = '\0';  // �������� �������
        return;
    }

    // ���������� ��������
    if (strncmp(command, "SAVE", 4) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));

        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }

        const char resp[] = "------------ ��������� ------------\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        return;
    }

    if (strncmp(command, "ALL_ON", 6) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));
        EEPROM.DEBUG_CATEG = 0xFF; // �������� ��� ��������� �������

        const char resp[] = "------------ ��� ON ------------\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        return;
    }
    if (strncmp(command, "ALL_OFF", 7) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));

        EEPROM.DEBUG_CATEG = 0x00; // ��������� ��� ��������� �������
        const char resp[] = "------------ ��� OFF ------------\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        return;
    }
    if (strncmp(command, "Help", 4) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));

        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }

        const char resp[] = "------------ HELP ------------\r\n";
        const char help_1[] =
            "1. DEBUG_GSM\n"
            "2. AT_COMMANDS - ������� AT-������\n";
        const char help_2[] =
            "3. DEBUG_RS485 - ������� RS-485\n"
            "4. DEBUG_ADC - ������� ���\n";
        const char help_3[] =
            "5. DEBUG_ADC_IN - ������� ��� ��\n"
            "6. DEBUG_FLASH - ������� FLASH\n";
        const char help_4[] =
            "7. DEBUG_EEPROM - ������� EEPROM\n"
            "8. DEBUG_OTHER - ������� ������ �������\n";
        const char help_5[] =
            "9. SAVE - ��������� ���������\n"
            "10. ALL_ON - �������� ��� ���������� ���������\n";
        const char help_6[] =
            "11. ALL_OFF - ��������� ��� ���������� ���������\n"
            "12. AT_MANUAL_ON - ������ �������� � ������� ����� ��������\n";
        const char help_7[] =
            "13. AT_MANUAL_OFF - ������ �������� � ������� ����� ���������\n"
            "14. Help - �������� ��� ���������\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        //while (CDC_Transmit_FS((uint8_t *)help, strlen(help)) == USBD_BUSY){}
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        collect_message(help_1); 
        collect_message(help_2); 
        collect_message(help_3); 
        collect_message(help_4); 
        collect_message(help_5); 
        collect_message(help_6); 
        collect_message(help_7); 
        collect_message(resp); 
        return;
    }
    if (strncmp(command, "AT_MANUAL_ON", 12) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));
        EEPROM.DEBUG_Mode = USB_AT_DEBUG;
        
        const char resp[] = "------------ ����� AT Manual ON ------------\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        return;
    }
    memset(command, 0, 512);
}

void AT_SEND()
{
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);
    if (strncmp(command, "AT_MANUAL_OFF", 13) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, strlen(command));
        EEPROM.DEBUG_Mode = USB_SNIFFING;
        
        const char resp[] = "------------ ����� AT Manual OFF ------------\r\n";
        //while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        collect_message(resp); 
        return;
    }
    char response[512];
    // ���������� �����
    snprintf(response, sizeof(response), "Command L651: %s", command);
    //CDC_Transmit_FS((uint8_t *)response, strlen(response));
    collect_message(response); 
    // �������� ��� ������� �� UART4
    SendSomeCommandAndSetFlag();
    HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    
    //memset(UserRxBufferFS, 0, RX_BUFFER_SIZE);
    memset(g_myRxBuffer, 0, RX_BUFFER_SIZE);
}



// �������� ��������� �� ���������� ������
// ----- ��������� ����� -----
uint8_t  s_usbLogRing[USB_LOG_RING_SZ];
uint32_t s_head = 0;  // ���� ������
uint32_t s_tail = 0;  // ������ �����
uint32_t s_dropped_bytes = 0; // ������� ���� �� �������� ��-�� ������������

static inline uint32_t ring_used(void) {
    return (uint32_t)(s_head - s_tail);
}
static inline uint32_t ring_free(void) {
    return USB_LOG_RING_SZ - ring_used();
}


// ������ � ������ ������ ����������
HAL_StatusTypeDef ring_write(const uint8_t *src, uint32_t len)
{
    // 1) ������ ����������� ����� � ������� head (���������� ����. �������)
    uint32_t pos;
    taskENTER_CRITICAL();
    if (len > ring_free()) {                 // ring_free() = SIZE - (head - tail)
        s_dropped_bytes += len;
        taskEXIT_CRITICAL();
        return HAL_ERROR;
    }
    pos = s_head;                            // ���� ������
    s_head += len;                           // ���������������
    taskEXIT_CRITICAL();

    // 2) �������� �������� ��� ��� critical (USB IRQ �� ��������)
    uint32_t idx = pos & USB_LOG_RING_MASK;
    uint32_t n1  = USB_LOG_RING_SZ - idx;
    if (n1 > len) n1 = len;
    memcpy(&s_usbLogRing[idx], src, n1);
    if (len > n1) memcpy(&s_usbLogRing[0], src + n1, len - n1);

    // 3) ��������� TX-������: ������� ����� ������
    xSemaphoreGive(USB_COM_TX_semaphore);
    return HAL_OK;
}

HAL_StatusTypeDef USB_LogService(TickType_t tx_done_timeout)
{
    // ��� ������ � ������ ������
    if (ring_used() == 0) return HAL_ERROR;

    // USB �� ��������������� � �� ��������, ������ �������
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) return HAL_ERROR;

    // ����� ������ head / ��������� �������� �����
    uint32_t head_snapshot;
    uint32_t rd, linear, chunk;
    uint8_t *ptr;

    taskENTER_CRITICAL();
    head_snapshot = s_head;
    if (head_snapshot == s_tail) { taskEXIT_CRITICAL(); return HAL_ERROR; }

    rd     = s_tail & USB_LOG_RING_MASK;
    linear = USB_LOG_RING_SZ - rd;
    uint32_t used = head_snapshot - s_tail;
    if (linear > used) linear = used;

    chunk = linear;
    if (chunk > USB_TX_CHUNK_MAX) chunk = USB_TX_CHUNK_MAX;

    // �������� ���� �� 64, ���� ������ 64; ���� ������ 64 � ��� ��� ����
    if (chunk > USB_FS_MPS) {
        chunk -= (chunk % USB_FS_MPS);
        if (chunk == 0) chunk = USB_FS_MPS;
    }
    ptr = &s_usbLogRing[rd];
    taskEXIT_CRITICAL();

    // ��������� �������� (��� ����-������)
    // ��������� ������: USBD_OK, USBD_BUSY, USBD_FAIL
    TickType_t t_start = xTaskGetTickCount();

    for (;;) {
        uint8_t rc = CDC_Transmit_FS(ptr, chunk);
        if (rc == USBD_OK) break;                       // �������
        if (rc == USBD_FAIL || hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED)
            return HAL_ERROR;                               // USB ������/������ � �������
        // BUSY: ��� ���������� ����������� TX, �� �� ����������
        if (xSemaphoreTake(USB_COM_TX_DONE_semaphore, pdMS_TO_TICKS(50)) != pdTRUE) {
            if ((xTaskGetTickCount() - t_start) >= tx_done_timeout) return HAL_ERROR;
        }
    }

    // ��������� ���������� ����� ��������
    if (xSemaphoreTake(USB_COM_TX_DONE_semaphore, tx_done_timeout) != pdTRUE) {
        // ��� ������� � �� ��������; ��������� �����
        return HAL_ERROR;
    }

    // �������� ����� �� ������� ������������
    taskENTER_CRITICAL();
    s_tail += chunk;
    taskEXIT_CRITICAL();

    return HAL_OK; // ��������� �����
}


