#include "fatfs.h"
#include "main.h"
#include "diskio.h"
#include <string.h>
#include "Settings.h"
#include "Status_codes.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"
#include <ctype.h>

void WriteToSDCard(void);
void SD_check();
void base62_encode(uint64_t value, char *buffer, size_t bufferSize);
void SETTINGS_REQUEST_DATA();
