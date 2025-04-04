#include "Internal_ADC.h"

const float alpha = 0.8f;
static uint8_t initialized = 0;
void Read_ADC_Voltage(void)
{
    HAL_ADC_Start(&hadc1);
    // ������� ���������� �������������� (���������� �� ���������)
    if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        uint32_t rawValue = HAL_ADC_GetValue(&hadc1);
        float voltage = (rawValue * 3.3f) / 4095.0f;
        voltage *= *IntADC.Colibrate_koeff;
        if(!initialized)
        {
            IntADC.ADC_AKB_volts = voltage;
            initialized = 1;
        }
        else
        {
            IntADC.ADC_AKB_volts = alpha * voltage + (1.0f - alpha) * IntADC.ADC_AKB_volts;
        }
        if (IntADC.ADC_AKB_volts<LOW_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_LOW;
        else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_LOW;
        if (IntADC.ADC_AKB_volts>HIGH_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_HIGH;
        else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_HIGH;
        
        IntADC.ADC_AKB_Proc = (uint8_t)voltageToSOC(IntADC.ADC_AKB_volts);
        sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
        sprintf(IntADC.ADC_AKB_Proc_char, "%d", IntADC.ADC_AKB_Proc);
    }
}

int Read_ADC_Colibrate_24V(void)
{
    // ��������� �������������� ���
    HAL_ADC_Start(&hadc1);
    // ������� ���������� �������������� � ����������� ���������� ���������
    if(HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK)
    {
        uint32_t rawValue = HAL_ADC_GetValue(&hadc1);
        IntADC.ADC_AKB_volts = (rawValue * 3.3f) / 4095.0f;
        *IntADC.Colibrate_koeff = 24.0f / IntADC.ADC_AKB_volts;
        sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
        EEPROM_SaveSettings(&EEPROM);
        if (EEPROM_CheckDataValidity() != HAL_OK){
            ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
            return -1;
        }
    }
    return 0;
}

float voltageToSOC(float voltage) {
    // Clamp voltages outside the expected range.
    if (voltage <= 22.4f) {
        return 0.0f;
    }
    if (voltage >= 27.2f) {
        return 100.0f;
    }

    float soc;
    if (voltage < 24.0f) {
        soc = (voltage - 22.4f) / (24.0f - 22.4f) * 10.0f;
    } else if (voltage < 25.6f) {
        soc = 10.0f + (voltage - 24.0f) / (25.6f - 24.0f) * 10.0f;
    } else if (voltage < 26.4f) {
        soc = 20.0f + (voltage - 25.6f) / (26.4f - 25.6f) * 50.0f;
    } else if (voltage < 26.8f) {
        soc = 70.0f + (voltage - 26.4f) / (26.8f - 26.4f) * 20.0f;
    } else {
        soc = 90.0f + (voltage - 26.8f) / (27.2f - 26.8f) * 10.0f;
    }

    return soc;
}