#include "w25q128.h"

#define W25_ENABLE_RESET 0x66
#define W25_RESET 0x99
#define W25_READ 0x03
#define W25_GET_JEDEC_ID 0x9f

#define cs_set() HAL_GPIO_WritePin(RESERVED_GPIO_Port,RESERVED_Pin, 0)
#define cs_reset() HAL_GPIO_WritePin(RESERVED_GPIO_Port,RESERVED_Pin, 1)
uint8_t rx_buf[1025];
uint8_t tx_buf[10];

extern SPI_HandleTypeDef hspi1;

void SPI1_Send(uint8_t *dt, uint16_t cnt)
{
    HAL_SPI_Transmit(&hspi1, dt, cnt, 1000);
}
//-------------------------------------------------------------
void SPI1_Recv(uint8_t *dt, uint16_t cnt)
{
    HAL_SPI_Receive(&hspi1, dt, cnt, 1000);
}

void W25_Reset(void)
{
    cs_set();
    HAL_Delay(2);
    tx_buf[0] = W25_ENABLE_RESET;
    tx_buf[1] = W25_RESET;
    SPI1_Send(tx_buf, 2);
    cs_reset();
}

void W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz)
{
    cs_set();
    HAL_Delay(2);
    tx_buf[0] = W25_READ;
    tx_buf[1] = (addr >> 16) & 0xFF;
    tx_buf[2] = (addr >> 8) & 0xFF;
    tx_buf[3] = addr & 0xFF;
    SPI1_Send(tx_buf, 4);
    SPI1_Recv(data, sz);
    cs_reset();
}

void W25_Ini(void)
{
    W25_Reset();
    HAL_Delay(100);
    unsigned int id = W25_Read_ID();
}

uint32_t W25_Read_ID(void)
{
    uint8_t dt[4];
    tx_buf[0] = W25_GET_JEDEC_ID;
    cs_set();
    HAL_Delay(2);
    SPI1_Send(tx_buf, 1);
    SPI1_Recv(dt, 3);
    cs_reset();
    return (dt[0] << 16) | (dt[1] << 8) | dt[2];
}