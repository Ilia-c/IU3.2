#include "MS5193T.h"

extern SPI_HandleTypeDef hspi2;
extern xSemaphoreHandle ADC_conv_end_semaphore;

// ������� ��� ���������� CS
#define SPI_CS_Enable() HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, GPIO_PIN_RESET)
#define SPI_CS_Disable() HAL_GPIO_WritePin(SPI2_CS_ADC_GPIO_Port, SPI2_CS_ADC_Pin, GPIO_PIN_SET)

// ������� ��� �������� � ��������� ������ �� SPI2
uint8_t SPI2_TransmitByte(uint8_t TxData)
{
    uint8_t RxData = 0;
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi2, &TxData, &RxData, 1, 100);
    if (status == HAL_TIMEOUT)
    {
        ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_TIMEOUT_ERROR;

    if (status == HAL_BUSY)
    {
        ERRCODE.STATUS |= STATUS_ADC_READY_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_READY_ERROR;
    if (status == HAL_ERROR)
    {
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
    ERRCODE.STATUS &= ~STATUS_ADC_EXTERNAL_INIT_ERROR;

    return RxData;
}

// ������� ��� ������ ������ ����� �� ��������
uint8_t SPI2_Read_OneByte(uint8_t reg)
{
    uint8_t receivedData;

    SPI_CS_Enable();
    // HAL_Delay(1);
    SPI2_TransmitByte(reg);                 // ���������� ����� ��������
    receivedData = SPI2_TransmitByte(0xFF); // ������ ������
    SPI_CS_Disable();
    return receivedData;
}

// ������� ��� ������ ���������� ���� �� ��������
void SPI2_Read_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    // HAL_Delay(1);
    SPI2_TransmitByte(reg); // ���������� ����� ��������

    for (uint8_t i = 0; i < len; i++)
    {
        pbuf[i] = SPI2_TransmitByte(0xFF); // ������ ������
    }

    SPI_CS_Disable();
}

// ������� ��� ������ ���������� ���� � �������
void SPI2_Write_buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    SPI_CS_Enable();
    // HAL_Delay(1);
    SPI2_TransmitByte(reg); // ���������� ����� ��������

    for (uint8_t i = 0; i < len; i++)
    {
        SPI2_TransmitByte(pbuf[i]); // ���������� ������
    }
    SPI_CS_Disable();
}

// ������� ������ MS5193T
void MS5193T_Reset(void)
{
    uint8_t resetCommand[3] = {0xFF, 0xFF, 0xFF};
    SPI2_Write_buf(0xFF, resetCommand, 3); // �����
    // HAL_Delay(1); // ��������� �������� ����� ������
}

uint8_t read_por = 0;
// ������� ������������� MS5193T
void MS5193T_Init(void)
{
    uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};   // ����������� �����, ������� ���������� 33.2 ��
    uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10000000}; //

    // ����� ����������
    MS5193T_Reset();
    // �������� ID ��������
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // ��������� 0x0B
    if ((deviceID & 0x0F) != 0x0B)
    {
        // ������ �������������
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
        return;
    }

    // ��������� �������� ������
    SPI2_Write_buf(0x08, ModeRegisterMsg, 2);

    // ��������� �������� ������������
    SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);

    // ��������� IO-��������
    // SPI2_Write_buf(0x28, IoRegisterMsg, 1);
    HAL_Delay(100);

    // �������� ������
    uint8_t status = SPI2_Read_OneByte(0x40); // �������� ����� RDY
    if (status & 0x80)
    {
        // ������: ���������� �� ������
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
    }
}

