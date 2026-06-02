/**
 **************************************************
 * @file        Inkplate4TEMPERA_Partial_Update_With_Deep_Sleep.ino
 * @brief       Partial e-paper update with ESP32 deep sleep for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to correctly use partial screen updates together
 *              with ESP32 deep sleep on Inkplate 4TEMPERA. Since partial updates rely
 *              on previously stored screen content in RAM, the screen must be
 *              recreated after waking from deep sleep before calling
 *              partialUpdate(). This example shows how to preserve variables in
 *              RTC memory, rebuild the screen, and safely perform partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - Serial settings: 115200 baud
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Upload the sketch to Inkplate 4TEMPERA.
 * 2) After first full refresh, the device will enter deep sleep.
 * 3) Every 10 seconds the ESP32 wakes up, updates variables,
 *    rebuilds the screen buffer, and performs a partial update.
 * 4) Observe changing values on the display after each wake cycle.
 *
 * Expected output:
 * - First boot performs a full refresh.
 * - Subsequent wake-ups perform partial updates only.
 * - Counter and decimal value increment after each deep sleep cycle.
 *
 * Notes:
 * - Partial update works only in 1-bit (black & white) mode.
 * - Do NOT use standard partial update examples together with deep sleep.
 * - Always rebuild the screen content after deep sleep before calling partialUpdate().
 * - It is recommended to perform a full refresh every 5–10 partial updates
 *   to maintain good image quality.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2021-02-11
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Soldered Inkplate4TEMPERA in the boards menu."
#endif

#include "Inkplate.h"      // Include Inkplate library to the sketch
#include "driver/rtc_io.h" // Include ESP32 library for RTC pin I/O (needed for rtc_gpio_isolate() function)
#include <rom/rtc.h>       // Include ESP32 library for RTC (needed for rtc_get_reset_reason() function)

#define uS_TO_S_FACTOR 1000000ULL // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  10      // Time how long ESP32 will be in deep sleep (in seconds). In this case 10 seconds.

Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1-bit mode (BW)

// Counter variable (stored in RTC RAM that stores variable even if deep sleep is used)
// Variables that are changed after each partial update has to be stored in RTC RAM in order to recreate screen before
// deep sleep
RTC_DATA_ATTR int counter = 0;
RTC_DATA_ATTR float decimal = PI;

void setup()
{
    display.begin(); // Init Inkplate library (you should call this function ONLY ONCE)
    createScreen();  // Function that contains everything that has to be written on screen

    if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) // Check if ESP32 is reseted by deep sleep or power up / user manual
                                                    // reset (or some other reason)
    {
        display.preloadScreen(); // If is woken up by deep sleep, recreate whole screen to be same as was before deep sleep
        counter++;            // Update variable / variables
        decimal *= 1.23;
        display.clearDisplay();      // Clear everything in buffer
        createScreen();              // Create new screen with new variables
        display.partialUpdate(true); // Partial update of screen. (Use this only in this
                                     // scenario, otherwise YOU CAN DAMAGE YOUR SCRREN)
    }
    else // If is not deep sleep reset, that must be some thing else, so use normal update procerude (full screen
         // update)
    {
        display.display();
    }
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP *
                                  uS_TO_S_FACTOR); // Set EPS32 to be woken up in 10 seconds (in this case)

    esp_deep_sleep_start(); // Put ESP32 into deep sleep (low power mode)
}

void loop()
{
    // Nothing should be here while using deep sleep!
}

void createScreen()
{
    display.setCursor(100, 250);      // Set text cursor @ X = 100, Y = 250
    display.setTextSize(3);           // Set font to be scaled up three times
    display.print("First variable:"); // Write first variable to buffer
    display.print(counter, DEC);
    display.setCursor(100, 300);       // Set text cursor @ X = 100, Y = 250
    display.print("Second variable:"); // Write second variable to buffer (use two decimals places)
    display.print(decimal, 2);
}
