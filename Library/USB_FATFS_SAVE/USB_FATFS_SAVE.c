#include "USB_FATFS_SAVE.h"


/* Глобальные переменные для USB Host и FATFS */
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
FATFS USBFatFs;    /* Объект файловой системы для USB диска */
FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */


/* Функция Process_USB_Flash:
   1. Монтирует файловую систему с USB накопителя.
   2. Если накопитель готов (Appli_state == APPLICATION_READY), открывает/создает файл "example.txt" 
      и записывает в него тестовую строку.
   3. Отмонтирует файловую систему.
*/
void Process_USB_Flash(void)
{
    FRESULT res;
    UINT bw;
    const char text[] = "Hello, USB Flash!\r\n";

    /* Монтирование файловой системы на USB накопителе */
    res = f_mount(&USBFatFs, USBHPath, 0);
    if(res != FR_OK)
    {
        // Если файловую систему не смонтировать, выходим
        return;
    }

    /* Открываем/создаем файл "example.txt" для записи */
    res = f_open(&MyFile, "1:example.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        res = f_write(&MyFile, text, strlen(text), &bw);
        f_close(&MyFile);
        if(res == FR_OK)
        {
            // Запись прошла успешно; выводим сообщение (при правильно настроенном printf, например, через UART)
        }
        else
        {
        }
    }
    else
    {

    }

    /* Отмонтируем файловую систему */
    f_mount(NULL, USBHPath, 0);
}