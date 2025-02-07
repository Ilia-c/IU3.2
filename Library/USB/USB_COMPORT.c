
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
uint8_t command_un = 0;
void USB_COM(void)
{
    command_un = 0;
    char *command = (char *)g_myRxBuffer;
    TrimCommand(command);

    // 2) Проверяем «STM+»
    if (strncmp(command, "STM+", 4) == 0)
    {
        command_un = 1;
        const char *cmd = &command[4];

        if (strstr(cmd, "STATUS") != NULL)
        {
            USB_Send_Status_Report();
        }
        else if (strstr(cmd, "HELLO") != NULL)
        {
            CDC_Transmit_FS((uint8_t *)"Hello from STM32!\r\n", 32);
        }
        else if (strstr(cmd, "RSTGSM") != NULL)
        {
            CDC_Transmit_FS((uint8_t *)"START RESET L651\r\n", 32);
            HAL_GPIO_WritePin(ON_N25_GPIO_Port, ON_N25_Pin, 0);
            HAL_Delay(100);
            HAL_GPIO_WritePin(ON_N25_GPIO_Port, ON_N25_Pin, 1);
            HAL_Delay(100);
            HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 1);
            HAL_Delay(600);
            HAL_GPIO_WritePin(UART4_WU_GPIO_Port, UART4_WU_Pin, 0);
            CDC_Transmit_FS((uint8_t *)"FINISH RESET L651\r\n", 32);
        }
        else
        {
            CDC_Transmit_FS((uint8_t *)"Unknown command\r\n", 17);
        }
    }

    if (strncmp(command, "AT", 2) == 0)
    {
        char response[512];
                // Отправляем ответ
        snprintf(response, sizeof(response),"Command L651: %s", command);
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
        snprintf(response, sizeof(response),"Command L651: %s", command);

        // Отправляем ответ
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
    }

    if (command_un == 0) CDC_Transmit_FS((uint8_t *)"Invalid command prefix\r\n", 25);
    memset(UserRxBufferFS, 0, RX_BUFFER_SIZE);
    memset(g_myRxBuffer, 0, RX_BUFFER_SIZE);


}

void USB_Send_Status_Report(void)
{
}
