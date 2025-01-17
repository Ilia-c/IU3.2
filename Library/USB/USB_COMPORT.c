
#include "USB_COMPORT.h"

#define RX_BUFFER_SIZE 512
extern uint8_t UserRxBufferFS[RX_BUFFER_SIZE];
extern uint16_t UserRxLength;
extern UART_HandleTypeDef huart4;

void TrimCommand(char *command)
{
    // Удаляем только завершающий '\n' (если есть)
    int len = strlen(command);
    if (len > 0 && command[len - 1] == '\n')
    {
        command[len - 1] = '\0';
    }
}

uint8_t command_un = 0;
void USB_COM(void *argument)
{
    command_un = 0;
    // 1) Удаляем завершающие символы (только '\n')
    char *command = (char *)UserRxBufferFS;
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
        command_un = 1;
        // Передаем всю команду по UART4
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);

        // Формируем ответ для USB
        char response[128];
        snprintf(response, sizeof(response),"Command L651: %s", command);

        // Отправляем ответ
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
    }

    if (strncmp(command, "GSM+", 4) == 0)
    {
        command_un = 1;
        memmove(command, command + 4, strlen(command + 4) + 1);
        SendSomeCommandAndSetFlag();
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);

        // Формируем ответ для USB
        char response[128];
        snprintf(response, sizeof(response),"Command L651: %s", command);

        // Отправляем ответ
        CDC_Transmit_FS((uint8_t *)response, strlen(response));
    }

    if (command_un == 0) CDC_Transmit_FS((uint8_t *)"Invalid command prefix\r\n", 25);
    memset(UserRxBufferFS, 0, RX_BUFFER_SIZE);

}





extern Prgramm_version_item Prog_ver;
void USB_Send_Status_Report(void)
{
    char buffer[128]; // Локальный буфер для передачи данных
    int len = 0;      // Текущая длина строки

    // Отправляем данные о GSM
    len = snprintf(buffer, sizeof(buffer), "GSM Status: %s\r\n", GSM_data.GSM_status_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "GSM SIM Card: %s\r\n", GSM_data.GSM_SIMCARD_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "GSM Ready: %s\r\n", GSM_data.GSM_status_ready_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "GSM Registered: %s\r\n", GSM_data.GSM_status_reg_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "GSM Operator: %s\r\n", GSM_data.GSM_operator_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "GSM Signal Level: %d\r\n", GSM_data.GSM_Signal_Level);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    // Отправляем данные о ADC
    len = snprintf(buffer, sizeof(buffer), "ADC Value: %d\r\n", ADC_data.ADC_value);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "ADC Status: %s\r\n", ADC_data.ADC_status_char);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    // Отправляем данные о версии программы
    len = snprintf(buffer, sizeof(buffer), "Program Version: %s\r\n", Prog_ver.VERSION_PROGRAMM);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "PCB Version: %s\r\n", Prog_ver.VERSION_PCB);
    CDC_Transmit_FS((uint8_t *)buffer, len);

    len = snprintf(buffer, sizeof(buffer), "Time Worked: %lu h, %lu m\r\n",
                   Prog_ver.time_work_h, Prog_ver.time_work_m);
    CDC_Transmit_FS((uint8_t *)buffer, len);
}
