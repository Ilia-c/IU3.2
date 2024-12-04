#include "MS5193T.h"

extern SPI_HandleTypeDef hspi2;


// Функции для управления CS
#define SPI_CS_Enable()  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define SPI_CS_Disable() HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)

// Функция для отправки и получения данных по SPI2
uint8_t SPI2_TransmitByte(uint8_t TxData)
{
    uint8_t RxData = 0;
    HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 1000);
    return RxData;
}

// Функция для чтения одного байта из регистра
uint8_t SPI2_Read_OneByte(uint8_t reg)
{
    uint8_t receivedData;

    SPI_CS_Enable();
    SPI2_TransmitByte(reg);         // Отправляем адрес регистра
    receivedData = SPI2_TransmitByte(0xFF); // Чтение данных
    SPI_CS_Disable();

    return receivedData;
}

// Функция для чтения нескольких байт из регистра
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    SPI2_TransmitByte(reg); // Отправляем адрес регистра

    for (uint8_t i = 0; i < len; i++) {
        pbuf[i] = SPI2_TransmitByte(0xFF); // Чтение данных
    }

    SPI_CS_Disable();
}

// Функция для записи нескольких байт в регистр
void SPI2_Write_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    SPI2_TransmitByte(reg); // Отправляем адрес регистра

    for (uint8_t i = 0; i < len; i++) {
        SPI2_TransmitByte(pbuf[i]); // Отправляем данные
    }

    SPI_CS_Disable();
}

// Функция сброса MS5193T
void MS5193T_Reset(void)
{
    uint8_t resetCommand[3] = {0xFF, 0xFF, 0xFF};
    SPI2_Write_buf(0xFF, resetCommand, 3); // Сброс
    HAL_Delay(1); // Небольшая задержка после сброса
}

// Функция инициализации MS5193T
void MS5193T_Init(void)
{
    uint8_t ModeRegisterMsg[2] = {0x00, 0x07};  // Непрерывный режим, частота 33.2 Гц
    uint8_t ConfigRegisterMsg[2] = {0x06, 0x91}; // Установка конфигурации: Gain = 64, внутренняя ссылка
    uint8_t IoRegisterMsg[1] = {0x00};           // Ток возбуждения выключен

    // Сброс устройства
    MS5193T_Reset();
    
    HAL_Delay(1);
    
    // Чтение ID регистра
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // Ожидается 0x0B для MS5193T

    // Проверка ID
    if ((deviceID & 0x0F) == 0x0B) {
        HAL_Delay(1);
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);  // Запись в регистр режима
        HAL_Delay(1);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2); // Запись в регистр конфигурации
        HAL_Delay(1);
        SPI2_Write_buf(0x28, IoRegisterMsg, 1);     // Запись в IO-регистр
        HAL_Delay(1);
    }
}

// Функция чтения данных из регистра данных MS5193T (24-битные данные)
int32_t Read_MS5193T_Data(void)
{
    uint8_t xtemp[3];
    int32_t adValue = 0;

    SPI2_Read_buf(0x58, xtemp, 3); // Чтение регистра данных

    // Преобразование 3 байт в 24-битное значение со знаком
    adValue = (((int32_t)xtemp[0]) << 16) | (((int32_t)xtemp[1]) << 8) | xtemp[2];

    return adValue;
}

// Функция для преобразования значения в двоичный формат (для отладки)
void ToBin(int32_t xValue)
{
    char btemp[26];
    for (int8_t i = 23; i >= 0; i--) {
        btemp[23 - i] = ((xValue >> i) & 0x01) ? '1' : '0';
    }
    btemp[24] = '\r';
    btemp[25] = '\n';
    
    // Здесь вы можете отправить `btemp` на последовательный порт или дисплей для отладки
}
