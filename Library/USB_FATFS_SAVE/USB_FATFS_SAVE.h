#include "main.h"
#include "Status_codes.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "fatfs.h"
#include "usbh_diskio.h"
#include <string.h>
#include <stdio.h>
#include "usb_host.h"
#include "diskio.h"
#include "Settings.h"
#include "cmsis_os.h"
#include "usbd_cdc_if.h"


void Process_USB_Flash(void);