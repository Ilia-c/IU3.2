#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

#include <stdint.h>
#include <inttypes.h>

/* ==============================
   �������� STATUS (������ ����������)
   � ������ ���������� uint64_t STATUS ������ �� 64 ������.
   ��� ���� ������ ������������� ��������������� �� 0 �� 63.
   ============================== */

/* ---------- 1. ������ ������ SPI (���� 0..2) ---------- */

#define STATUS_SPI_DISPLAY_NO_RESPONSE (1ULL << 0) // ����� �� �������� +

/* ---------- 2. ������ ������� EEPROM (���� 3..6) ---------- */
#define STATUS_EEPROM_INIT_ERROR (1ULL << 1)        // ������ ������������� EEPROM (���������� �� �� �����) +
#define STATUS_EEPROM_READY_ERROR (1ULL << 2)       // ������ ���������� EEPROM +
#define STATUS_EEPROM_WRITE_ERROR (1ULL << 3)       // ������ ������ � EEPROM +
#define STATUS_EEPROM_READ_ERROR (1ULL << 4)        // ������ ������ �� EEPROM +
#define STATUS_EEPROM_CRC_ERROR (1ULL << 5)         // ������ �������� CRC ������ EEPROM (�������� ����������� ������) +
#define STATUS_EEPROM_TIMEOUT_I2C_ERROR (1ULL << 6) // ��������� ����� �������� ������ �� EEPROM +

/* ---------- 3. ������ �������� ��� (���� 7..10) ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR (1ULL << 7)  // ������ ������ � �������� ������ �� �������� ��� +
#define STATUS_ADC_RANGE_ERROR (1ULL << 8)          // ������ ��������� ������ ��� (�����) +
#define STATUS_ADC_TIMEOUT_ERROR (1ULL << 9)        // ������� ������ �� ��� +
#define STATUS_ADC_READY_ERROR (1ULL << 10)         // SPI ��� ����� +
#define STATUS_ADC_TIMEOUT_CYCLE_ERROR (1ULL << 11) // ������ �������� ��� � ����� (��� ��������)  +

/* ---------- 5. ������ RS-485 (���� 13..15) ---------- */
#define STATUS_RS485_INIT_ERROR (1ULL << 12) // ������ ������������� RS-485 (UART) -
#define STATUS_RS485_RX_ERROR (1ULL << 13)   // ����� ������ ������ (parity/framing/overrun) -
#define STATUS_RS485_TX_ERROR (1ULL << 14)   // ������ �������� ������ RS-485 -
#define STATUS_RS485_RESERVED_1 (1ULL << 15) // ��������� ������ - 

/* ---------- 6. ������ USB (���� 16..19) ---------- */
#define STATUS_USB_FLASH_MOUNT_ERROR (1ULL << 16) // ������ ������������ USB-Flash +
#define STATUS_USB_FULL_ERROR (1ULL << 17)        //  ������ ������������ USB-Flash +
#define STATUS_USB_OPEN_ERROR (1ULL << 18)        //  ������ �������� ����� USB-Flash +
#define STATUS_USB_LSEEK_ERROR (1ULL << 19)       //  ������ ������ ����� ����� USB-Flash +
#define STATUS_USB_FLASH_WRITE_ERROR (1ULL << 20) // ������ ������ �� USB-Flash +
#define STATUS_USB_FLASH_READ_ERROR (1ULL << 21)  // ������ ������ �� USB-Flash +
#define STATUS_USB_FLASH_SYNC_ERROR (1ULL << 22)  // ������ ������������� USB-Flash +

/* ---------- 7. ������ FLASH (���� 20..23) ---------- */
#define STATUS_FLASH_INIT_ERROR (1ULL << 23)     // ������ ������������� ���������� Flash ������ (������ ID) +
#define STATUS_FLASH_READY_ERROR (1ULL << 24)    // ������ ���������� ����� SPI Flash +
#define STATUS_FLASH_SEND_ERROR (1ULL << 25)     // ������ �������� ������� �� Flash ������ +
#define STATUS_FLASH_RECV_ERROR (1ULL << 26)     // ������ ������ ������� �� Flash ������ +
#define STATUS_FLASH_TIEOUT_ERROR (1ULL << 27)   // ������ �������� Flash ������ +
#define STATUS_FLASH_CRC_ERROR (1ULL << 28)      // ������ ������ ������ �� Flash ������ (��������� ����) +
#define STATUS_FLASH_OVERFLOW_ERROR (1ULL << 29) // ������ ������������ ������ �� Flash ������ (����������� 1 ���) + 

