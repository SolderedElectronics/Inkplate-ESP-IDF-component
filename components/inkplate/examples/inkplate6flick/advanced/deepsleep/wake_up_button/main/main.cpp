/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Wake-up button and timer deep sleep example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to wake the ESP32 from deep sleep on Inkplate 6
 *              Flick using an external interrupt (WakeUp button, GPIO36) and a
 *              fallback timer. The example stores a boot counter in RTC memory,
 *              displays the boot count on the e-paper display, and prints the
 *              wake-up reason (button press vs. timer wake-up).
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
 * 2) After displaying boot info, the board enters deep sleep.
 * 3) Wake the board by pressing the WakeUp button, or wait 30 seconds for timer
 *    wake-up.
 * 4) On each wake, the display updates with the new boot count and wake-up reason.
 *
 * Expected output:
 * - Inkplate display shows an incrementing boot count.
 * - Wake-up reason is shown as either WakeUp button or timer.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - bootCount is stored in RTC memory (RTC_DATA_ATTR) so it persists across
 *   deep sleep cycles.
 * - WakeUp button uses EXT0 wake-up on GPIO36 (active low).
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

#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Fallback timer: wake up after 30 seconds if button is not pressed
#define TIME_TO_SLEEP_US (30ULL * 1000000ULL)

// Boot counter persists across deep sleep cycles
RTC_DATA_ATTR static int bootCount = 0;

// Display boot count and the reason for the most recent wake-up
static void displayInfo(Inkplate &display) {
    display.clearDisplay();

    display.setTextColor(BLACK, WHITE);
    display.setTextSize(3);

    // Title
    display.setCursor(10, 80);
    display.print("Inkplate 6 Flick - Deep Sleep Demo");

    // Divider line
    display.drawFastHLine(10, 130, 1004, BLACK);

    // Boot count
    display.setCursor(10, 170);
    display.setTextSize(4);
    display.print("Boot count: ");
    display.print(bootCount);

    // Wake-up reason
    display.setCursor(10, 260);
    display.setTextSize(3);
    switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0:
        display.print("Wake-up cause: WakeUp button (GPIO36)");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        display.print("Wake-up cause: Timer (30 s elapsed)");
        break;
    default:
        display.print("Wake-up cause: Power-on / reset");
        break;
    }

    // Hint
    display.setCursor(10, 680);
    display.setTextSize(2);
    display.print("Press the WakeUp button or wait 30 s to wake again.");

    display.display();
}

extern "C" void app_main(void) {
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    // Increment persistent boot counter
    ++bootCount;

    displayInfo(display);

    // Enable fallback timer wake-up (30 s)
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    // Enable wake-up from deep sleep via WakeUp button (GPIO36, active low)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

    esp_deep_sleep_start();
}
