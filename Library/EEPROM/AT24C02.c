#include "AT24C02.h"

extern I2C_HandleTypeDef hi2c2;
extern ADC_MS5193T_item ADC_data;
extern GSM_STATUS_item GSM_data;
extern EEPROM_Settings_item EEPROM;

// обновление символов char и разбиение на части
void Data_UPDATE_char(){
    ADC_data.MAX_LVL_char[0] = (int32_t*)ADC_data.MAX_LVL;

    // Выделяем дробную часть и масштабируем её в целое число
    double fractionalPart = *ADC_data.MAX_LVL - ADC_data.MAX_LVL_char[0];
    ADC_data.MAX_LVL_char[1] = (int32_t)(fractionalPart * 1e6); // Сохраняем 6 знаков после запя
}