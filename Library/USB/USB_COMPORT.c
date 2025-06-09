
#include "USB_COMPORT.h"

#define RX_BUFFER_SIZE 512
extern uint8_t UserRxBufferFS[RX_BUFFER_SIZE];
extern uint16_t UserRxLength;
extern UART_HandleTypeDef huart4;
extern EEPROM_Settings_item EEPROM;

void TrimCommand(char *command)
{
    // Удаляем только завершающий '\n' (если есть)
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
    { "DEBUG_GSM",          DEBUG_GSM,          "DEBUG_GSM включен",          "DEBUG_GSM выключен"          },
    { "AT_COMMANDS",        AT_COMMANDS,        "AT_COMMANDS включен",        "AT_COMMANDS выключен" },
    { "DEBUG_RS485",        DEBUG_RS485,        "DEBUG_RS485 включен",        "DEBUG_RS485 выключен"        },
    { "DEBUG_ADC",          DEBUG_ADC,          "DEBUG_ADC включен",          "DEBUG_ADC выключен"          },
    { "DEBUG_ADC_IN",       DEBUG_ADC_IN,       "DEBUG_ADC_IN включен",       "DEBUG_ADC_IN выключен"       },
    { "DEBUG_FLASH",        DEBUG_FLASH,        "DEBUG_FLASH включен",        "DEBUG_FLASH выключен"        },
    { "DEBUG_EEPROM",       DEBUG_EEPROM,       "DEBUG_EEPROM включен",       "DEBUG_EEPROM выключен"       },
    { "DEBUG_OTHER",        DEBUG_OTHER,        "DEBUG_OTHER включен",        "DEBUG_OTHER выключен"        }
};
#define DEBUG_CMD_COUNT  (sizeof(debugCmds) / sizeof(debugCmds[0]))

void DEBUG_USB(void)
{
    g_myRxCount = 0;
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);

    // Обработка DEBUG_* команд
    for (size_t i = 0; i < DEBUG_CMD_COUNT; ++i)
    {
        const DebugCmd_t *d = &debugCmds[i];
        size_t cmdLen = strlen(d->cmd);
        if (strncmp(command, d->cmd, cmdLen) == 0)
        {
            // очистить только ту часть, которую заняла команда
            memset(command, 0, cmdLen + 1);

            // проверка и переключение бита
            uint8_t is_set = (EEPROM.DEBUG_CATEG & d->flag) ? 1 : 0;
            if (is_set)
                EEPROM.DEBUG_CATEG &= ~d->flag;
            else
                EEPROM.DEBUG_CATEG |= d->flag;

            // отправка ответа
            char resp[64];
            int len = snprintf(resp, sizeof(resp),
                               "------------ %s ------------\r\n",
                               is_set ? d->msg_off : d->msg_on);
            while (CDC_Transmit_FS((uint8_t *)resp, len) == USBD_BUSY){}
            return;
        }
    }

    // Установка уровня отладки: цифра 1–5
    if (strlen(command) <= 2 && command[0] >= '1' && command[0] <= '5')
    {
        uint8_t lvl = command[0] - '0';
        EEPROM.DEBUG_LEVL = lvl - 1;
        char resp[64];
        int len = snprintf(resp, sizeof(resp), "------------ Новый уровень отладки: %u ------------\r\n", lvl);
        while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        command[0] = '\0';  // очистили команду
        return;
    }

    // Сохранение настроек
    if (strncmp(command, "SAVE", 4) == 0)
    {
        // очистить "SAVE"
        memset(command, 0, strlen(command));

        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }

        const char resp[] = "------------ СОХРАНЕНО ------------\r\n";
        while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        return;
    }

    if (strncmp(command, "ALL_ON", 6) == 0)
    {
        // очистить "SAVE"
        memset(command, 0, strlen(command));
        EEPROM.DEBUG_CATEG = 0xFF; // Включаем все категории отладки

        const char resp[] = "------------ Все включено ------------\r\n";
       while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        return;
    }
    if (strncmp(command, "ALL_OFF", 7) == 0)
    {
        // очистить "SAVE"
        memset(command, 0, strlen(command));

        EEPROM.DEBUG_CATEG = 0x00; // Выключаем все категории отладки
        const char resp[] = "------------ Все выключено ------------\r\n";
        while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        return;
    }
    if (strncmp(command, "Help", 4) == 0)
    {
        // очистить "SAVE"
        memset(command, 0, strlen(command));

        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK)
        {
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }

        const char resp[] = "------------ HELP ------------\r\n";
        const char help[] =
            "1. DEBUG_GSM\n"
            "2. AT_COMMANDS - отладка AT-команд\n"
            "3. DEBUG_RS485 - отладка RS-485\n"
            "4. DEBUG_ADC - отладка АЦП\n"
            "5. DEBUG_ADC_IN - отладка АЦП мк\n"
            "6. DEBUG_FLASH - отладка FLASH\n"
            "7. DEBUG_EEPROM - отладка EEPROM\n"
            "8. DEBUG_OTHER - отладка других модулей\n"
            "9. SAVE - сохранить настройки\n"
            "10. ALL_ON - включить все отладочные категории\n"
            "11. ALL_OFF - выключить все отладочные категории\n"
            "12. Help - показать это сообщение\r\n";
        while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        while (CDC_Transmit_FS((uint8_t *)help, strlen(help)) == USBD_BUSY){}
        while (CDC_Transmit_FS((uint8_t *)resp, strlen(resp)) == USBD_BUSY){}
        return;
    }
    //memset(command, 0, 512);
}

void AT_SEND()
{
    command_un = 0;
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);
    if (strncmp(command, "AT", 2) == 0)
    {
        char response[512];
        // Отправляем ответ
        snprintf(response, sizeof(response), "Command L651: %s", command);
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
        command_un = 1;
        // Передаем всю команду по UART4
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
    }

    if (strncmp(command, "GSM+", 4) == 0)
    {
        command_un = 1;
        memmove(command, command + 4, strlen(command + 4) + 1);
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);

        // Формируем ответ для USB
        char response[512];
        snprintf(response, sizeof(response), "Command L651: %s", command);

        // Отправляем ответ
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
    }

    if (command_un == 0)
        CDC_Transmit_FS((uint8_t *)"Invalid command prefix\r\n", 25);
    memset(UserRxBufferFS, 0, RX_BUFFER_SIZE);
    memset(g_myRxBuffer, 0, RX_BUFFER_SIZE);
}
