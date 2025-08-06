/* Driver_rs485.h */
#ifndef __DRIVER_RS485_H
#define __DRIVER_RS485_H

#include "main.h" 

#define RS485_RX_BUFFER_SIZE 256
#define RS485_TX_BUFFER_SIZE 256

extern uint8_t  rs485_rx_byte;
void ON_RS485(void);
void OFF_RS485(void);
void RS485_StartReceive_IT(void);
/**
 * �������� ����� ������ (IT-�����)
 * @param pData � ��������� �� �����
 * @param Size  � ����� � ������
 * @return HAL_OK / HAL_ERROR / HAL_BUSY / HAL_TIMEOUT
 */
HAL_StatusTypeDef RS485_Transmit_IT(uint8_t *pData, uint16_t Size);

/** ������� ���������� ��������, �� ��� �� ��������� ���� */
uint16_t RS485_Available(void);

/**
 * ������� ���� ���� �� �������� ������
 * @param pByte � ���� �������� ��������� ����
 * @return 1, ���� ���� �����; 0, ���� ����� ����
 */
int RS485_ReadByte(uint8_t *pByte);

/** ����������� � ����� �������� (����� ��������������) */
__weak void RS485_TxCpltCallback(void);

/** ����������� ��� ����� ����� (����� ��������������) */
__weak void RS485_RxCpltCallback(uint8_t byte);

#endif /* __DRIVER_RS485_H */
