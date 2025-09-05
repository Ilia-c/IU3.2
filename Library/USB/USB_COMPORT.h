#ifndef USB_COMPORT_H
#define USB_COMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "Settings.h"
#include "Parser.h"
#include "cmsis_os.h"   // ��� TickType_t � vTaskDelay
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// ----- ��������� ������ ����� -----
#define USB_LOG_RING_SZ     1024u             // ����� ������ ������
#define USB_LOG_RING_MASK   (USB_LOG_RING_SZ - 1u) // 512 = 2^9, ������ ������
#define USB_LOG_MAX_MSG     256u              // ����. ����� ������ ���������

// ����� ������� ������ TX-����� (����� 64/128/256; ��������� 64 �������� FS USB)
#define USB_TX_CHUNK_MAX    128u
#define USB_FS_MPS          64u              // ������ �������� FS

extern xSemaphoreHandle USB_COM_TX_semaphore;
extern xSemaphoreHandle USB_COM_TX_DONE_semaphore;
extern USBD_HandleTypeDef hUsbDeviceFS;
extern HAL_StatusTypeDef USB_LogService(TickType_t tx_done_timeout);
extern HAL_StatusTypeDef ring_write(const uint8_t *src, uint32_t len);
extern void collect_message(char message[]);

extern uint8_t  s_usbLogRing[USB_LOG_RING_SZ];
extern uint32_t s_head;  // ���� ������
extern uint32_t s_tail;  // ������ �����
extern uint32_t s_dropped_bytes;

void USB_Send_Status_Report(void);
void USB_COM(void);
void AT_SEND(void);
void DEBUG_USB(void);

#ifdef __cplusplus
}
#endif

#endif 
