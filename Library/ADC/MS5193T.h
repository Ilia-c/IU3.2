#include "main.h"
#include "Settings.h"

uint8_t SPI2_TransmitByte(uint8_t TxData);
uint8_t SPI2_Read_OneByte(uint8_t reg);
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len);
void SPI2_Write_buf(uint8_t reg, uint8_t *pbuf, uint8_t len);
void MS5193T_Reset(void);
void MS5193T_Init(void);
int32_t Read_MS5193T_Data(void);
void calculate_ADC_data_temp(int32_t adValue);
void calculate_ADC_data_heigh(int32_t adValue);