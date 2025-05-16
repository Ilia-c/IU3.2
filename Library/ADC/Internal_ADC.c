#include "Internal_ADC.h"

const float alpha = 0.7f;
static uint8_t initialized = 0;

ADC_HandleTypeDef    hadc1;
DMA_HandleTypeDef    hdma_adc1;
#define ADC_BUFFER_LEN  256 // Размер буфера для DMA
uint16_t adc_buffer[ADC_BUFFER_LEN];

static int cmp_uint16(const void *p1, const void *p2)
{
    uint16_t a = *(const uint16_t*)p1;
    uint16_t b = *(const uint16_t*)p2;
    if (a < b) return -1;
    if (a > b) return  1;
    return 0;
}
float TruncatedMeanVoltage()
{
    float discard_ratio = 0.1f; // Отбросить 10% крайних значений
    if (discard_ratio < 0.0f || discard_ratio >= 0.5f)
        return 0.0f;

    // 1) Скопировать буфер
    uint16_t tmp[ADC_BUFFER_LEN];
    memcpy(tmp, adc_buffer, ADC_BUFFER_LEN * sizeof(uint16_t));

    // 2) Отсортировать копию с помощью собственного компаратора
    qsort(tmp, ADC_BUFFER_LEN, sizeof(uint16_t), cmp_uint16);

    // 3) Вычислить индексы отброса
    size_t d     = (size_t)(discard_ratio * ADC_BUFFER_LEN);
    size_t start = d;
    size_t end   = ADC_BUFFER_LEN - d;

    // 4) Усреднить оставшиеся
    uint32_t sum = 0;
    for (size_t i = start; i < end; ++i) {
        sum += tmp[i];
    }
    float avg_counts = (float)sum / (float)(end - start);

    // 5) Перевести в напряжение
    return (avg_counts * 3.3f) / 1024.0f;
}

void ADC_Start(void)
{
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
        Error_Handler();
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, ADC_BUFFER_LEN) != HAL_OK)
        Error_Handler();
}

void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_adc1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) { 
        //if (HAL_GPIO_ReadPin(ON_OWEN_GPIO_Port, ON_OWEN_Pin) == 0) HAL_ADC_Stop_DMA(&hadc1);
    }
}

void ADC_Voltage_Calculate(void)
{
    
    IntADC.ADC_AKB_volts = TruncatedMeanVoltage();
    IntADC.ADC_AKB_volts *= *IntADC.Colibrate_koeff;
    
    if (IntADC.ADC_AKB_volts < LOW_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_LOW;
    else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_LOW;
    if (IntADC.ADC_AKB_volts > HIGH_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_HIGH;
    else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_HIGH;

    IntADC.ADC_AKB_Proc = (uint8_t)voltageToSOC(IntADC.ADC_AKB_volts);
    sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
    sprintf(IntADC.ADC_AKB_Proc_char, "%d", IntADC.ADC_AKB_Proc);
}

int Read_ADC_Colibrate_24V(void)
{
    *IntADC.Colibrate_koeff = 24.0f / IntADC.ADC_AKB_volts;
    sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
    EEPROM_SaveSettings(&EEPROM);
    if (EEPROM_CheckDataValidity() != HAL_OK){
        ERRCODE.STATUS |= STATUS_EEPROM_WRITE_ERROR;
        return -1;
    }
    return 0;
}

float voltageToSOC(float voltage) {
    // Clamp voltages outside the expected range.
    if (voltage <= 22.4f) {
        return 0.0f;
    }
    if (voltage >= 27.1f) {
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
        soc = 90.0f + (voltage - 26.8f) / (27.1f - 26.8f) * 10.0f;
    }

    return soc;
}