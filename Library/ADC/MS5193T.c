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
    //HAL_Delay(1);
    SPI2_TransmitByte(reg);         // Отправляем адрес регистра
    receivedData = SPI2_TransmitByte(0xFF); // Чтение данных
    SPI_CS_Disable();

    return receivedData;
}

// Функция для чтения нескольких байт из регистра
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    //HAL_Delay(1);
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
    //HAL_Delay(1);
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
    //HAL_Delay(1); // Небольшая задержка после сброса
}

uint8_t read_por = 0;
// Функция инициализации MS5193T
void MS5193T_Init(void) {
    uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  // Непрерывный режим, частота обновления 33.2 Гц
    uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; //

    // Сброс устройства
    MS5193T_Reset();

    // Проверка ID регистра
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // Ожидается 0x0B
    if ((deviceID & 0x0F) != 0x0B) {
        // Ошибка: Неверный ID устройства
        return;
    }

    // Настройка регистра режима
    SPI2_Write_buf(0x08, ModeRegisterMsg, 2);

    // Настройка регистра конфигурации
    SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);

    // Настройка IO-регистра
    //SPI2_Write_buf(0x28, IoRegisterMsg, 1);
    //HAL_Delay(1);

    // Проверка режима
    uint8_t status = SPI2_Read_OneByte(0x40); // Проверка флага RDY
    if (status & 0x80) {
        // Ошибка: Устройство не готово
    }
}

// Функция чтения данных из регистра данных MS5193T (24-битные данные)
void Read_MS5193T_Data(void)
{
    taskENTER_CRITICAL();
	uint8_t xtemp[3] = {0x00, 0x00, 0x00};
	int32_t adValue=0;
    SPI2_Read_buf(0x58, xtemp, 3); // Чтение регистра данных
    
    // Преобразование 3 байт в 24-битное значение со знаком
    adValue = (((int32_t)xtemp[0]) << 16) | (((int32_t)xtemp[1]) << 8) | xtemp[2];
    if (read_por == 0)
    {
        calculate_ADC_data_temp(adValue);
        read_por++;
        uint8_t ModeRegisterMsg[2] = {0b00000000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; // канал на АЦП
        // Настройка регистра режима
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        //osDelay(1);
        // Настройка регистра конфигурации
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    else
    {
        calculate_ADC_data_heigh(adValue);
        read_por = 0;
        uint8_t ModeRegisterMsg[2] = {0b00000000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000001}; // канал на АЦП
        // Настройка регистра режима
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        //osDelay(1);
        // Настройка регистра конфигурации
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    taskEXIT_CRITICAL();
}




void calculate_ADC_data_temp(int32_t adValue) {
    double koeff = 0.0000000697; 
    koeff = (double)adValue*koeff-0.5;
    koeff /= 0.01; 

    ADC_data.ADC_MS5193T_temp = koeff;
    
    for (int i = 0; i<11; i++) ADC_data.ADC_MS5193T_temp_char[i] = '\0';
    snprintf(ADC_data.ADC_MS5193T_temp_char, sizeof(ADC_data.ADC_MS5193T_temp_char), "%4g", ADC_data.ADC_MS5193T_temp);
    
    
    //uint8_t ModeRegisterMsg[2] = {0b00000000, 0b00000111};  
    //uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; // канал на АЦП
    // Настройка регистра режима
    //SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
    //osDelay(1);
    // Настройка регистра конфигурации
    //SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
}

void calculate_ADC_data_heigh(int32_t adValue) {
    double koeff = 0.0000000697; 
    koeff = adValue*koeff-0.4;
    koeff *= 9.375; 
    ADC_data.ADC_SI_value = koeff;
    for (int i = 0; i<11; i++) ADC_data.ADC_value_char[i] = '\0';
    //gcvt(ADC_data.ADC_SI_value, 6, ADC_data.ADC_value_char);
    snprintf(ADC_data.ADC_value_char, sizeof(ADC_data.ADC_value_char), "%4g", ADC_data.ADC_SI_value);


    //uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  
    //uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000001}; //
    // Настройка регистра режима
    //SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
    //osDelay(1);
    // Настройка регистра конфигурации
    //SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    //osDelay(1);
}






