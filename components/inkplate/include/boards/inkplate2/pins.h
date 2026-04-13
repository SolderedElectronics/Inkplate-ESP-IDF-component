#ifndef _INKPLATE2_PINS_H_
#define _INKPLATE2_PINS_H_

#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

// Color Palette of the Inkplate 2 panel
static uint32_t pallete[] = {0xFFFFFF, 0x000000, 0xFF0000};

#define EPAPER_RST_PIN  GPIO_NUM_19
#define EPAPER_DC_PIN   GPIO_NUM_33
#define EPAPER_CS_PIN   GPIO_NUM_27
#define EPAPER_BUSY_PIN GPIO_NUM_32
#define EPAPER_CLK      GPIO_NUM_18
#define EPAPER_DIN      GPIO_NUM_23

#define BUSY_TIMEOUT_MS 1000

// in defines.h
#define INKPLATE2_WHITE 0
#define INKPLATE2_BLACK 1
#define INKPLATE2_RED   2

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                                                            \
    {                                                                                                                  \
        int16_t t = a;                                                                                                 \
        a = b;                                                                                                         \
        b = t;                                                                                                         \
    }
#endif

#endif
