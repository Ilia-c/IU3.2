#include "main.h"
#include "Settings.h"

extern EEPROM_Settings_item EEPROM;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern Internal_ADC_item IntADC;
void Read_ADC_Voltage(void);
void Read_ADC_Colibrate_24V(void);
float voltageToSOC(float voltage);