/* ---------- 9. ������ GSM (���� 32..40) ---------- */
#define STATUS_UART_NO_RESPONSE (1ULL << 30)          // ��� ������ �� ������ ����� +
#define STATUS_GSM_NO_SIM (1ULL << 31)                // SIM-����� ����������� + 
#define STATUS_GSM_SIGNEL_ERROR (1ULL << 32)          // ������ ������ ������ (������ 5) +
#define STATUS_GSM_REG_ERROR (1ULL << 33)             // ������ ����������� � ���� GSM +
#define STATUS_NBIOT_REG_ERROR (1ULL << 34)           // ������ ����������� � ���� NB-IoT -
#define STATUS_HTTP_SERVER_COMM_ERROR (1ULL << 35)    // ������ ����� � �������� +
#define STATUS_GSM_SMS_SEND_ERROR (1ULL << 36)        // ������ �������� SMS +
#define STATUS_HTTP_WRONG_PASSWORD_ERROR (1ULL << 37) // �������� ������ +
#define STATUS_HTTP_NO_BINDING_ERROR (1ULL << 38)     // ���������� �� ��������� �� ����� +
#define STATUS_HTTP_RESERVED (1ULL << 39)             // ��������� ������ -

/* ---------- 10. ������ �������� (���� 41..43) ---------- */
#define STATUS_FIRMWARE_UPDATE_ERROR (1ULL << 40) // ������ ���������� (�������� ����������� ��������) -
#define STATUS_FIRMWARE_COPY_SITE (1ULL << 41)    // ������ ���������� � ����� �������� -
#define STATUS_FIRMWARE_COPY_USB (1ULL << 42)     // ������ ����������� � USB-Flash �������� -

/* ---------- 11. ����� ������ (���� 44..47) ---------- */
#define STATUS_IDLE_LOOP_MODE (1ULL << 43)   // ������� � idle loop ��-�� ������� ����������� (��������� ���� ��� ��������) +
#define STATUS_VOLTAGE_TOO_LOW (1ULL << 44)  // ������� ������ ���������� ������� +
#define STATUS_VOLTAGE_TOO_HIGH (1ULL << 45) // ������� ������� ���������� ������� +
#define LOW_VOLTAGE 22.5
#define HIGH_VOLTAGE 29.3

/* ---------- 14. �������������� ����� (FAULTS) (���� 56..59) ----------  */
#define STATUS_STACK_OVERFLOW (1ULL << 46)      // ������������ ����� (RTOS/�������) +
#define STATUS_HARDFAULT_OCCURRED (1ULL << 47)  // ��������� HardFault (��� BusFault, ���� �� ��������� ��������) +
#define STATUS_NMI_OCCURRED (1ULL << 48)        // ��������� NMI (Non-Maskable Interrupt) +
#define STATUS_MEMMANAGE_FAULT (1ULL << 49)     // ��������� MemManage Fault (������ ���������� �������) +
#define STATUS_BUSFAULT_OCCURRED (1ULL << 50)   // ��������� BusFault (������ ������� � ������ ��� ���������) +
#define STATUS_USAGEFAULT_OCCURRED (1ULL << 51) // ��������� UsageFault (������������ �����) +

#define STATUS_STACK_OVERFLOW_ACK (1ULL << 52)      // ������������ ����� ������������� (2 ������) (RTOS/�������) +
#define STATUS_HARDFAULT_OCCURRED_ACK (1ULL << 53)  // ��������� HardFault ������������� (2 ������) (��� BusFault, ���� �� ��������� ��������) +
#define STATUS_NMI_OCCURRED_ACK (1ULL << 54)        // ��������� NMI ������������� (2 ������) (Non-Maskable Interrupt) +
#define STATUS_MEMMANAGE_FAULT_ACK (1ULL << 55)     // ��������� MemManage Fault ������������� (2 ������) (������ ���������� �������) +
#define STATUS_BUSFAULT_OCCURRED_ACK (1ULL << 56)   // ��������� BusFault ������������� (2 ������) (������ ������� � ������ ��� ���������) +
#define STATUS_USAGEFAULT_OCCURRED_ACK (1ULL << 57) // ��������� UsageFault ������������� (2 ������) (������������ �����) +

#define ERROR_RTC (1ULL << 58)             // ������ RTC, ������ �� ����������� ���������� +
#define STATUS_RESERVED_1 (1ULL << 59) // ��������� ������
#define STATUS_VBAT_LOW (1ULL << 60) // ������ ���������� �� ������� RTC (CR2032) +

/* ---------- 15. ������ (���� 60..63) ---------- */
#define STATUS_RESERVED_2 (1ULL << 61) // ��������� ������
#define STATUS_RESERVED_3 (1ULL << 62) // ��������� ������
#define STATUS_RESERVED_4 (1ULL << 63) // ��������� ������
/* ---------- ��������� ����� ������ ---------- */

/* 1. ������ ������ SPI */
#define STATUS_SPI_ERRORS (STATUS_SPI_DISPLAY_NO_RESPONSE)

/* 2. ������ ������� EEPROM */
#define STATUS_EEPROM_ERRORS (STATUS_EEPROM_INIT_ERROR | STATUS_EEPROM_READY_ERROR | \
                              STATUS_EEPROM_WRITE_ERROR | STATUS_EEPROM_READ_ERROR | \
                              STATUS_EEPROM_CRC_ERROR | STATUS_EEPROM_TIMEOUT_I2C_ERROR)

