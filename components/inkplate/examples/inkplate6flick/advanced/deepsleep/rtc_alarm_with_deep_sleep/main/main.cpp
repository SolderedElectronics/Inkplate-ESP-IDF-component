/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm wake-up with deep sleep for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to use the onboard RTC alarm interrupt to wake
 *              the Inkplate 6 Flick from ESP32 deep sleep. The RTC alarm is
 *              configured to trigger 10 seconds from the current time, waking
 *              the board, refreshing the e-paper display with the current
 *              weekday, date, and time, and then returning the system back to
 *              low-power deep sleep mode.
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
 * 2) On first boot, the RTC time and date are initialized if not already set.
 * 3) The current time and date are shown on the display.
 * 4) The board enters deep sleep and wakes up every 10 seconds using the RTC
 *    alarm.
 * 5) After each wake-up, the display is refreshed and the board goes back to
 *    sleep.
 *
 * Expected output:
 * - Inkplate display shows the current weekday, date, and time.
 * - Display refreshes automatically on every RTC alarm wake-up (~10 s interval).
 *
 * Notes:
 * - RTC alarm interrupt is connected to GPIO39 on Inkplate 6 Flick.
 * - The alarm flag must be cleared at the start of each wake cycle so the
 *   next alarm can trigger correctly.
 * - When using deep sleep, all application logic must be placed in app_main().
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

#include "driver/rtc_io.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/rtc.h"
#include "time.h"

#include "Inkplate.h"
#include "RTC.h"

// Helper: print a 2-digit number with leading zero if needed
static void print2Digits(Inkplate *display, uint8_t d) {
    if (d < 10)
        display->print('0');
    display->print(d, 10);
}

// Print the current date and time read from the RTC to the display buffer
static void printCurrentTime(Inkplate *display) {
    tm t = {};
    display->rtc.getTime(&t);

    display->setTextColor(BLACK, WHITE);

    // ---- Weekday ----
    display->setCursor(100, 220);
    display->setTextSize(4);
    switch (t.tm_wday) {
    case 0: display->print("Sunday");    break;
    case 1: display->print("Monday");    break;
    case 2: display->print("Tuesday");   break;
    case 3: display->print("Wednesday"); break;
    case 4: display->print("Thursday");  break;
    case 5: display->print("Friday");    break;
    case 6: display->print("Saturday");  break;
    default: display->print("---");      break;
    }

    // ---- Date (DD.MM.YYYY.) ----
    display->setCursor(100, 310);
    display->setTextSize(4);
    display->print(t.tm_mday);
    display->print('.');
    display->print(t.tm_mon);
    display->print('.');
    display->print(t.tm_year);
    display->print('.');

    // ---- Time (HH:MM:SS) ----
    display->setCursor(100, 460);
    display->setTextSize(6);
    print2Digits(display, t.tm_hour);
    display->print(':');
    print2Digits(display, t.tm_min);
    display->print(':');
    print2Digits(display, t.tm_sec);
}

extern "C" void app_main(void) {
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    // Clear any pending alarm flag from the previous cycle
    display.rtc.clearAlarmFlag();

    // Set RTC time only if it has not been configured yet
    if (!display.rtc.isSet()) {
        tm t = {};
        t.tm_hour = 13;
        t.tm_min  = 30;
        t.tm_sec  = 0;
        t.tm_mday = 5;
        t.tm_mon  = 12;
        t.tm_year = 2022;
        t.tm_wday = 1; // Monday

        display.rtc.setTime(t);
    }

    // Draw title
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(100, 80);
    display.print("Inkplate 6 Flick - RTC Alarm Sleep");
    display.drawFastHLine(10, 160, 1004, BLACK);

    // Draw current time and date
    printCurrentTime(&display);

    // Footer hint
    display.setTextSize(2);
    display.setCursor(100, 700);
    display.print("Wakes via RTC alarm every 10 seconds (GPIO39).");

    display.display();

    // Set the next RTC alarm 10 seconds from now
    time_t now;
    display.rtc.getTime(&now);
    display.rtc.setAlarmEpoch(now + 10);

    // Enable wake-up from deep sleep via RTC alarm interrupt (GPIO39, active low)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

    esp_deep_sleep_start();
}
