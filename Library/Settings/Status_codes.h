#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

#include <stdint.h>

/* ==============================
   ОПИСАНИЕ STATUS (ОШИБКИ УСТРОЙСТВА)
   uint64_t STATUS позволяет хранить до 64 ошибок
   ============================== */

/* ---------- 1. ОШИБКИ ЭКРАНА SPI ---------- */
#define STATUS_SPI_DISPLAY_INIT_ERROR      0x0000000000000001ULL  // Ошибка инициализации экрана SPI
#define STATUS_SPI_DISPLAY_TX_ERROR        0x0000000000000002ULL  // Ошибка передачи данных на экран
#define STATUS_SPI_DISPLAY_NO_RESPONSE     0x0000000000000004ULL  // Экран не отвечает

/* ---------- 2. ОШИБКИ ВНЕШНЕЙ EEPROM ---------- */
#define STATUS_EEPROM_INIT_ERROR           0x0000000000000008ULL  // Ошибка инициализации EEPROM
#define STATUS_EEPROM_WRITE_ERROR          0x0000000000000010ULL  // Ошибка записи в EEPROM
#define STATUS_EEPROM_READ_ERROR           0x0000000000000020ULL  // Ошибка чтения из EEPROM
#define STATUS_EEPROM_CRC_ERROR            0x0000000000000040ULL  // Ошибка проверки CRC данных EEPROM

/* ---------- 3. ОШИБКИ ВНЕШНЕГО АЦП ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR     0x0000000000000080ULL  // Ошибка инициализации внешнего АЦП
#define STATUS_ADC_EXTERNAL_SENSOR_ERROR   0x0000000000000100ULL  // Ошибка считывания данных с внешнего датчика
#define STATUS_ADC_BOARD_TEMP_ERROR        0x0000000000000200ULL  // Ошибка считывания температуры платы (аналоговый датчик)
#define STATUS_ADC_RANGE_ERROR             0x0000000000000400ULL  // Ошибка диапазона данных АЦП

/* ---------- 4. ОШИБКИ ДАТЧИКОВ ТЕМПЕРАТУРЫ ---------- */
#define STATUS_TEMP_SENSOR_COMM_ERROR      0x0000000000000800ULL  // Ошибка связи с цифровым датчиком температуры
#define STATUS_TEMP_SENSOR_RANGE_ERROR     0x0000000000001000ULL  // Ошибка диапазона данных цифрового датчика температуры

/* ---------- 5. ОШИБКИ RS-485 ---------- */
#define STATUS_RS485_INIT_ERROR            0x0000000000002000ULL  // Ошибка инициализации RS-485 (UART)
#define STATUS_RS485_RX_ERROR              0x0000000000004000ULL  // Ошибка приема данных RS-485
#define STATUS_RS485_TX_ERROR              0x0000000000008000ULL  // Ошибка передачи данных RS-485

/* ---------- 6. ОШИБКИ USB ---------- */
#define STATUS_USB_INIT_ERROR              0x0000000000010000ULL  // Ошибка инициализации USB
#define STATUS_USB_UART_CMD_ERROR          0x0000000000020000ULL  // Ошибка работы USB-UART (неверная команда)
#define STATUS_USB_FLASH_WRITE_ERROR       0x0000000000040000ULL  // Ошибка записи на USB-Flash
#define STATUS_USB_RESERVED_ERROR          0x0000000000080000ULL  // Ошибка резервного режима USB

/* ---------- 7. ОШИБКИ ВНУТРЕННЕЙ FLASH ПАМЯТИ ---------- */
#define STATUS_FLASH_INIT_ERROR            0x0000000000100000ULL  // Ошибка инициализации внутренней Flash памяти
#define STATUS_FLASH_WRITE_ERROR           0x0000000000200000ULL  // Ошибка записи во внутреннюю Flash память
#define STATUS_FLASH_READ_ERROR            0x0000000000400000ULL  // Ошибка чтения из внутренней Flash памяти
#define STATUS_FLASH_CRC_ERROR             0x0000000000800000ULL  // Ошибка контрольной суммы Flash памяти

