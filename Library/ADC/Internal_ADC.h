#ifndef INTERNAL_ADC_H
#define INTERNAL_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include "Settings.h"
#include "AT24C02.h"
#include "Status_codes.h"

extern EEPROM_Settings_item EEPROM;
extern Internal_ADC_item IntADC;
void ADC_Voltage_Calculate(void);
int Read_ADC_Colibrate_24V(void);
float voltageToSOC(float voltage);
void ADC_Start(void);

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H