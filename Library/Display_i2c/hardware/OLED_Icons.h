#ifndef ICONS_H_
#define ICONS_H_

#include <stdint.h>

#define NULL_ICON (void*)0

/* Icon in XBM */
extern const uint8_t icon_clock[];
extern const uint8_t image_bits[];

extern const uint8_t signal_0[];
extern const uint8_t signal_1[];
extern const uint8_t signal_2[];
extern const uint8_t signal_3[];
extern const uint8_t no_signal[];
extern const uint8_t akb[];
extern const uint8_t USB_XMB[];
extern const uint8_t QR[];
extern const uint8_t CR_XMB[];
extern const uint8_t ScreenSaver_Static[];
extern const uint16_t frame_delays[];

#define FRAME_COUNT 23
#define FRAME_BYTES_PER_FRAME 1026
extern const uint8_t frames[FRAME_COUNT][FRAME_BYTES_PER_FRAME];

#endif /* ICONS_H_ */
