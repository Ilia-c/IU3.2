#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

#include <stdint.h>
#include <inttypes.h>

/* ==============================
   ОПИСАНИЕ STATUS (ОШИБКИ УСТРОЙСТВА)
   В данной реализации uint64_t STATUS хранит до 64 флагов.
   Все коды ошибок пронумерованы последовательно от 0 до 63.
   ============================== */

/* ---------- 1. ОШИБКИ ЭКРАНА SPI (биты 0..2) ---------- */

#define STATUS_SPI_DISPLAY_NO_RESPONSE (1ULL << 0) // Экран не отвечает +

/* ---------- 2. ОШИБКИ ВНЕШНЕЙ EEPROM (биты 3..6) ---------- */
#define STATUS_EEPROM_INIT_ERROR (1ULL << 1)        // Ошибка инициализации EEPROM (устройство ее не видит) +
#define STATUS_EEPROM_READY_ERROR (1ULL << 2)       // Ошибка готовности EEPROM +
#define STATUS_EEPROM_WRITE_ERROR (1ULL << 3)       // Ошибка записи в EEPROM +
#define STATUS_EEPROM_READ_ERROR (1ULL << 4)        // Ошибка чтения из EEPROM +
#define STATUS_EEPROM_CRC_ERROR (1ULL << 5)         // Ошибка проверки CRC данных EEPROM (нарушена целостность данных) +
#define STATUS_EEPROM_TIMEOUT_I2C_ERROR (1ULL << 6) // Превышено время ожидпния ответа от EEPROM +

/* ---------- 3. ОШИБКИ ВНЕШНЕГО АЦП (биты 7..10) ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR (1ULL << 7)  // Ошибка приема и передачи данных от внешнего АЦП +
#define STATUS_ADC_RANGE_ERROR (1ULL << 8)          // Ошибка диапазона данных АЦП (обрыв) +
#define STATUS_ADC_TIMEOUT_ERROR (1ULL << 9)        // Таймаут ответа от АЦП +
#define STATUS_ADC_READY_ERROR (1ULL << 10)         // SPI АЦП занят +
#define STATUS_ADC_TIMEOUT_CYCLE_ERROR (1ULL << 11) // Ошибка таймаута АЦП в цикле (доп проверка)  +

/* ---------- 5. ОШИБКИ RS-485 (биты 13..15) ---------- */
#define STATUS_RS485_INIT_ERROR (1ULL << 12) // Ошибка инициализации RS-485 (UART) -
#define STATUS_RS485_RX_ERROR (1ULL << 13)   // Любая ошибка приема (parity/framing/overrun) -
#define STATUS_RS485_TX_ERROR (1ULL << 14)   // Ошибка передачи данных RS-485 -
#define STATUS_RS485_RESERVED_1 (1ULL << 15) // Резервная ошибка - 

/* ---------- 6. ОШИБКИ USB (биты 16..19) ---------- */
#define STATUS_USB_FLASH_MOUNT_ERROR (1ULL << 16) // Ошибка монтирования USB-Flash +
#define STATUS_USB_FULL_ERROR (1ULL << 17)        //  Ошибка заполнености USB-Flash +
#define STATUS_USB_OPEN_ERROR (1ULL << 18)        //  Ошибка открытия файла USB-Flash +
#define STATUS_USB_LSEEK_ERROR (1ULL << 19)       //  Ошибка поиска конца файла USB-Flash +
#define STATUS_USB_FLASH_WRITE_ERROR (1ULL << 20) // Ошибка записи на USB-Flash +
#define STATUS_USB_FLASH_READ_ERROR (1ULL << 21)  // Ошибка чтения на USB-Flash +
#define STATUS_USB_FLASH_SYNC_ERROR (1ULL << 22)  // Ошибка синхронизации USB-Flash +

/* ---------- 7. ОШИБКИ FLASH (биты 20..23) ---------- */
#define STATUS_FLASH_INIT_ERROR (1ULL << 23)     // Ошибка инициализации внутренней Flash памяти (чтение ID) +
#define STATUS_FLASH_READY_ERROR (1ULL << 24)    // Ошибка готовности линии SPI Flash +
#define STATUS_FLASH_SEND_ERROR (1ULL << 25)     // ошибка отправки команды на Flash память +
#define STATUS_FLASH_RECV_ERROR (1ULL << 26)     // ошибка приема команды от Flash памяти +
#define STATUS_FLASH_TIEOUT_ERROR (1ULL << 27)   // Ошибка таймаута Flash памяти +
#define STATUS_FLASH_CRC_ERROR (1ULL << 28)      // Ошибка чтения данных из Flash памяти (поврежден блок) +
#define STATUS_FLASH_OVERFLOW_ERROR (1ULL << 29) // Ошибка переполнения данных на Flash памяти (установится 1 раз) + 

