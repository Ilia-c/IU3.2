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
#define STATUS_SPI_DISPLAY_INIT_ERROR      (1ULL << 0)   // Ошибка инициализации экрана SPI
#define STATUS_SPI_DISPLAY_TX_ERROR        (1ULL << 1)   // Ошибка передачи данных на экран
#define STATUS_SPI_DISPLAY_NO_RESPONSE     (1ULL << 2)   // Экран не отвечает

/* ---------- 2. ОШИБКИ ВНЕШНЕЙ EEPROM (биты 3..6) ---------- */
#define STATUS_EEPROM_INIT_ERROR           (1ULL << 3)   // Ошибка инициализации EEPROM
#define STATUS_EEPROM_WRITE_ERROR          (1ULL << 4)   // Ошибка записи в EEPROM
#define STATUS_EEPROM_READ_ERROR           (1ULL << 5)   // Ошибка чтения из EEPROM
#define STATUS_EEPROM_CRC_ERROR            (1ULL << 6)   // Ошибка проверки CRC данных EEPROM

/* ---------- 3. ОШИБКИ ВНЕШНЕГО АЦП (биты 7..10) ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR     (1ULL << 7)   // Ошибка инициализации внешнего АЦП
#define STATUS_ADC_EXTERNAL_SENSOR_ERROR   (1ULL << 8)   // Ошибка считывания данных с внешнего датчика
#define STATUS_ADC_BOARD_TEMP_ERROR        (1ULL << 9)   // Ошибка считывания температуры платы (аналоговый датчик)
#define STATUS_ADC_RANGE_ERROR             (1ULL << 10)  // Ошибка диапазона данных АЦП

/* ---------- 4. ОШИБКИ ДАТЧИКОВ ТЕМПЕРАТУРЫ (биты 11..12) ---------- */
#define STATUS_TEMP_SENSOR_COMM_ERROR      (1ULL << 11)  // Ошибка связи с цифровым датчиком температуры
#define STATUS_TEMP_SENSOR_RANGE_ERROR     (1ULL << 12)  // Ошибка диапазона данных цифрового датчика температуры

/* ---------- 5. ОШИБКИ RS-485 (биты 13..15) ---------- */
#define STATUS_RS485_INIT_ERROR            (1ULL << 13)  // Ошибка инициализации RS-485 (UART)
#define STATUS_RS485_RX_ERROR              (1ULL << 14)  // Любая ошибка приема (parity/framing/overrun)
#define STATUS_RS485_TX_ERROR              (1ULL << 15)  // Ошибка передачи данных RS-485

/* ---------- 6. ОШИБКИ USB (биты 16..19) ---------- */
#define STATUS_USB_INIT_ERROR              (1ULL << 16)  // Ошибка инициализации USB
#define STATUS_USB_UART_CMD_ERROR          (1ULL << 17)  // Ошибка работы USB-UART (неверная команда)
#define STATUS_USB_FLASH_WRITE_ERROR       (1ULL << 18)  // Ошибка записи на USB-Flash
#define STATUS_USB_RESERVED_ERROR          (1ULL << 19)  // Резервная ошибка USB

/* ---------- 7. ОШИБКИ ВНУТРЕННЕЙ FLASH (биты 20..23) ---------- */
#define STATUS_FLASH_INIT_ERROR            (1ULL << 20)  // Ошибка инициализации внутренней Flash памяти
#define STATUS_FLASH_WRITE_ERROR           (1ULL << 21)  // Ошибка записи во внутреннюю Flash память
#define STATUS_FLASH_READ_ERROR            (1ULL << 22)  // Ошибка чтения из внутренней Flash памяти
#define STATUS_FLASH_CRC_ERROR             (1ULL << 23)  // Ошибка контрольной суммы Flash памяти

/* ---------- 8. ОШИБКИ SD-КАРТЫ (биты 24..31) ---------- */
#define STATUS_SD_INIT_ERROR               (1ULL << 24)  // Ошибка инициализации SD-карты
#define STATUS_SD_MOUNT_ERROR              (1ULL << 25)  // Ошибка монтирования файловой системы SD
#define STATUS_SD_WRITE_ERROR              (1ULL << 26)  // Ошибка записи на SD
#define STATUS_SD_READ_ERROR               (1ULL << 27)  // Ошибка чтения с SD
#define STATUS_SD_CORRUPTED_DATA           (1ULL << 28)  // Данные повреждены на SD
#define STATUS_SD_CRC_MISMATCH             (1ULL << 29)  // Несовпадение контрольной суммы SD
#define STATUS_SD_FILE_OPEN_ERROR          (1ULL << 30)  // Ошибка открытия файла на SD
#define STATUS_SD_TEMP_OUT_OF_RANGE        (1ULL << 31)  // Температура вне допустимого диапазона для SD

