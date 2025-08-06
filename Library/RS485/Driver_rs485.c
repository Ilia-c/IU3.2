/* Driver_rs485.c */
#include "Driver_rs485.h"

extern UART_HandleTypeDef huart5;  // из main.c, сгенерированного CubeMX

static uint8_t rs485_rx_buf[RS485_RX_BUFFER_SIZE];
static volatile uint16_t rs485_rx_head = 0;
static volatile uint16_t rs485_rx_tail = 0;
uint8_t  rs485_rx_byte;



void RS485_StartReceive_IT(void)
{
    HAL_UART_Receive_IT(&huart5, &rs485_rx_byte, 1);
}

HAL_StatusTypeDef RS485_Transmit_IT(uint8_t *pData, uint16_t Size)
{
    return HAL_UART_Transmit_IT(&huart5, pData, Size);
}

uint16_t RS485_Available(void)
{
    if (rs485_rx_head >= rs485_rx_tail)
        return rs485_rx_head - rs485_rx_tail;
    else
        return RS485_RX_BUFFER_SIZE - rs485_rx_tail + rs485_rx_head;
}

int RS485_ReadByte(uint8_t *pByte)
{
    if (rs485_rx_head == rs485_rx_tail) {
        return 0;
    }
    *pByte = rs485_rx_buf[rs485_rx_tail++];
    if (rs485_rx_tail >= RS485_RX_BUFFER_SIZE)
        rs485_rx_tail = 0;
    return 1;
}


__weak void RS485_TxCpltCallback(void)
{
}

__weak void RS485_RxCpltCallback(uint8_t byte)
{
    rs485_rx_buf[rs485_rx_head++] = byte;
    if (rs485_rx_head >= RS485_RX_BUFFER_SIZE)
        rs485_rx_head = 0;
}

/* === Вызовы из HAL === */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART5) {
        RS485_TxCpltCallback();
    }
}