
#include "USB_COMPORT.h"

#define RX_BUFFER_SIZE 512
extern uint8_t UserRxBufferFS[RX_BUFFER_SIZE];
extern uint16_t UserRxLength;
extern UART_HandleTypeDef huart4;

void TrimCommand(char *command)
{
    char *end = command + strlen(command) - 1;

    while (end >= command && (*end == '\r' || *end == '\n'))
    {
        *end = '\0';
        end--;
    }
}

void USB_COM(void *argument)
{
    // Удаляем завершающие символы (\r и \n)
    char *command = (char *)UserRxBufferFS;
    TrimCommand(command);

    // Проверяем префикс команды
    if (strncmp(command, "STM+", 4) == 0)
    {
        const char *cmd = &command[4];

        if (strcmp(cmd, "STATUS") == 0)
        {
            USB_Send_Status_Report();
            return;
        }
        else if (strcmp(cmd, "HELLO") == 0)
        {
            CDC_Transmit_FS((uint8_t *)"Hello from STM32!\r\n", 20);
            return;
        }

        CDC_Transmit_FS((uint8_t *)"Unknown command\r\n", 17);
        return;
    }
    if (strncmp(command, "AT", 2) == 0)
    {
        // Пересылка команды AT+ через UART4
        HAL_UART_Transmit(&huart4, (uint8_t *)command, strlen(command), HAL_MAX_DELAY);
        CDC_Transmit_FS((uint8_t *)"Command sent via UART4\r\n", 24);
        return;
    }
    CDC_Transmit_FS((uint8_t *)"Invalid command prefix\r\n", 25);
    return;
    // Очищаем буфер
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
