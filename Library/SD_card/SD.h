#include "fatfs.h"
#include "main.h"
#include "diskio.h"
#include <string.h>
#include "Settings.h"
#include "Status_codes.h"

void WriteToSDCard(void);
void SD_check();
void uint64_to_hex_str(uint64_t value, char *buffer, size_t bufferSize);