/* ---------- 9. ОШИБКИ GSM (биты 32..40) ---------- */
#define STATUS_GSM_REG_ERROR               (1ULL << 32)  // Ошибка регистрации в сети GSM
#define STATUS_NBIOT_REG_ERROR             (1ULL << 33)  // Ошибка регистрации в сети NB-IoT
#define STATUS_GSM_NET_ERROR               (1ULL << 34)  // Ошибка выхода в интернет по GSM
#define STATUS_GSM_NO_SIM                  (1ULL << 35)  // SIM-карта отсутствует
#define STATUS_UART_SMS_SEND_ERROR         (1ULL << 36)  // Ошибка отправки SMS
#define STATUS_UART_SERVER_COMM_ERROR      (1ULL << 37)  // Ошибка связи с сервером
#define STATUS_UART_SERVER_UNAVAILABLE     (1ULL << 38)  // Сервер недоступен
#define STATUS_UART_NO_RESPONSE            (1ULL << 39)  // Нет ответа от сервера/устройства
#define STATUS_UART_WRONG_PASSWORD         (1ULL << 40)  // Неверный пароль

/* ---------- 10. ОШИБКИ ПРОШИВКИ (биты 41..43) ---------- */
#define STATUS_FIRMWARE_UPDATE_ERROR       (1ULL << 41)  // Ошибка обновления (нарушена целостность прошивки)
#define STATUS_FIRMWARE_UPDATE_INTERRUPTED (1ULL << 42)  // Обновление прервано
#define STATUS_FIRMWARE_CHECK_ERROR        (1ULL << 43)  // Ошибка проверки прошивки (rollback или неверная подпись)

/* ---------- 11. ОБЩИЕ ОШИБКИ (биты 44..47) ---------- */
#define STATUS_IDLE_LOOP_MODE              (1ULL << 44)  // Переход в idle loop из-за долгого бездействия
#define STATUS_INTERNAL_ERROR              (1ULL << 45)  // Внутренняя ошибка системы
#define STATUS_VOLTAGE_TOO_LOW             (1ULL << 46)  // Слишком низкое напряжение питания
#define STATUS_VOLTAGE_TOO_HIGH            (1ULL << 47)  // Слишком высокое напряжение питания

/* ---------- 12. КРИТИЧЕСКИЕ ОШИБКИ (биты 48..51) ---------- */
#define STATUS_CRITICAL_ERROR              (1ULL << 48)  // Критическая ошибка системы
#define STATUS_UNKNOWN_ERROR               (1ULL << 49)  // Неизвестная ошибка системы
#define STATUS_STACK_OVERFLOW              (1ULL << 50)  // Переполнение стека (RTOS/система)
#define STATUS_HARDFAULT_OCCURRED          (1ULL << 51)  // Произошёл HardFault (или BusFault, если не обработан отдельно)

/* ---------- 13. ОШИБКИ W25Q (биты 52..55) ---------- */
#define STATUS_W25Q_INIT_ERROR             (1ULL << 52)  // Не удалось инициализировать W25Q
#define STATUS_W25Q_WRITE_ERROR            (1ULL << 53)  // Ошибка записи в W25Q (Page Program)
#define STATUS_W25Q_ERASE_ERROR            (1ULL << 54)  // Ошибка стирания в W25Q (Sector/Block Erase)
#define STATUS_W25Q_TIMEOUT                (1ULL << 55)  // Превышен тайм-аут W25Q (Busy не снялся)

/* ---------- 14. ИСКЛЮЧИТЕЛЬНЫЕ ФОЛТЫ (FAULTS) (биты 56..59) ---------- */
#define STATUS_NMI_OCCURRED                (1ULL << 56)  // Произошёл NMI (Non-Maskable Interrupt)
#define STATUS_MEMMANAGE_FAULT             (1ULL << 57)  // Произошёл MemManage Fault (ошибка управления памятью)
#define STATUS_BUSFAULT_OCCURRED           (1ULL << 58)  // Произошёл BusFault (ошибка доступа к памяти или периферии)
#define STATUS_USAGEFAULT_OCCURRED         (1ULL << 59)  // Произошёл UsageFault (неопределённая инструкция или иное)

/* ---------- 15. РЕЗЕРВ (биты 60..63) ---------- */
#define STATUS_RESERVED_60                 (1ULL << 60)
#define STATUS_RESERVED_61                 (1ULL << 61)
#define STATUS_RESERVED_62                 (1ULL << 62)
#define STATUS_RESERVED_63                 (1ULL << 63)

#endif // __STATUS_CODES_H__

