/**
 **************************************************
 * @file        main.cpp
 * @brief       Partial e-paper update with ESP32 deep sleep — ESP-IDF port.
 *
 * @details     Inkplate is instantiated locally in app_main() and passed by
 *              pointer to all helper functions. No global Inkplate object.
 *
 *              RTC_DATA_ATTR variables survive deep sleep and are used to
 *              recreate the screen buffer before calling partialUpdate().
 *
 * Requirements:
 * - ESP-IDF >= 5.x
 * - Board: Soldered Inkplate 10
 *
 * Notes:
 * - Partial update works only in 1-bit (B/W) mode.
 * - Always call preloadScreen() + rebuild before partialUpdate() after deep sleep.
 * - Perform a full refresh every 5–10 partial updates to maintain image quality.
 *
 * @author      Ported from Arduino sketch by Soldered
 * @license     GNU GPL V3
 **************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_attr.h"       // RTC_DATA_ATTR
#include "rom/rtc.h"        // rtc_get_reset_reason(), DEEPSLEEP_RESET

#include "Inkplate.h"

#define TIME_TO_SLEEP_US (10ULL * 1000000ULL)   // 10 seconds in microseconds

/* ── Variables stored in RTC RAM — survive deep sleep ───────────────────── */
RTC_DATA_ATTR static int   counter = 0;
RTC_DATA_ATTR static float decimal = 3.14159265f;   // M_PI equivalent

/* ── Forward declarations ───────────────────────────────────────────────── */
static void createScreen(Inkplate *display);

/* ═══════════════════════════════════════════════════════════════════════════
 * app_main  —  called on every boot / wake-up
 * ═══════════════════════════════════════════════════════════════════════════ */
extern "C" void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    createScreen(&display);

    if (rtc_get_reset_reason(0) == DEEPSLEEP_RESET)
    {
        /* Woken from deep sleep:
         * 1. Restore the framebuffer to match what's physically on the panel.
         * 2. Update variables.
         * 3. Rebuild the screen with new values.
         * 4. Partial update — only changed pixels are redrawn. */
        display.preloadScreen();

        counter++;
        decimal *= 1.23f;

        display.clearDisplay();
        createScreen(&display);
        display.partialUpdate(true);
    }
    else
    {
        /* First boot or manual reset — full refresh */
        display.display();
    }

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    esp_deep_sleep_start();
}

/* ═══════════════════════════════════════════════════════════════════════════
 * createScreen
 *   Draws the current state of all variables into the framebuffer.
 *   Must be called both before the initial display() AND before every
 *   partialUpdate() after deep sleep.
 *
 *   @param display  Pointer to the caller-owned Inkplate object.
 * ═══════════════════════════════════════════════════════════════════════════ */
static void createScreen(Inkplate *display)
{
    display->setFont(NULL);
    display->setTextSize(3);
    display->setTextColor(BLACK, WHITE);

    display->setCursor(200, 300);
    display->print("First variable:");
    display->print(counter, 10);

    display->setCursor(200, 340);
    display->print("Second variable:");
    display->print(decimal, 2);
}