/* ---------- 8. ОШИБКИ SD-КАРТЫ ---------- */
#define STATUS_SD_INIT_ERROR               0x0000000001000000ULL  // Ошибка инициализации SD-карты
#define STATUS_SD_MOUNT_ERROR              0x0000000002000000ULL  // Ошибка монтирования файловой системы SD-карты
#define STATUS_SD_WRITE_ERROR              0x0000000004000000ULL  // Ошибка записи на SD-карту
#define STATUS_SD_READ_ERROR               0x0000000008000000ULL  // Ошибка чтения с SD-карты
#define STATUS_SD_CORRUPTED_DATA           0x0000000010000000ULL  // Данные повреждены на SD
#define STATUS_SD_CRC_MISMATCH             0x0000000020000000ULL  // Несовпадение контрольной суммы SD

/* ---------- 9. ОШИБКИ GSM-МОДУЛЯ ---------- */
#define STATUS_GSM_REG_ERROR               0x0000000040000000ULL  // Ошибка регистрации в сети оператора
#define STATUS_NBIOT_REG_ERROR             0x0000000080000000ULL  // Ошибка регистрации в сети NB-IoT
#define STATUS_GSM_NET_ERROR               0x0000000100000000ULL  // Ошибка регистрации в интернет-сети от GSM
#define STATUS_GSM_NO_SIM                  0x0000000200000000ULL  // Отсутствует SIM-карта
#define STATUS_UART_SMS_SEND_ERROR         0x0000000400000000ULL  // Ошибка отправки SMS
#define STATUS_UART_SERVER_COMM_ERROR      0x0000000800000000ULL  // Ошибка связи с сервером
#define STATUS_UART_SERVER_UNAVAILABLE     0x0000001000000000ULL  // Сервер недоступен
#define STATUS_UART_NO_RESPONSE            0x0000002000000000ULL  // Нет ответа от сайта
#define STATUS_UART_WRONG_PASSWORD         0x0000004000000000ULL  // Неверный пароль с сайта

/* ---------- 10. ОШИБКИ ПРОШИВКИ ---------- */
#define STATUS_FIRMWARE_UPDATE_ERROR       0x0000008000000000ULL  // Ошибка обновления, нарушена целостность прошивки
#define STATUS_FIRMWARE_UPDATE_INTERRUPTED 0x0000010000000000ULL  // Обновление было прервано

/* ---------- 11. ОБЩИЕ ОШИБКИ ---------- */
#define STATUS_IDLE_LOOP_MODE              0x0000020000000000ULL  // Переход в режим ЦИКЛ из-за долгого бездействия
#define STATUS_INTERNAL_ERROR              0x0000040000000000ULL  // Внутренняя ошибка

/* ---------- 12. КРИТИЧЕСКИЕ ОШИБКИ ---------- */
#define STATUS_CRITICAL_ERROR              0x0000080000000000ULL  // Критическая ошибка системы
#define STATUS_UNKNOWN_ERROR               0x0000100000000000ULL  // Неизвестная ошибка системы


/* ===============================
    ЗАРЕЗЕРВИРОВАННЫЕ КОДЫ ОШИБОК
   =============================== */

#define STATUS_RESERVED_1   0x0000200000000000ULL
#define STATUS_RESERVED_2   0x0000400000000000ULL
#define STATUS_RESERVED_3   0x0000800000000000ULL
#define STATUS_RESERVED_4   0x0001000000000000ULL
#define STATUS_RESERVED_5   0x0002000000000000ULL
#define STATUS_RESERVED_6   0x0004000000000000ULL
#define STATUS_RESERVED_7   0x0008000000000000ULL
#define STATUS_RESERVED_8   0x0010000000000000ULL
#define STATUS_RESERVED_9   0x0020000000000000ULL
#define STATUS_RESERVED_10  0x0040000000000000ULL
#define STATUS_RESERVED_11  0x0080000000000000ULL
#define STATUS_RESERVED_12  0x0100000000000000ULL
#define STATUS_RESERVED_13  0x0200000000000000ULL
#define STATUS_RESERVED_14  0x0400000000000000ULL
#define STATUS_RESERVED_15  0x0800000000000000ULL
#define STATUS_RESERVED_16  0x1000000000000000ULL
#define STATUS_RESERVED_17  0x2000000000000000ULL
#define STATUS_RESERVED_18  0x4000000000000000ULL
#define STATUS_RESERVED_19  0x8000000000000000ULL

#endif // __STATUS_CODES_H__