/* 3. ������ �������� ��� */
#define STATUS_ADC_ERRORS (STATUS_ADC_EXTERNAL_INIT_ERROR | \
                           STATUS_ADC_TIMEOUT_ERROR | STATUS_ADC_READY_ERROR |       \
                           STATUS_ADC_TIMEOUT_CYCLE_ERROR)

/* 4. ������ RS-485 */
#define STATUS_RS485_ERRORS (STATUS_RS485_INIT_ERROR | STATUS_RS485_RX_ERROR | \
                             STATUS_RS485_TX_ERROR)

/* 5. ������ USB */
#define STATUS_USB_ERRORS (STATUS_USB_FLASH_MOUNT_ERROR | STATUS_USB_FULL_ERROR |       \
                           STATUS_USB_OPEN_ERROR | STATUS_USB_LSEEK_ERROR |             \
                           STATUS_USB_FLASH_WRITE_ERROR | STATUS_USB_FLASH_READ_ERROR | \
                           STATUS_USB_FLASH_SYNC_ERROR)

/* 6. ������ Flash */
#define STATUS_FLASH_ERRORS (STATUS_FLASH_INIT_ERROR | STATUS_FLASH_READY_ERROR | \
                             STATUS_FLASH_SEND_ERROR | STATUS_FLASH_RECV_ERROR |  \
                             STATUS_FLASH_TIEOUT_ERROR | STATUS_FLASH_CRC_ERROR | \
                             STATUS_FLASH_OVERFLOW_ERROR)

/* 7. ������ GSM/NB-IoT � ����� */
#define STATUS_GSM_ERRORS (STATUS_UART_NO_RESPONSE | STATUS_GSM_NO_SIM |                  \
                           STATUS_GSM_SIGNEL_ERROR | STATUS_GSM_REG_ERROR |               \
                           STATUS_NBIOT_REG_ERROR | STATUS_HTTP_SERVER_COMM_ERROR |       \
                           STATUS_GSM_SMS_SEND_ERROR | STATUS_HTTP_WRONG_PASSWORD_ERROR | \
                           STATUS_HTTP_NO_BINDING_ERROR)

/* 8. ������ �������� */
#define STATUS_FIRMWARE_ERRORS (STATUS_FIRMWARE_UPDATE_ERROR | STATUS_FIRMWARE_UPDATE_SITE)

/* 9. ����� ������ */
#define STATUS_COMMON_ERRORS (STATUS_IDLE_LOOP_MODE | STATUS_VOLTAGE_TOO_LOW | STATUS_VOLTAGE_TOO_HIGH)

/* 10. ����������� ������ */
#define STATUS_CRITICAL_ERRORS (STATUS_STACK_OVERFLOW | STATUS_HARDFAULT_OCCURRED)

/* 11. ����� (�������������� ������) */
#define STATUS_FAULTS (STATUS_STACK_OVERFLOW| STATUS_HARDFAULT_OCCURRED | STATUS_NMI_OCCURRED | \
   STATUS_MEMMANAGE_FAULT | STATUS_USAGEFAULT_OCCURRED | STATUS_USAGEFAULT_OCCURRED)
#define STATUS_FAULTS_ACK (STATUS_STACK_OVERFLOW_ACK| STATUS_HARDFAULT_OCCURRED_ACK | STATUS_NMI_OCCURRED_ACK | \
      STATUS_MEMMANAGE_FAULT_ACK | STATUS_USAGEFAULT_OCCURRED_ACK | STATUS_USAGEFAULT_OCCURRED_ACK)


#define FAULTS_MASK (STATUS_STACK_OVERFLOW |         \
                     STATUS_HARDFAULT_OCCURRED |     \
                     STATUS_NMI_OCCURRED |           \
                     STATUS_MEMMANAGE_FAULT |        \
                     STATUS_BUSFAULT_OCCURRED |      \
                     STATUS_USAGEFAULT_OCCURRED |    \
                     STATUS_STACK_OVERFLOW_ACK |     \
                     STATUS_HARDFAULT_OCCURRED_ACK | \
                     STATUS_NMI_OCCURRED_ACK |       \
                     STATUS_MEMMANAGE_FAULT_ACK |    \
                     STATUS_BUSFAULT_OCCURRED_ACK |  \
                     STATUS_USAGEFAULT_OCCURRED_ACK)

#define CRITICAL_ERRORS_MASK (STATUS_SPI_DISPLAY_NO_RESPONSE | STATUS_EEPROM_INIT_ERROR  | \
                             STATUS_EEPROM_CRC_ERROR | STATUS_EEPROM_TIMEOUT_I2C_ERROR | \
                             STATUS_ADC_EXTERNAL_INIT_ERROR | STATUS_ADC_TIMEOUT_ERROR | \
                             STATUS_RS485_INIT_ERROR | STATUS_FLASH_INIT_ERROR | \
                             STATUS_FLASH_OVERFLOW_ERROR | STATUS_UART_NO_RESPONSE | \
                             STATUS_VOLTAGE_TOO_LOW | STATUS_VOLTAGE_TOO_HIGH | \
                             FAULTS_MASK | ERROR_RTC | STATUS_VBAT_LOW)

#endif // __STATUS_CODES_H__
