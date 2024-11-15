#include "main.h"

uint8_t SPI2_TransmitByte(uint8_t TxData);
uint8_t SPI2_Read_OneByte(uint8_t reg);
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len);
void SPI2_Write_buf(uint8_t reg, uint8_t *pbuf, uint8_t len);
void AD7793_Reset(void);
void AD7793_Init(void);
int32_t Read_AD7793_Data(void);
void ToBin(int32_t xValue);