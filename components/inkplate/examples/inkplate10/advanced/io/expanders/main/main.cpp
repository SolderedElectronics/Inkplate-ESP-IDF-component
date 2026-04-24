

#include "Inkplate.h"
#include "PCAL.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// We are going to use pin P1-7 (GPB7) = IO_NUM_B7 on both expanders.
// GPA0 = IO_NUM_A0 ... GPA7 = IO_NUM_A7
// GPB0 = IO_NUM_B0 ... GPB7 = IO_NUM_B7
#define LED_PIN IO_NUM_B7

// expander1 is the internal IO expander (addr IO_INT_ADDR = 0x20), declared
// as an extern in Inkplate10.h — it is owned by the board driver.
extern PCAL expander1;

// expander2 is the external IO expander (addr IO_EXT_ADDR = 0x21).
// Declare it here the same way expander1 is declared in the library.
extern PCAL expander2;


extern "C" void app_main(void)
{
    Inkplate display;

    // Configure LED pin as output on both IO expanders.
    // bypass = false is safe here because IO_NUM_B7 is not a restricted pin.
    expander2.setDirection(LED_PIN, IO_MODE_OUTPUT);  // external expander
    expander1.setDirection(LED_PIN, IO_MODE_OUTPUT);  // internal expander — only B1–B7 are safe!

    while (true)
    {
        // --- External IO Expander (expander2, addr 0x21) ---
        for (int i = 0; i < 5; i++)
        {
            expander2.setLevel(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            expander2.setLevel(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

        // --- Internal IO Expander (expander1, addr 0x20) ---
        // bypass = true is NOT needed for IO_NUM_B7 since it isn't a PMIC pin.
        // blockGpioPins() only blocks A0–A5 (WAKEUP, PWRUP, VCOM, OE, GMOD, SPV).
        for (int i = 0; i < 5; i++)
        {
            expander1.setLevel(LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            expander1.setLevel(LED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}