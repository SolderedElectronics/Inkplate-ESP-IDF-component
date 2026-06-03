/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Partial e-paper update with ESP32 deep sleep for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to correctly use partial screen updates together
 *              with ESP32 deep sleep on Inkplate 6 Flick. Since partial updates
 *              rely on previously stored screen content in RAM (which is erased
 *              during deep sleep), the screen must be recreated after waking up
 *              before calling partialUpdate(). This example shows how to preserve
 *              variables in RTC memory, rebuild the screen, and safely perform
 *              partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) After the first full refresh, the device will enter deep sleep.
 * 3) Every 10 seconds the ESP32 wakes up, updates the variables,
 *    rebuilds the screen buffer, and performs a partial update.
 * 4) Observe the changing values on the display after each wake cycle.
 *
 * Expected output:
 * - First boot performs a full refresh.
 * - Subsequent wake-ups perform partial updates only.
 * - Counter increments and decimal multiplies by 1.23 after each sleep cycle.
 *
 * Notes:
 * - Partial update works only in 1-bit (black & white) mode.
 * - Always rebuild the screen content after deep sleep before calling
 *   partialUpdate() — skipping preloadScreen() will corrupt the display.
 * - It is recommended to perform a full refresh every 5-10 partial updates
 *   to maintain good image quality.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "esp_attr.h"  // RTC_DATA_ATTR
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/rtc.h"   // rtc_get_reset_reason(), DEEPSLEEP_RESET

#include "Inkplate.h"

// Wake up every 10 seconds
#define TIME_TO_SLEEP_US (10ULL * 1000000ULL)

// Variables stored in RTC RAM — survive deep sleep, lost on power cycle
RTC_DATA_ATTR static int counter   = 0;
RTC_DATA_ATTR static float decimal = 3.14159265f;

// Build the screen content from the current variable values.
// This function is called both on first boot (to draw before display()) and
// after deep sleep (to recreate the previous frame before partialUpdate()).
static void createScreen(Inkplate *display) {
    display->setFont(NULL);
    display->setTextColor(BLACK, WHITE);

    // Title
    display->setTextSize(3);
    display->setCursor(200, 120);
    display->print("Inkplate 6 Flick - Partial Update + Sleep");
    display->drawFastHLine(10, 180, 1004, BLACK);

    // Counter value
    display->setTextSize(4);
    display->setCursor(200, 280);
    display->print("Counter:  ");
    display->print(counter);

    // Decimal value (2 decimal places)
    display->setCursor(200, 360);
    display->print("Decimal:  ");
    display->print(decimal, 2);

    // Footer
    display->setTextSize(2);
    display->setCursor(200, 680);
    display->print("Partial update every 10 s via deep sleep.");
}

extern "C" void app_main(void) {
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    // Draw the initial screen layout (with current variable values)
    createScreen(&display);

    if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET) {
        // Woken from deep sleep: recreate the old frame so the driver knows
        // what was on screen, then update the values and do a partial refresh.
        display.preloadScreen();

        counter++;
        decimal *= 1.23f;

        display.clearDisplay();
        createScreen(&display);

        // Forced partial update — use ONLY in this deep-sleep recreation scenario
        display.partialUpdate(true);
    } else {
        // First boot or manual reset: perform a full refresh
        display.display();
    }

    // Set timer wake-up and enter deep sleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    esp_deep_sleep_start();
}
