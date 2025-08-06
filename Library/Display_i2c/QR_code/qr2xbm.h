#ifndef QR2XBM_H_
#define QR2XBM_H_

#include "OLED_Icons.h"
#include "qrcodegen.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CANVAS_SIZE      45    // фиксированный холст 41?41
#define QR_VERSION        5    // версия QR: 4 ? 33?33 модулей
#define MARGIN_MODS     ((CANVAS_SIZE - (4*QR_VERSION + 17)) / 2)
#define MAX_TEXT         84    // макс. длина строки
#define BYTES_PER_ROW   ((CANVAS_SIZE + 7) / 8)
#define DATA_BYTES      (CANVAS_SIZE * BYTES_PER_ROW)
#define QR_BUFFER_LEN    qrcodegen_BUFFER_LEN_FOR_VERSION(QR_VERSION)


int QR_create(char *text);


#endif