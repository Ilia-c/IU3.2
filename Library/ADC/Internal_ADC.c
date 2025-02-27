#include "Internal_ADC.h"


void Read_ADC_Voltage(void)
{
    // Запускаем преобразование АЦП
    HAL_ADC_Start(&hadc1);
    
    // Ожидаем завершения преобразования с максимально допустимым таймаутом
    if(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
    {
        uint32_t rawValue = HAL_ADC_GetValue(&hadc1);
        IntADC.ADC_AKB_volts = (rawValue * 3.3f) / 4095.0f;
        IntADC.ADC_AKB_volts *= *IntADC.Colibrate_koeff;
        IntADC.ADC_AKB_Proc = (uint8_t)voltageToSOC(IntADC.ADC_AKB_volts);
        sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
        sprintf(IntADC.ADC_AKB_Proc_char, "%d", IntADC.ADC_AKB_Proc);
    }
}

void Read_ADC_Colibrate_24V(void)
{
    // Запускаем преобразование АЦП
    HAL_ADC_Start(&hadc1);
    // Ожидаем завершения преобразования с максимально допустимым таймаутом
    if(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
    {
        uint32_t rawValue = HAL_ADC_GetValue(&hadc1);
        IntADC.ADC_AKB_volts = (rawValue * 3.3f) / 4095.0f;
        *IntADC.Colibrate_koeff = 24.0f / IntADC.ADC_AKB_volts;
        sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
        EEPROM_SaveSettings(&EEPROM);
        if (!EEPROM_CheckDataValidity()){
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        }
    }
}

float voltageToSOC(float voltage) {
    // Clamp voltages outside the expected range.
    if (voltage <= 20.0f) {
        return 0.0f;   // 20.0V or below: fully discharged (0%)
    }
    if (voltage >= 28.8f) {
        return 100.0f; // 28.8V or above: fully charged (100%)
    }

    float soc;
    if (voltage < 24.0f) {
        // Between 20.0V and 24.0V -> approximately 0% to 10% SoC
        soc = (voltage - 20.0f) / (24.0f - 20.0f) * 10.0f;
    } else if (voltage < 25.6f) {
        // Between 24.0V and 25.6V -> ~10% to 20% SoC
        soc = 10.0f + (voltage - 24.0f) / (25.6f - 24.0f) * 10.0f;
    } else if (voltage < 26.4f) {
        // Between 25.6V and 26.4V -> ~20% to 70% SoC (flat plateau region)
        soc = 20.0f + (voltage - 25.6f) / (26.4f - 25.6f) * 50.0f;
    } else if (voltage < 26.8f) {
        // Between 26.4V and 26.8V -> ~70% to 90% SoC
        soc = 70.0f + (voltage - 26.4f) / (26.8f - 26.4f) * 20.0f;
    } else {
        // Between 26.8V and 28.8V -> ~90% to 100% SoC (near full charge)
        soc = 90.0f + (voltage - 26.8f) / (28.8f - 26.8f) * 10.0f;
    }

    return soc;
}