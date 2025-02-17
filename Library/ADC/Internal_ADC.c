#include "Internal_ADC.h"


int32_t ADC1_Read_PC0(void) { 
    // --- Калибровка АЦП ---
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        Error_Handler();
    }

    // --- Чтение VREFINT ---
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_VREFINT;  // VREFINT
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        Error_Handler();
    }
    uint32_t vrefint_raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // --- Чтение PC0 ---
    sConfig.Channel = ADC_CHANNEL_1;  // PC0
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK) {
        Error_Handler();
    }
    uint32_t measurement_raw = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return measurement_raw;  // Возвращаем скорректированное значение АЦП
}
