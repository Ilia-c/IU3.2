#include "USB_FATFS_SAVE.h"


/* Глобальные переменные для USB Host и FATFS */
extern USBH_HandleTypeDef hUsbHostFS;
extern ApplicationTypeDef Appli_state;
FATFS USBFatFs;    /* Объект файловой системы для USB диска */
FIL MyFile;        /* Объект файла */
extern char USBHPath[4];  /* Логический путь USB диска */
