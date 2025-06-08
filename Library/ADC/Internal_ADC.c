#include "Internal_ADC.h"

const float alpha = 0.7f;
static uint8_t initialized = 0;

ADC_HandleTypeDef    hadc2;
ADC_HandleTypeDef    hadc1;
DMA_HandleTypeDef    hdma_adc2;
DMA_HandleTypeDef    hdma_adc1;

#define ADC_BUFFER_LEN  256 // Размер буфера для DMA
uint16_t adc_buffer[ADC_BUFFER_LEN];
uint16_t adc1_buffer[ADC_3_BUF_LEN];  // [0]=TS, [1]=VBAT

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
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK)
        Error_Handler();
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
        Error_Handler();

    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_buffer, ADC_BUFFER_LEN) != HAL_OK)
        Error_Handler();
    if (HAL_ADC_Start_DMA(&hadc1, adc1_buffer, ADC_3_BUF_LEN) != HAL_OK)
        Error_Handler();
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1) { 
    }
}

void ADC_Voltage_Calculate(void)
{
    IntADC.MK_temp = Convert_Temperature(adc1_buffer[0]);
    IntADC.MK_VBAT = Convert_Voltage(adc1_buffer[1]);
    if (IntADC.MK_VBAT<2.5f) ERRCODE.STATUS |= STATUS_VBAT_LOW;
    else ERRCODE.STATUS &= ~STATUS_VBAT_LOW;

    IntADC.ADC_AKB_volts = TruncatedMeanVoltage();
    IntADC.ADC_AKB_volts *= *IntADC.Colibrate_koeff;
    
    if (IntADC.ADC_AKB_volts < LOW_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_LOW;
    else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_LOW;
    if (IntADC.ADC_AKB_volts > HIGH_VOLTAGE) ERRCODE.STATUS |= STATUS_VOLTAGE_TOO_HIGH;
    else ERRCODE.STATUS &= ~STATUS_VOLTAGE_TOO_HIGH;

    IntADC.ADC_AKB_Proc = (uint8_t)voltageToSOC(IntADC.ADC_AKB_volts);
    sprintf(IntADC.MK_vbat_char, "%.2f", IntADC.MK_VBAT);
    sprintf(IntADC.MK_temp_char, "%.2f", IntADC.MK_temp);
    sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
    sprintf(IntADC.ADC_AKB_Proc_char, "%d", IntADC.ADC_AKB_Proc);
}

int Read_ADC_Colibrate_24V(void)
{
    IntADC.ADC_AKB_volts = TruncatedMeanVoltage();
    *IntADC.Colibrate_koeff = 24.0f / IntADC.ADC_AKB_volts;
    IntADC.ADC_AKB_volts *= *IntADC.Colibrate_koeff;
    IntADC.ADC_AKB_Proc = (uint8_t)voltageToSOC(IntADC.ADC_AKB_volts);
    sprintf(IntADC.ADC_AKB_volts_char, "%.2f", IntADC.ADC_AKB_volts);
    sprintf(IntADC.ADC_AKB_Proc_char, "%d", IntADC.ADC_AKB_Proc);
    uint16_t tmp[ADC_BUFFER_LEN] = {0};
    memcpy(adc_buffer, tmp, ADC_BUFFER_LEN * sizeof(uint16_t));
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

/* Адреса калибровочных значений в системной памяти STM32L476 */
#define TS_CAL1_ADDR    ((uint16_t*)0x1FFF75A8)  // значение при +30 °C
#define TS_CAL2_ADDR    ((uint16_t*)0x1FFF75CA)  // значение при +110 °C
 #define VREFINT_CAL_ADDR ((uint16_t *)0x1FFF75AAU)
#define TEMP_CAL1_DEG   30.0f
#define TEMP_CAL2_DEG   110.0f

// Преобразование показаний температурного датчика в градусы Цельсия
float Convert_Temperature(uint16_t raw_ts)
{
    uint16_t VREF = adc1_buffer[2];
    uint16_t vrefint_cal = *VREFINT_CAL_ADDR;
    float vdda = 3000.0f * ((float)vrefint_cal / (float)VREF);
    float raw_ts_corr = raw_ts * (vdda / 3000.0f);
    uint16_t cal1 = *TS_CAL1_ADDR; // raw при 30 °C
    uint16_t cal2 = *TS_CAL2_ADDR; // raw при 110 °C
    float temperature = ((raw_ts_corr - cal1) * (110.0f - 30.0f) / (cal2 - cal1)) + 30.0f;
    //float temperature2 = __HAL_ADC_CALC_TEMPERATURE((uint32_t)vdda, (uint32_t)raw_ts, ADC_RESOLUTION_12B); // через макросы HAL - целая часть
    return temperature;
}

// Преобразование показаний температурного датчика в градусы Цельсия
float Convert_Voltage(uint16_t raw_vbat)
{
    uint16_t raw_vref    = adc1_buffer[2];              
    uint16_t vrefint_cal = *VREFINT_CAL_ADDR;  
    float vdda = 3.0f * ((float)vrefint_cal / (float)raw_vref);
    float vbat_div3 = ((float)raw_vbat) * (vdda / 4095.0f);
    float battery_voltage = vbat_div3 * 3.0f;
    if (battery_voltage < 1.0f)  
        battery_voltage = 0.0f;
    return battery_voltage;
}
