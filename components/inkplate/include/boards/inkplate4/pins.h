#ifndef _INKPLATE4_PINS_H_
#define _INKPLATE4_PINS_H_

#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "PCAL.h"

extern PCAL expander1;

#define IO_INT_ADDR  0x20
#define IO_EXT_ADDR  0x21

// pin on the internal io expander which controls MOSFET for turning on and off the SD card
#define SD_PMOS_PIN  IO_NUM_B3

// enables the GPIO0/CL level shifter
#define GPIO0_ENABLE IO_NUM_B0

#define FRONTLIGHT   IO_NUM_B2

#define WAKEUP       IO_NUM_A3
#define WAKEUP_SET   do { expander1.setLevel(WAKEUP, 1, true); } while(0)
#define WAKEUP_CLEAR do { expander1.setLevel(WAKEUP, 0, true); } while(0)

#define PWRUP        IO_NUM_A4
#define PWRUP_SET    do { expander1.setLevel(PWRUP, 1, true); } while(0)
#define PWRUP_CLEAR  do { expander1.setLevel(PWRUP, 0, true); } while(0)

#define VCOM         IO_NUM_A5
#define VCOM_SET     do { expander1.setLevel(VCOM, 1, true); } while(0)
#define VCOM_CLEAR   do { expander1.setLevel(VCOM, 0, true); } while(0)

#define OE           IO_NUM_A0
#define OE_SET       do { expander1.setLevel(OE, 1, true); } while(0)
#define OE_CLEAR     do { expander1.setLevel(OE, 0, true); } while(0)

#define GMOD         IO_NUM_A1
#define GMOD_SET     do { expander1.setLevel(GMOD, 1, true); } while(0)
#define GMOD_CLEAR   do { expander1.setLevel(GMOD, 0, true); } while(0)

#define SPV          IO_NUM_A2
#define SPV_SET      do { expander1.setLevel(SPV, 1, true); } while(0)
#define SPV_CLEAR    do { expander1.setLevel(SPV, 0, true); } while(0)

#define CL           0x01
#define CL_SET       do { GPIO.out_w1ts = CL; } while(0)
#define CL_CLEAR     do { GPIO.out_w1tc = CL; } while(0)

#define CKV          0x01
#define CKV_SET      do { GPIO.out1_w1ts.val = CKV; } while(0)
#define CKV_CLEAR    do { GPIO.out1_w1tc.val = CKV; } while(0)

#define SPH          0x02
#define SPH_SET      do { GPIO.out1_w1ts.val = SPH; } while(0)
#define SPH_CLEAR    do { GPIO.out1_w1tc.val = SPH; } while(0)

#define LE           0x04
#define LE_SET       do { GPIO.out_w1ts = LE; } while(0)
#define LE_CLEAR     do { GPIO.out_w1tc = LE; } while(0)


#define DATA 0x0E8C0030

#endif
