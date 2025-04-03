#include "fatfs.h"
#include "main.h"
#include "diskio.h"
#include <string.h>
#include "Settings.h"
#include "Status_codes.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"
#include <ctype.h>

extern EEPROM_Settings_item EEPROM;


void base62_encode(uint64_t value, char *buffer, size_t bufferSize);
void SD_write_log(char *string, char *path);
void Collect_DATA();
void remove_whitespace(char *str);
void SETTINGS_REQUEST_DATA();