// ������� ������ ������ �� �������� ������ MS5193T (24-������ ������)
uint32_t Read_MS5193T_Data(void)
{
    // �������� ID ��������
    uint8_t deviceID = SPI2_Read_OneByte(0x60); // ��������� 0x0B
    if ((deviceID & 0x0F) != 0x0B)
    {
        // ������ �������������
        ERRCODE.STATUS |= STATUS_ADC_EXTERNAL_INIT_ERROR;
        return;
    }

    taskENTER_CRITICAL();
    uint8_t xtemp[3] = {0x00, 0x00, 0x00};
    int32_t adValue = 0;
    SPI2_Read_buf(0x58, xtemp, 3); // ������ �������� ������
    // �������������� 3 ���� � 24-������ �������� �� ������
    adValue = (((int32_t)xtemp[0]) << 16) | (((int32_t)xtemp[1]) << 8) | xtemp[2];
    if (read_por == 0)
    {
        calculate_ADC_data_heigh(adValue, 0);
        read_por++;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010001}; // ����� �� ���
        // ��������� �������� ������
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
        taskEXIT_CRITICAL();
        return adValue;
    }
    if (read_por == 1)
    {
        calculate_ADC_data_heigh(adValue, 1);
        read_por++;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010010}; // ����� �� ���
        // ��������� �������� ������
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
        taskEXIT_CRITICAL();
        return adValue;
    }
    if (read_por == 2)
    {
        calculate_ADC_data_heigh(adValue, 2);
        read_por = 0;
        uint8_t ModeRegisterMsg[2] = {0b00100000, 0b00000111};
        uint8_t ConfigRegisterMsg[2] = {0b00010000, 0b10010000}; // ����� �� ���
        // ��������� �������� ������
        SPI2_Write_buf(0x08, ModeRegisterMsg, 2);
        SPI2_Write_buf(0x10, ConfigRegisterMsg, 2);
    }
    taskEXIT_CRITICAL();
    return adValue;
}

double round_to_n(double value, int n)
{
    double factor = pow(10.0, n);
    return round(value * factor) / factor;
}

void calculate_ADC_data_heigh(int32_t adValue, uint8_t channel)
{

    if (channel > 2)
    {
        ERRCODE.STATUS |= PROGRAMM_ERROR;
        return;
    }
    snprintf(ADC_data.ADC_value_char[channel], sizeof(ADC_data.ADC_value_char[channel]), "%" PRId32, adValue);

    const long double koeff = 0.00000006973743438720703125;
    // ��������� ���������� ADC
    long double ADC_Volts_ld = adValue * koeff;
    // ��������� ��� �� ������ ���
    long double ADC_Current_ld = ADC_Volts_ld / (ADC_data.ADC_RESISTOR[channel]);
    ADC_data.ADC_Volts[channel] = (double)ADC_Volts_ld;
    // ��������� ������������� ������������
    ADC_data.ADC_Current[channel] = (double)ADC_Current_ld * Main_data.k_koeff[channel] + Main_data.b_koeff[channel];

    // ���������� Imin � Imax � ����������� �� ������ ADC
    long double Imin = (EEPROM.mode_ADC[channel] == 0) ? DEFAULT_GVL_CORRECT_4M : 0.0L;
    long double Imax = DEFAULT_GVL_CORRECT_20M;
    long double Imax_Imin = Imax - Imin;
    long double VPI_NPI = EEPROM.MAX_LVL[channel] - EEPROM.ZERO_LVL[channel];

    // ��������� ������������� ������
    long double ADC_min = ADC_data.ADC_Current[channel] - Imin;
    long double ADC_d_Imm = ADC_min / Imax_Imin;

    // ��������� ��������� � ������ ��������� �������
    long double ADC_SI_value_ld = fmal(ADC_d_Imm, VPI_NPI, EEPROM.ZERO_LVL[channel]);
    ADC_data.ADC_SI_value[channel] = (double)ADC_SI_value_ld;

    // ��������� �������������
    ADC_data.ADC_SI_value_correct[channel] = ADC_data.ADC_SI_value[channel] + EEPROM.Correct[channel];

    // �������� ������ ��� �������� ��������
    for (int i = 0; i < 11; i++)
        ADC_data.ADC_SI_value_char[channel][i] = '\0';
    for (int i = 0; i < 11; i++)
        ADC_data.ADC_SI_value_correct_char[channel][i] = '\0';

    // ��������� �������� �������� � ��������� ������ ��� �����������
    if (ADC_data.ADC_Current[channel] < Imin - 0.0001)
    {
        if (channel == 0)
            ERRCODE.STATUS |= STATUS_ADC_RANGE_ERROR_1;
        if (channel == 1)
            ERRCODE.STATUS |= STATUS_ADC_RANGE_ERROR_2;
        if (channel == 2)
            ERRCODE.STATUS |= STATUS_ADC_RANGE_ERROR_3;
        if (EEPROM.len == 0)
        {
            snprintf(ADC_data.ADC_SI_value_char[channel], sizeof(ADC_data.ADC_SI_value_char[channel]), "�����");
            snprintf(ADC_data.ADC_SI_value_correct_char[channel], sizeof(ADC_data.ADC_SI_value_correct_char[channel]), "�����");
        }
        else
        {
            snprintf(ADC_data.ADC_SI_value_char[channel], sizeof(ADC_data.ADC_SI_value_char[channel]), "Break");
            snprintf(ADC_data.ADC_SI_value_correct_char[channel], sizeof(ADC_data.ADC_SI_value_correct_char[channel]), "Break");
        }
        if (EEPROM.Mode == 0)
            Remove_units(channel);
    }
    else
    {
        if (channel == 0)
            ERRCODE.STATUS &= ~STATUS_ADC_RANGE_ERROR_1;
        if (channel == 1)
            ERRCODE.STATUS &= ~STATUS_ADC_RANGE_ERROR_2;
        if (channel == 2)
            ERRCODE.STATUS &= ~STATUS_ADC_RANGE_ERROR_3;
        if (ADC_data.ADC_SI_value[channel] < EEPROM.ZERO_LVL[channel])
        {
            snprintf(ADC_data.ADC_SI_value_char[channel], sizeof(ADC_data.ADC_SI_value_char[channel]), "%4.2f", EEPROM.ZERO_LVL[channel]);
            snprintf(ADC_data.ADC_SI_value_correct_char[channel], sizeof(ADC_data.ADC_SI_value_correct_char[channel]), "%4.2f", ADC_data.ADC_SI_value_correct[channel]);
        }
        else
        {
            snprintf(ADC_data.ADC_SI_value_char[channel], sizeof(ADC_data.ADC_SI_value_char[channel]), "%4.2f", ADC_data.ADC_SI_value[channel]);
            snprintf(ADC_data.ADC_SI_value_correct_char[channel], sizeof(ADC_data.ADC_SI_value_correct_char[channel]), "%4.2f", ADC_data.ADC_SI_value_correct[channel]);
        }
        if (EEPROM.Mode == 0)
            Add_units(channel);
    }
    // snprintf(ADC_data.ADC_Volts_char[channel], sizeof(ADC_data.ADC_Volts_char[channel]), "%4.2f", ADC_data.ADC_Volts[channel]); // ����������� �������� ����������
    snprintf(ADC_data.ADC_Current_char[channel], sizeof(ADC_data.ADC_Current_char[channel]), "%4.3f", ADC_data.ADC_Current[channel] * 1000);
}