/* ---------- 9. ОШИБКИ GSM (биты 32..40) ---------- */
#define STATUS_UART_NO_RESPONSE (1ULL << 30)          // Нет ответа от модуля связи +
#define STATUS_GSM_NO_SIM (1ULL << 31)                // SIM-карта отсутствует + 
#define STATUS_GSM_SIGNEL_ERROR (1ULL << 32)          // Ошибка слабый сигнал (меньше 5) +
#define STATUS_GSM_REG_ERROR (1ULL << 33)             // Ошибка регистрации в сети GSM +
#define STATUS_NBIOT_REG_ERROR (1ULL << 34)           // Ошибка регистрации в сети NB-IoT -
#define STATUS_HTTP_SERVER_COMM_ERROR (1ULL << 35)    // Ошибка связи с сервером +
#define STATUS_GSM_SMS_SEND_ERROR (1ULL << 36)        // Ошибка отправки SMS +
#define STATUS_HTTP_WRONG_PASSWORD_ERROR (1ULL << 37) // Неверный пароль +
#define STATUS_HTTP_NO_BINDING_ERROR (1ULL << 38)     // Устройство не привязано на сайте +
#define STATUS_HTTP_RESERVED (1ULL << 39)             // Резервная ошибка -

/* ---------- 10. ОШИБКИ ПРОШИВКИ (биты 41..43) ---------- */
#define STATUS_FIRMWARE_UPDATE_ERROR (1ULL << 40) // Ошибка обновления (нарушена целостность прошивки) -
#define STATUS_FIRMWARE_COPY_SITE (1ULL << 41)    // Ошибка скачивания с сайта прошивки -
#define STATUS_FIRMWARE_COPY_USB (1ULL << 42)     // Ошибка копирования с USB-Flash прошивки -

/* ---------- 11. ОБЩИЕ ОШИБКИ (биты 44..47) ---------- */
#define STATUS_IDLE_LOOP_MODE (1ULL << 43)   // Переход в idle loop из-за долгого бездействия (сохранять коды при переходе) +
#define STATUS_VOLTAGE_TOO_LOW (1ULL << 44)  // Слишком низкое напряжение питания +
#define STATUS_VOLTAGE_TOO_HIGH (1ULL << 45) // Слишком высокое напряжение питания +
#define LOW_VOLTAGE 22.5
#define HIGH_VOLTAGE 29.3

/* ---------- 14. ИСКЛЮЧИТЕЛЬНЫЕ ФОЛТЫ (FAULTS) (биты 56..59) ----------  */
#define STATUS_STACK_OVERFLOW (1ULL << 46)      // Переполнение стека (RTOS/система) +
#define STATUS_HARDFAULT_OCCURRED (1ULL << 47)  // Произошёл HardFault (или BusFault, если не обработан отдельно) +
#define STATUS_NMI_OCCURRED (1ULL << 48)        // Произошёл NMI (Non-Maskable Interrupt) +
#define STATUS_MEMMANAGE_FAULT (1ULL << 49)     // Произошёл MemManage Fault (ошибка управления памятью) +
#define STATUS_BUSFAULT_OCCURRED (1ULL << 50)   // Произошёл BusFault (ошибка доступа к памяти или периферии) +
#define STATUS_USAGEFAULT_OCCURRED (1ULL << 51) // Произошёл UsageFault (переполнение стека) +

#define STATUS_STACK_OVERFLOW_ACK (1ULL << 52)      // Переполнение стека подтверждение (2 подряд) (RTOS/система) +
#define STATUS_HARDFAULT_OCCURRED_ACK (1ULL << 53)  // Произошёл HardFault подтверждение (2 подряд) (или BusFault, если не обработан отдельно) +
#define STATUS_NMI_OCCURRED_ACK (1ULL << 54)        // Произошёл NMI подтверждение (2 подряд) (Non-Maskable Interrupt) +
#define STATUS_MEMMANAGE_FAULT_ACK (1ULL << 55)     // Произошёл MemManage Fault подтверждение (2 подряд) (ошибка управления памятью) +
#define STATUS_BUSFAULT_OCCURRED_ACK (1ULL << 56)   // Произошёл BusFault подтверждение (2 подряд) (ошибка доступа к памяти или периферии) +
#define STATUS_USAGEFAULT_OCCURRED_ACK (1ULL << 57) // Произошёл UsageFault подтверждение (2 подряд) (переполнение стека) +

