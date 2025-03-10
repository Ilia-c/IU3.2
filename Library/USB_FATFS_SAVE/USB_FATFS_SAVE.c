#include "USB_FATFS_SAVE.h"


/* Глобальные переменные для USB Host и FATFS */
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
FATFS USBFatFs;    /* Объект файловой системы для USB диска */
FIL MyFile;        /* Объект файла */
char USBPath[4] = "0:";  /* Логический путь USB диска */

/* Прототипы функций */
int Init_USB(void);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
static void Process_USB_Flash(void);

/* Основной пример работы */
int Init_USB(void)
{

    /* Инициализация USB Host */
    USBH_Init(&hUsbHostFS, USBH_UserProcess, 0);
    USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS);
    USBH_Start(&hUsbHostFS);
    USBH_Process(&hUsbHostFS);
    Process_USB_Flash();
}

/* Callback-функция для обработки событий USB Host */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
        break;
    case HOST_USER_CONNECTION:
        Appli_state = APPLICATION_START;
        break;
    case HOST_USER_CLASS_ACTIVE:
        Appli_state = APPLICATION_READY;
        break;
    case HOST_USER_DISCONNECTION:
        Appli_state = APPLICATION_DISCONNECT;
        break;
    default:
        break;
    }
}

/* Функция Process_USB_Flash:
   1. Монтирует файловую систему с USB накопителя.
   2. Если накопитель готов (Appli_state == APPLICATION_READY), открывает/создает файл "example.txt" 
      и записывает в него тестовую строку.
   3. Отмонтирует файловую систему.
*/
static void Process_USB_Flash(void)
{
    FRESULT res;
    UINT bw;
    char text[] = "Hello, USB Flash!\r\n";

    /* Монтирование файловой системы на USB накопителе */
    res = f_mount(&USBFatFs, USBPath, 0);
    if(res != FR_OK)
    {
        // Если файловую систему не смонтировать, выходим
        return;
    }

    /* Открываем/создаем файл "example.txt" для записи */
    res = f_open(&MyFile, "example.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if(res == FR_OK)
    {
        res = f_write(&MyFile, text, strlen(text), &bw);
        f_close(&MyFile);
        if(res == FR_OK)
        {
            // Запись прошла успешно; выводим сообщение (при правильно настроенном printf, например, через UART)
            printf("File written successfully on USB Flash.\r\n");
        }
        else
        {
            printf("Error writing file on USB Flash.\r\n");
        }
    }
    else
    {
        printf("Error opening file on USB Flash.\r\n");
    }

    /* Отмонтируем файловую систему */
    f_mount(NULL, USBPath, 0);
}