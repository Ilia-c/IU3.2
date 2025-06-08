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

#define ADC_3_BUF_LEN   3

extern EEPROM_Settings_item EEPROM;
extern Internal_ADC_item IntADC;
extern uint16_t adc1_buffer[ADC_3_BUF_LEN];
void ADC_Voltage_Calculate(void);
int Read_ADC_Colibrate_24V(void);
float voltageToSOC(float voltage);
void ADC_Start(void);
float Convert_Temperature(uint16_t raw_ts);
float Convert_Voltage(uint16_t raw_vbat);

#ifdef __cplusplus
}
#endif

#endif // SETTINGS_H