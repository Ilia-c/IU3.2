#include "MS5193T.h"

extern SPI_HandleTypeDef hspi2;


#define ADC_CS_PIN UART4_WU_Pin // Укажите реальный пин CS
#define ADC_CS_PORT UART4_WU_GPIO_Port      // Укажите реальный порт CS

// Функция для установки CS в низкий уровень
void ADC_Select() {
    HAL_GPIO_WritePin(ADC_CS_PORT, ADC_CS_PIN, GPIO_PIN_RESET);
}

void ADC_Deselect() {
    HAL_GPIO_WritePin(ADC_CS_PORT, ADC_CS_PIN, GPIO_PIN_SET);
}

// Функция для чтения статусного регистра
uint8_t ADC_ReadStatus() {
    uint8_t status = 0x40; // Команда для чтения статусного регистра
    uint8_t rxData;

    ADC_Select();
    HAL_SPI_Transmit(&hspi2, &status, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &rxData, 1, HAL_MAX_DELAY);
    ADC_Deselect();

    return rxData;
}

// Функция для чтения 24-битного значения с АЦП
uint32_t ADC_ReadData() {
    uint8_t rxData[3] = {0};
    uint32_t adcValue = 0;

    ADC_Select();
    HAL_SPI_Receive(&hspi2, rxData, 3, HAL_MAX_DELAY);
    ADC_Deselect();

    adcValue = ((uint32_t)rxData[0] << 16) | ((uint32_t)rxData[1] << 8) | rxData[2];
    return adcValue;
}

// Инициализация конфигурации
void ADC_Init() {
    uint8_t configData[2] = {0x01, 0x10}; // Установите нужные параметры

    ADC_Select();
    HAL_SPI_Transmit(&hspi2, configData, 2, HAL_MAX_DELAY);
    ADC_Deselect();
}
