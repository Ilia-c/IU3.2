#ifndef __STATUS_CODES_H__
#define __STATUS_CODES_H__

#include <stdint.h>

/* ==============================
   ОПИСАНИЕ STATUS (ОШИБКИ УСТРОЙСТВА)
   uint64_t STATUS позволяет хранить до 64 ошибок
   ============================== */

/* ---------- 1. ОШИБКИ ЭКРАНА SPI ---------- */
#define STATUS_SPI_DISPLAY_INIT_ERROR      (1ULL << 0)   // Ошибка инициализации экрана SPI
#define STATUS_SPI_DISPLAY_TX_ERROR        (1ULL << 1)   // Ошибка передачи данных на экран
#define STATUS_SPI_DISPLAY_NO_RESPONSE     (1ULL << 2)   // Экран не отвечает

/* ---------- 2. ОШИБКИ ВНЕШНЕЙ EEPROM ---------- */
#define STATUS_EEPROM_INIT_ERROR           (1ULL << 3)   // Ошибка инициализации EEPROM
#define STATUS_EEPROM_WRITE_ERROR          (1ULL << 4)   // Ошибка записи в EEPROM
#define STATUS_EEPROM_READ_ERROR           (1ULL << 5)   // Ошибка чтения из EEPROM
#define STATUS_EEPROM_CRC_ERROR            (1ULL << 6)   // Ошибка проверки CRC данных EEPROM

/* ---------- 3. ОШИБКИ ВНЕШНЕГО АЦП ---------- */
#define STATUS_ADC_EXTERNAL_INIT_ERROR     (1ULL << 7)   // Ошибка инициализации внешнего АЦП
#define STATUS_ADC_EXTERNAL_SENSOR_ERROR   (1ULL << 8)   // Ошибка считывания данных с внешнего датчика
#define STATUS_ADC_BOARD_TEMP_ERROR        (1ULL << 9)   // Ошибка считывания температуры платы (аналоговый датчик)
#define STATUS_ADC_RANGE_ERROR             (1ULL << 10)  // Ошибка диапазона данных АЦП

/* ---------- 4. ОШИБКИ ДАТЧИКОВ ТЕМПЕРАТУРЫ ---------- */
#define STATUS_TEMP_SENSOR_COMM_ERROR      (1ULL << 11)  // Ошибка связи с цифровым датчиком температуры
#define STATUS_TEMP_SENSOR_RANGE_ERROR     (1ULL << 12)  // Ошибка диапазона данных цифрового датчика температуры

/* ---------- 5. ОШИБКИ RS-485 ---------- */
#define STATUS_RS485_INIT_ERROR            (1ULL << 13)  // Ошибка инициализации RS-485 (UART)
#define STATUS_RS485_RX_ERROR              (1ULL << 14)  // Ошибка приема данных RS-485
#define STATUS_RS485_TX_ERROR              (1ULL << 15)  // Ошибка передачи данных RS-485

/* ---------- 6. ОШИБКИ USB ---------- */
#define STATUS_USB_INIT_ERROR              (1ULL << 16)  // Ошибка инициализации USB
#define STATUS_USB_UART_CMD_ERROR          (1ULL << 17)  // Ошибка работы USB-UART (неверная команда)
#define STATUS_USB_FLASH_WRITE_ERROR       (1ULL << 18)  // Ошибка записи на USB-Flash
#define STATUS_USB_RESERVED_ERROR          (1ULL << 19)  // Ошибка резервного режима USB

/* ---------- 7. ОШИБКИ ВНУТРЕННЕЙ FLASH ПАМЯТИ ---------- */
#define STATUS_FLASH_INIT_ERROR            (1ULL << 20)  // Ошибка инициализации внутренней Flash памяти
#define STATUS_FLASH_WRITE_ERROR           (1ULL << 21)  // Ошибка записи во внутреннюю Flash память
#define STATUS_FLASH_READ_ERROR            (1ULL << 22)  // Ошибка чтения из внутренней Flash памяти
#define STATUS_FLASH_CRC_ERROR             (1ULL << 23)  // Ошибка контрольной суммы Flash памяти

