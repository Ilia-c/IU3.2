#include "SD.h"

#include "fatfs.h"
#include "string.h"

/* Внешние переменные, определённые в другом файле */
extern FATFS SDFatFS;
extern SD_HandleTypeDef hsd1;
extern FIL SDFile;
extern char SDPath[4];

void WriteToSDCard(void)
{
    FRESULT res;         // Результат операции
    UINT bytesWritten;   // Количество записанных байтов
    const char *testMessage = "Hello, SD Card!\r\n"; // \r\n для переноса строки
    uint8_t buffer[512];

    // Пример чтения первого сектора (опционально, если требуется проверка MBR)
    if (HAL_SD_ReadBlocks(&hsd1, buffer, 0, 1, HAL_MAX_DELAY) == HAL_OK) {
        // Прочитан первый сектор (MBR)
    } else {
        // Здесь зависание или ошибка
    }

    // 1. Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    if (res != FR_OK) {
        // Обработка ошибки
        return;
    }

    // 2. Открываем (или создаём, если нет) файл для записи.
    //    FA_OPEN_ALWAYS открывает файл, если он существует,
    //    иначе создаёт новый.
    //    FA_WRITE разрешает запись.
    res = f_open(&SDFile, "0:/test.txt", FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        // Обработка ошибки
        f_mount(NULL, "", 1);
        return;
    }

    // 3. Переходим в конец файла (для записи "в конец" - append)
    res = f_lseek(&SDFile, f_size(&SDFile));
    if (res != FR_OK) {
        // Если не удалось перейти в конец, выводим ошибку и выходим
        f_close(&SDFile);
        f_mount(NULL, "", 1);
        return;
    }

    // 4. Записываем данные
    res = f_write(&SDFile, testMessage, strlen(testMessage), &bytesWritten);
    if ((res != FR_OK) || (bytesWritten < strlen(testMessage))) {
        // Обработка ошибки записи
    }

    // 5. Закрываем файл
    f_close(&SDFile);

    // 6. Отмонтируем файловую систему
    f_mount(NULL, "", 1);
}



