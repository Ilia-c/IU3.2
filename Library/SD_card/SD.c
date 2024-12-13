#include "SD.h"

extern FATFS SDFatFS;
extern SD_HandleTypeDef hsd1;
extern FIL SDFile;
void WriteToSDCard(void)
{
    extern char SDPath[4];
    FRESULT res;         // Результат операции
    UINT bytesWritten;   // Количество записанных байтов
    const char *testMessage = "Hello, SD Card!";
    //if (HAL_SD_ConfigWideBusOperation(&hsd1, SDMMC_BUS_WIDE_4B) != HAL_OK) {
    //    // Если ошибка, проверьте линии, подтяжки и понизьте частоту
    //}
    uint8_t buffer[512];
if (HAL_SD_ReadBlocks(&hsd1, buffer, 0, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Прочитан первый сектор (MBR)
} else {
    // Здесь зависание или ошибка
}
    // 1. Монтируем файловую систему
    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
    if (res != FR_OK) {
        // Обработка ошибки
        //printf("Failed to mount SD card (Error %d)\n", res);
        return;
    }

    // 2. Открываем или создаем файл на запись
    //char path_to_file[13] = "test.txt";
    //path_to_file[12] = '\0';
    res = BSP_SD_GetCardState();
    res = disk_status(0);
    res = f_open(&SDFile, "0:/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
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
    f_close(&SDFile);

    // 5. Отмонтируем файловую систему
    f_mount(NULL, "", 1);
}


