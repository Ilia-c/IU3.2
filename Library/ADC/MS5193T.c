#include "MS5193T.h"

extern SPI_HandleTypeDef hspi2;

// Функции для управления CS
#define SPI_CS_Enable()  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define SPI_CS_Disable() HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)


// Функция для отправки и получения данных по SPI2
uint8_t SPI2_TransmitByte(uint8_t TxData)
{
    uint8_t RxData = 0;
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 100);
    if (status == HAL_TIMEOUT) {
        ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_TIMEOUT_ERROR;

    if (status == HAL_BUSY) {
        ERRCODE.STATUS |= STATUS_ADC_READY_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_READY_ERROR;
    if (status == HAL_ERROR) {
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_EXTERNAL_INIT_ERROR;

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
        // Ошибка инициализации
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
        return;
    }

    // Настройка регистра режима
    SPI2_Write_buf(0x08, ModeRegisterMsg, 2);

    // Настройка регистра конфигурации
    SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);

    // Настройка IO-регистра
    //SPI2_Write_buf(0x28, IoRegisterMsg, 1);
    HAL_Delay(100);

    // Проверка режима
    uint8_t status = SPI2_Read_OneByte(0x40); // Проверка флага RDY
    if (status & 0x80) {
        // Ошибка: Устройство не готово
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
}

// Функция чтения данных из регистра данных MS5193T (24-битные данные)
uint32_t Read_MS5193T_Data(void)
{
    // Проверка ID регистра
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // Ожидается 0x0B
    if ((deviceID & 0x0F) != 0x0B)
    {
        // Ошибка инициализации
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
        return;
    }
    
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
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010000}; // канал на АЦП
        // Настройка регистра режима
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    else
    {
        calculate_ADC_data_heigh(adValue);
        read_por = 0;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010001}; // канал на АЦП
        // Настройка регистра режима
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    taskEXIT_CRITICAL();
    return adValue;
}

void calculate_ADC_data_temp(int32_t adValue) {
    double koeff = 0.0000000697; 
    koeff = (double)adValue*koeff-0.5;
    koeff /= 0.01; 

    ADC_data.ADC_MS5193T_temp = koeff+*ADC_data.Temp_correct_A;
    for (int i = 0; i<11; i++) ADC_data.ADC_MS5193T_temp_char[i] = '\0';
    snprintf(ADC_data.ADC_MS5193T_temp_char, sizeof(ADC_data.ADC_MS5193T_temp_char), "%4.1f", ADC_data.ADC_MS5193T_temp);
    if ((ADC_data.ADC_MS5193T_temp<-40) || (ADC_data.ADC_MS5193T_temp>150)) ERRCODE.STATUS |= STATUS_ADC_TEMP_ERROR;
    else ERRCODE.STATUS &= ~STATUS_ADC_TEMP_ERROR;
    //uint8_t ModeRegisterMsg[2] = {0b00000000, 0b00000111};  
    //uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; // канал на АЦП
    // Настройка регистра режима
    //SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
    //osDelay(1);
    // Настройка регистра конфигурации
    //SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
}



double round_to_n(double value, int n) {
    double factor = pow(10.0, n);
    return round(value * factor) / factor;
}
extern EEPROM_Settings_item EEPROM;
void calculate_ADC_data_heigh(int32_t adValue) {
    snprintf(ADC_data.ADC_value_char, sizeof(ADC_data.ADC_value_char), "%" PRId32, adValue);
    const long double koeff = 0.00000006973743438720703125;
    // Вычисляем напряжение ADC с использованием long double
    long double ADC_Volts_ld = adValue * koeff;
    // Вычисляем первичный ток (А) по закону Ома
    long double ADC_Current_ld = ADC_Volts_ld / (*ADC_data.ADC_RESISTOR);

    // Сохраняем результаты в структуре как double
    ADC_data.ADC_Volts = (double)ADC_Volts_ld;
    ADC_data.ADC_Current = (double)ADC_Current_ld;

    // Определяем Imin и Imax в зависимости от режима ADC
    long double Imin = (EEPROM.mode_ADC == 0) ? EEPROM.GVL_correct_4m : 0.0L;
    long double Imax = EEPROM.GVL_correct_20m;
    long double Imax_Imin = Imax - Imin;
    long double VPI_NPI = (*ADC_data.MAX_LVL) - (*ADC_data.ZERO_LVL);

    // Вычисляем относительный сигнал
    long double ADC_min = ADC_Current_ld - Imin;
    long double ADC_d_Imm = ADC_min / Imax_Imin;

    // Используем fmal для расчёта ADC_SI_value с одним округлением
    long double ADC_SI_value_ld = fmal(ADC_d_Imm, VPI_NPI, *ADC_data.ZERO_LVL);
    ADC_data.ADC_SI_value = (double)ADC_SI_value_ld;

    // Пересчёт тока с учетом калибровки
    long double H_correct = ADC_SI_value_ld - EEPROM.ZERO_LVL;
    long double H_Imm = H_correct * Imax_Imin;
    long double ADC_Current_new_ld = fmal((H_Imm / VPI_NPI), 1.0L, EEPROM.GVL_correct_4m);
    ADC_Current_new_ld *= 1000; // Перевод в мА
    double ADC_Current_new = (double)ADC_Current_new_ld;

    ADC_data.ADC_SI_value_correct = ADC_data.ADC_SI_value + *ADC_data.GVL_correct;

    for (int i = 0; i<11; i++) ADC_data.ADC_SI_value_char[i] = '\0';
    for (int i = 0; i<11; i++) ADC_data.ADC_SI_value_correct_char[i] = '\0';

    if (ADC_data.ADC_SI_value < (double)*ADC_data.ZERO_LVL-0.1) {
        ERRCODE.STATUS |= STATUS_ADC_RANGE_ERROR;
        if (EEPROM.len == 0){
            snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "Обрыв");
            snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "Обрыв");
        }
        else{
            snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "Break");
            snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "Break");
        }
        if (EEPROM.Mode == 0) Remove_units();
    }
    else{
        ERRCODE.STATUS &= ~STATUS_ADC_RANGE_ERROR;
        snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "%4.2f", ADC_data.ADC_SI_value);
        snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "%4.2f", ADC_data.ADC_SI_value_correct);
        if (EEPROM.Mode == 0) Add_units();
    }
    snprintf(ADC_data.ADC_Volts_char, sizeof(ADC_data.ADC_Volts_char), "%4.2f", ADC_data.ADC_Volts);
    snprintf(ADC_data.ADC_Current_char, sizeof(ADC_data.ADC_Current_char), "%4.1f", ADC_Current_new);
}






