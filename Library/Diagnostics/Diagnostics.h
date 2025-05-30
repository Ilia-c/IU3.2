#include "main.h"
#include "Settings.h"
#include "AT24C02.h"
#include "fatfs.h"
#include "OLED.h"
#include "w25q128.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern SD_HandleTypeDef hsd1;
extern SPI_HandleTypeDef hspi2;

void Diagnostics();
HAL_StatusTypeDef EEPROM_CHECK();
