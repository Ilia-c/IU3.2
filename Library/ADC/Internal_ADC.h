#ifndef INTERNAL_ADC_H
#define INTERNAL_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include "Settings.h"


extern EEPROM_Settings_item EEPROM;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern Internal_ADC_item IntADC;
void Read_ADC_Voltage(void);
int Read_ADC_Colibrate_24V(void);
float voltageToSOC(float voltage);

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H