
#include "USB_COMPORT.h"

#define RX_BUFFER_SIZE 512
extern uint8_t UserRxBufferFS[RX_BUFFER_SIZE];
extern uint16_t UserRxLength;
extern UART_HandleTypeDef huart4;
extern EEPROM_Settings_item EEPROM;

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
uint8_t command_un = 0;
void USB_COM(void)
{
    if (EEPROM.USB_mode == USB_DEBUG){
        DEBUG_USB();
    }

    if (EEPROM.USB_mode == USB_AT){
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
    { "DEBUG_GSM",          DEBUG_GSM,          "DEBUG_GSM �������",          "DEBUG_GSM ��������"          },
    { "AT_COMMANDS",        AT_COMMANDS,        "AT_COMMANDS �������",        "AT_COMMANDS ��������" },
    { "DEBUG_RS485",        DEBUG_RS485,        "DEBUG_RS485 �������",        "DEBUG_RS485 ��������"        },
    { "DEBUG_ADC",          DEBUG_ADC,          "DEBUG_ADC �������",          "DEBUG_ADC ��������"          },
    { "DEBUG_ADC_IN",       DEBUG_ADC_IN,       "DEBUG_ADC_IN �������",       "DEBUG_ADC_IN ��������"       },
    { "DEBUG_FLASH",        DEBUG_FLASH,        "DEBUG_FLASH �������",        "DEBUG_FLASH ��������"        },
    { "DEBUG_EEPROM",       DEBUG_EEPROM,       "DEBUG_EEPROM �������",       "DEBUG_EEPROM ��������"       },
    { "DEBUG_OTHER",        DEBUG_OTHER,        "DEBUG_OTHER �������",        "DEBUG_OTHER ��������"        },
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
            CDC_Transmit_FS((uint8_t *)resp, len);
            return;
        }
    }

    // ��������� ������ �������: ����� 1�5
    if (strlen(command) <= 2 && command[0] >= '1' && command[0] <= '5')
    {
        uint8_t lvl = command[0] - '0';
        EEPROM.DEBUG_LEVL = lvl - 1;

        char resp[64];
        int len = snprintf(resp, sizeof(resp),
                           "------------ ����� ������� �������: %u ------------\r\n",
                           lvl);
        CDC_Transmit_FS((uint8_t *)resp, len);

        command[0] = '\0';  // �������� �������
        return;
    }

    // ���������� ��������
    if (strncmp(command, "SAVE", 4) == 0)
    {
        // �������� "SAVE"
        memset(command, 0, 5);

        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }

        const char resp[] = "------------ ��������� ------------\r\n";
        CDC_Transmit_FS((uint8_t *)resp, strlen(resp));
        return;
    }
    
}

void AT_SEND()
{
    command_un = 0;
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);
    if (strncmp(command, "AT", 2) == 0)
    {
        char response[512];
        // ���������� �����
        snprintf(response, sizeof(response), "Command L651: %s", command);
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
        command_un = 1;
        // �������� ��� ������� �� UART4
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    }

    if (strncmp(command, "GSM+", 4) == 0)
    {
        command_un = 1;
        memmove(command, command + 4, strlen(command + 4) + 1);
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);

        // ��������� ����� ��� USB
        char response[512];
        snprintf(response, sizeof(response), "Command L651: %s", command);

        // ���������� �����
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
    }

    if (command_un == 0)
        CDC_Transmit_FS((uint8_t *)"Invalid command prefix\r\n", 25);
    memset(UserRxBufferFS, 0, RX_BUFFER_SIZE);
    memset(g_myRxBuffer, 0, RX_BUFFER_SIZE);
}
