#include <stdio.h>
#include <string.h>
#include "main.h"
#include "Settings.h"
#include "Status_codes.h"

void SPI1_Send(uint8_t *dt, uint16_t cnt);
void SPI1_Recv(uint8_t *dt, uint16_t cnt);
void W25_Reset(void);
void W25_Read_Data(uint32_t addr, uint8_t *data, uint32_t sz);
void W25_Ini(void);
uint32_t W25_Read_ID(void);