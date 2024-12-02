#include "SD.h"
#include "main.h"
#include "diskio.h"

extern FATFS SDFatFS;
extern SD_HandleTypeDef hsd1;
extern FIL SDFile;
void WriteToSDCard(void)
{
    extern char SDPath[4];
    FRESULT res;         // Результат операции
    UINT bytesWritten;   // Количество записанных байтов
    const char *testMessage = "Hello, SD Card!";

    if (HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B) != HAL_OK) {
    // Обработка ошибки
    return;
    }
    
    // 1. Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    if (res != FR_OK) {
        // Обработка ошибки
        //printf("Failed to mount SD card (Error %d)\n", res);
        return;
    }

    // 2. Открываем или создаем файл на запись
    char path[13] = "test.txt";
    path[12] = '\0';
    res = f_open(&SDFile, (char*)path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        // Обработка ошибки
        //printf("Failed to open file (Error %d)\n", res);
        f_mount(NULL, "", 1); // Отмонтируем файловую систему
        return;
    }

    // 3. Записываем данные
    res = f_write(&SDFile, testMessage, strlen(testMessage), &bytesWritten);
    if (res != FR_OK || bytesWritten < strlen(testMessage)) {
        // Обработка ошибки записи
        //printf("Failed to write data to file (Error %d)\n", res);
    } else {
        //printf("Data written successfully!\n");
    }

    // 4. Закрываем файл
    f_close(&SDFatFS);

    // 5. Отмонтируем файловую систему
    f_mount(NULL, "", 1);
}