#define ERROR_RTC (1ULL << 58)             // Ошибка RTC, запуск от внутреннего генератора +
#define STATUS_RESERVED_1 (1ULL << 59) // Резервная ошибка
#define STATUS_VBAT_LOW (1ULL << 60) // Низкое напряжение на батарее RTC (CR2032) +

/* ---------- 15. РЕЗЕРВ (биты 60..63) ---------- */
#define STATUS_RESERVED_2 (1ULL << 61) // Резервная ошибка
#define STATUS_RESERVED_3 (1ULL << 62) // Резервная ошибка
#define STATUS_RESERVED_4 (1ULL << 63) // Резервная ошибка
/* ---------- Групповые маски ошибок ---------- */

/* 1. Ошибки экрана SPI */
#define STATUS_SPI_ERRORS (STATUS_SPI_DISPLAY_NO_RESPONSE)

/* 2. Ошибки внешней EEPROM */
#define STATUS_EEPROM_ERRORS (STATUS_EEPROM_INIT_ERROR | STATUS_EEPROM_READY_ERROR | \
                              STATUS_EEPROM_WRITE_ERROR | STATUS_EEPROM_READ_ERROR | \
                              STATUS_EEPROM_CRC_ERROR | STATUS_EEPROM_TIMEOUT_I2C_ERROR)

/* 3. Ошибки внешнего АЦП */
#define STATUS_ADC_ERRORS (STATUS_ADC_EXTERNAL_INIT_ERROR | \
                           STATUS_ADC_TIMEOUT_ERROR | STATUS_ADC_READY_ERROR |       \
                           STATUS_ADC_TIMEOUT_CYCLE_ERROR)

/* 4. Ошибки RS-485 */
#define STATUS_RS485_ERRORS (STATUS_RS485_INIT_ERROR | STATUS_RS485_RX_ERROR | \
                             STATUS_RS485_TX_ERROR)

/* 5. Ошибки USB */
#define STATUS_USB_ERRORS (STATUS_USB_FLASH_MOUNT_ERROR | STATUS_USB_FULL_ERROR |       \
                           STATUS_USB_OPEN_ERROR | STATUS_USB_LSEEK_ERROR |             \
                           STATUS_USB_FLASH_WRITE_ERROR | STATUS_USB_FLASH_READ_ERROR | \
                           STATUS_USB_FLASH_SYNC_ERROR)

/* 6. Ошибки Flash */
#define STATUS_FLASH_ERRORS (STATUS_FLASH_INIT_ERROR | STATUS_FLASH_READY_ERROR | \
                             STATUS_FLASH_SEND_ERROR | STATUS_FLASH_RECV_ERROR |  \
                             STATUS_FLASH_TIEOUT_ERROR | STATUS_FLASH_CRC_ERROR | \
                             STATUS_FLASH_OVERFLOW_ERROR)

/* 7. Ошибки GSM/NB-IoT и связи */
#define STATUS_GSM_ERRORS (STATUS_UART_NO_RESPONSE | STATUS_GSM_NO_SIM |                  \
                           STATUS_GSM_SIGNEL_ERROR | STATUS_GSM_REG_ERROR |               \
                           STATUS_NBIOT_REG_ERROR | STATUS_HTTP_SERVER_COMM_ERROR |       \
                           STATUS_GSM_SMS_SEND_ERROR | STATUS_HTTP_WRONG_PASSWORD_ERROR | \
                           STATUS_HTTP_NO_BINDING_ERROR)

/* 8. Ошибки прошивки */
#define STATUS_FIRMWARE_ERRORS (STATUS_FIRMWARE_UPDATE_ERROR | STATUS_FIRMWARE_UPDATE_SITE)

/* 9. Общие ошибки */
#define STATUS_COMMON_ERRORS (STATUS_IDLE_LOOP_MODE | STATUS_VOLTAGE_TOO_LOW | STATUS_VOLTAGE_TOO_HIGH)

/* 10. Критические ошибки */
#define STATUS_CRITICAL_ERRORS (STATUS_STACK_OVERFLOW | STATUS_HARDFAULT_OCCURRED)

/* 11. Фолты (исключительные ошибки) */
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