// ����������� ����� - �������� ���������� ���������
void Data_ADC_Thread()
{
    // �������� ��������� ��������
    // !!! ���������� ���� ����
    if (!(ERRCODE.STATUS & STATUS_ADC_EXTERNAL_INIT_ERROR))
    {
        osThreadResume(ADC_readHandle);
        // ���� ������������ � ��������� ���������� ���
        if (xSemaphoreTake(ADC_conv_end_semaphore, pdMS_TO_TICKS(15000)) != pdFALSE)
        {
            // �������������
            if ((ADC_data.ADC_SI_value_char[0][0] != 'N')){}
            else ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_CYCLE_ERROR;
        }
        else
        {
            ERRCODE.STATUS |= STATUS_ADC_TIMEOUT_CYCLE_ERROR;
        }
        osThreadSuspend(ADC_readHandle);
    }
    #if BOARD_VERSION == Version3_75 
      HAL_GPIO_WritePin(ON_OWEN_GPIO_Port, ON_OWEN_Pin, 0);
      HAL_GPIO_WritePin(ON_ROM_GPIO_Port, ON_ROM_Pin, 1);
    #elif BOARD_VERSION == Version3_80
      HAL_GPIO_WritePin(ON_OWEN_1_GPIO_Port, ON_OWEN_1_Pin, 0);
      HAL_GPIO_WritePin(ON_OWEN_2_GPIO_Port, ON_OWEN_2_Pin, 0);
      HAL_GPIO_WritePin(ON_OWEN_3_GPIO_Port, ON_OWEN_3_Pin, 0);
    #endif
}
