#include "MS5193T.h"

extern SPI_HandleTypeDef hspi2;

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜ CS
#define SPI_CS_Enable()  HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_RESET)
#define SPI_CS_Disable() HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ROM_Pin, GPIO_PIN_SET)


// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ SPI2
uint8_t SPI2_TransmitByte(uint8_t TxData)
{
    uint8_t RxData = 0;
    HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 100);
    return RxData;
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜
uint8_t SPI2_Read_OneByte(uint8_t reg)
{
    uint8_t receivedData;

    SPI_CS_Enable();
    //HAL_Delay(1);
    SPI2_TransmitByte(reg);         // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜
    receivedData = SPI2_TransmitByte(0xFF); // ˜˜˜˜˜˜ ˜˜˜˜˜˜
    SPI_CS_Disable();

    return receivedData;
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    //HAL_Delay(1);
    SPI2_TransmitByte(reg); // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜

    for (uint8_t i = 0; i < len; i++) {
        pbuf[i] = SPI2_TransmitByte(0xFF); // ˜˜˜˜˜˜ ˜˜˜˜˜˜
    }

    SPI_CS_Disable();
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜˜˜
void SPI2_Write_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    //HAL_Delay(1);
    SPI2_TransmitByte(reg); // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜

    for (uint8_t i = 0; i < len; i++) {
        SPI2_TransmitByte(pbuf[i]); // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    }
    SPI_CS_Disable();
}

// ˜˜˜˜˜˜˜ ˜˜˜˜˜˜ MS5193T
void MS5193T_Reset(void)
{
    uint8_t resetCommand[3] = {0xFF, 0xFF, 0xFF};
    SPI2_Write_buf(0xFF, resetCommand, 3); // ˜˜˜˜˜
    //HAL_Delay(1); // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜
}

uint8_t read_por = 0;
// ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜ MS5193T
void MS5193T_Init(void) {
    uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  // ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜, ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ 33.2 ˜˜
    uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; //

    // ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜
    MS5193T_Reset();
    HAL_Delay(1);
    // ˜˜˜˜˜˜˜˜ ID ˜˜˜˜˜˜˜˜
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // ˜˜˜˜˜˜˜˜˜ 0x0B
    if ((deviceID & 0x0F) != 0x0B) {
        // ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
        return;
    }

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    SPI2_Write_buf(0x08, ModeRegisterMsg, 2);

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜
    SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);

    // ˜˜˜˜˜˜˜˜˜ IO-˜˜˜˜˜˜˜˜
    //SPI2_Write_buf(0x28, IoRegisterMsg, 1);
    HAL_Delay(100);

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    uint8_t status = SPI2_Read_OneByte(0x40); // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜ RDY
    if (status & 0x80) {
        // ˜˜˜˜˜˜: ˜˜˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
}

// ˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ MS5193T (24-˜˜˜˜˜˜ ˜˜˜˜˜˜)
uint32_t Read_MS5193T_Data(void)
{
    taskENTER_CRITICAL();
	uint8_t xtemp[3] = {0x00, 0x00, 0x00};
	int32_t adValue=0;
    SPI2_Read_buf(0x58, xtemp, 3); // ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    // ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ 3 ˜˜˜˜ ˜ 24-˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜
    adValue = (((int32_t)xtemp[0]) << 16) | (((int32_t)xtemp[1]) << 8) | xtemp[2];
    if (read_por == 0)
    {
        calculate_ADC_data_temp(adValue);
        read_por++;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010000}; // ˜˜˜˜˜ ˜˜ ˜˜˜
        // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    else
    {
        calculate_ADC_data_heigh(adValue);
        read_por = 0;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};  
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010001}; // ˜˜˜˜˜ ˜˜ ˜˜˜
        // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
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
    
    //uint8_t ModeRegisterMsg[2] = {0b00000000, 0b00000111};  
    //uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; // ˜˜˜˜˜ ˜˜ ˜˜˜
    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    //SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
    //osDelay(1);
    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜
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
    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ADC ˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ long double
    long double ADC_Volts_ld = adValue * koeff;
    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜ (˜) ˜˜ ˜˜˜˜˜˜ ˜˜˜
    long double ADC_Current_ld = ADC_Volts_ld / (*ADC_data.ADC_RESISTOR);

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜ double
    ADC_data.ADC_Volts = (double)ADC_Volts_ld;
    ADC_data.ADC_Current = (double)ADC_Current_ld;

    // ˜˜˜˜˜˜˜˜˜˜ Imin ˜ Imax ˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜ ADC
    long double Imin = (EEPROM.mode_ADC == 0) ? EEPROM.GVL_correct_4m : 0.0L;
    long double Imax = EEPROM.GVL_correct_20m;
    long double Imax_Imin = Imax - Imin;
    long double VPI_NPI = (*ADC_data.MAX_LVL) - (*ADC_data.ZERO_LVL);

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    long double ADC_min = ADC_Current_ld - Imin;
    long double ADC_d_Imm = ADC_min / Imax_Imin;

    // ˜˜˜˜˜˜˜˜˜˜ fmal ˜˜˜ ˜˜˜˜˜˜˜ ADC_SI_value ˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜
    long double ADC_SI_value_ld = fmal(ADC_d_Imm, VPI_NPI, *ADC_data.ZERO_LVL);
    ADC_data.ADC_SI_value = (double)ADC_SI_value_ld;

    // ˜˜˜˜˜˜˜˜ ˜˜˜˜ ˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜
    long double H_correct = ADC_SI_value_ld - EEPROM.ZERO_LVL;
    long double H_Imm = H_correct * Imax_Imin;
    long double ADC_Current_new_ld = fmal((H_Imm / VPI_NPI), 1.0L, EEPROM.GVL_correct_4m);
    ADC_Current_new_ld *= 1000; // ˜˜˜˜˜˜˜ ˜ ˜˜
    double ADC_Current_new = (double)ADC_Current_new_ld;

    ADC_data.ADC_SI_value_correct = ADC_data.ADC_SI_value + *ADC_data.GVL_correct;

    for (int i = 0; i<11; i++) ADC_data.ADC_SI_value_char[i] = '\0';
    for (int i = 0; i<11; i++) ADC_data.ADC_SI_value_correct_char[i] = '\0';

    if (ADC_data.ADC_SI_value < (double)*ADC_data.ZERO_LVL-0.1) {
        if (EEPROM.len == 0){
            snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "˜˜˜˜˜");
            snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "˜˜˜˜˜");
        }
        else{
            snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "Break");
            snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "Break");
        }
        Remove_units();
    }
    else{
        
        snprintf(ADC_data.ADC_SI_value_char, sizeof(ADC_data.ADC_SI_value_char), "%4.2f", ADC_data.ADC_SI_value);
        snprintf(ADC_data.ADC_SI_value_correct_char, sizeof(ADC_data.ADC_SI_value_correct_char), "%4.2f", ADC_data.ADC_SI_value_correct);
        Add_units();
    }
    snprintf(ADC_data.ADC_Volts_char, sizeof(ADC_data.ADC_Volts_char), "%4.2f", ADC_data.ADC_Volts);
    snprintf(ADC_data.ADC_Current_char, sizeof(ADC_data.ADC_Current_char), "%4.1f", ADC_Current_new);
}