/* ---------- 8. ОШИБКИ SD-КАРТЫ ---------- */
#define STATUS_SD_INIT_ERROR               (1ULL << 24)  // Ошибка инициализации SD-карты
#define STATUS_SD_MOUNT_ERROR              (1ULL << 25)  // Ошибка монтирования файловой системы SD-карты
#define STATUS_SD_WRITE_ERROR              (1ULL << 26)  // Ошибка записи на SD-карту
#define STATUS_SD_READ_ERROR               (1ULL << 27)  // Ошибка чтения с SD-карты
#define STATUS_SD_CORRUPTED_DATA           (1ULL << 28)  // Данные повреждены на SD
#define STATUS_SD_CRC_MISMATCH             (1ULL << 29)  // Несовпадение контрольной суммы SD

/* ---------- 9. ОШИБКИ GSM-МОДУЛЯ ---------- */
#define STATUS_GSM_REG_ERROR               (1ULL << 30)  // Ошибка регистрации в сети оператора
#define STATUS_NBIOT_REG_ERROR             (1ULL << 31)  // Ошибка регистрации в сети NB-IoT
#define STATUS_GSM_NET_ERROR               (1ULL << 32)  // Ошибка регистрации в интернет-сети от GSM
#define STATUS_GSM_NO_SIM                  (1ULL << 33)  // Отсутствует SIM-карта
#define STATUS_UART_SMS_SEND_ERROR         (1ULL << 34)  // Ошибка отправки SMS
#define STATUS_UART_SERVER_COMM_ERROR      (1ULL << 35)  // Ошибка связи с сервером
#define STATUS_UART_SERVER_UNAVAILABLE     (1ULL << 36)  // Сервер недоступен
#define STATUS_UART_NO_RESPONSE            (1ULL << 37)  // Нет ответа от сайта
#define STATUS_UART_WRONG_PASSWORD         (1ULL << 38)  // Неверный пароль с сайта

/* ---------- 10. ОШИБКИ ПРОШИВКИ ---------- */
#define STATUS_FIRMWARE_UPDATE_ERROR       (1ULL << 39)  // Ошибка обновления, нарушена целостность прошивки
#define STATUS_FIRMWARE_UPDATE_INTERRUPTED (1ULL << 40)  // Обновление было прервано

/* ---------- 11. ОБЩИЕ ОШИБКИ ---------- */
#define STATUS_IDLE_LOOP_MODE              (1ULL << 41)  // Переход в режим ЦИКЛ из-за долгого бездействия
#define STATUS_INTERNAL_ERROR              (1ULL << 42)  // Внутренняя ошибка

/* ---------- 12. КРИТИЧЕСКИЕ ОШИБКИ ---------- */
#define STATUS_CRITICAL_ERROR              (1ULL << 43)  // Критическая ошибка системы
#define STATUS_UNKNOWN_ERROR               (1ULL << 44)  // Неизвестная ошибка системы

/* ===============================
    ЗАРЕЗЕРВИРОВАННЫЕ КОДЫ ОШИБОК
   =============================== */
#define STATUS_RESERVED_1   (1ULL << 45)
#define STATUS_RESERVED_2   (1ULL << 46)
#define STATUS_RESERVED_3   (1ULL << 47)
#define STATUS_RESERVED_4   (1ULL << 48)
#define STATUS_RESERVED_5   (1ULL << 49)
#define STATUS_RESERVED_6   (1ULL << 50)
#define STATUS_RESERVED_7   (1ULL << 51)
#define STATUS_RESERVED_8   (1ULL << 52)
#define STATUS_RESERVED_9   (1ULL << 53)
#define STATUS_RESERVED_10  (1ULL << 54)
#define STATUS_RESERVED_11  (1ULL << 55)
#define STATUS_RESERVED_12  (1ULL << 56)
#define STATUS_RESERVED_13  (1ULL << 57)
#define STATUS_RESERVED_14  (1ULL << 58)
#define STATUS_RESERVED_15  (1ULL << 59)
#define STATUS_RESERVED_16  (1ULL << 60)
#define STATUS_RESERVED_17  (1ULL << 61)
#define STATUS_RESERVED_18  (1ULL << 62)
#define STATUS_RESERVED_19  (1ULL << 63)

#endif // __STATUS_CODES_H__
