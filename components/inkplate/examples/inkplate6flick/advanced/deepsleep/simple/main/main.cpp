/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Simple deep sleep slideshow example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates low-power operation on Inkplate 6 Flick using ESP32
 *              deep sleep. On each wake-up (timer-based), the board redraws the
 *              screen with the next slide in a small slideshow, performs a full
 *              display refresh, and then returns to deep sleep. Each slide is
 *              drawn using geometric primitives — no external image files are
 *              required.
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
 * 2) The board will show a slide, go to deep sleep, and wake up every 20 seconds.
 * 3) After each wake-up, the next slide is shown (loops through 3 slides).
 *
 * Expected output:
 * - Inkplate display cycles through 3 pattern slides every 20 seconds.
 * - The current slide number is printed in the top-left corner.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - RAM contents are lost during deep sleep, so RTC_DATA_ATTR is used to
 *   persist the slide index across sleep cycles.
 * - This example uses 1-bit (black and white) mode.
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
#include "esp_attr.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Wake up every 20 seconds
#define TIME_TO_SLEEP_US (20ULL * 1000000ULL)

// Screen dimensions (Inkplate 6 Flick)
#define SCREEN_W 1024
#define SCREEN_H 758

// Slide index persists across deep sleep cycles
RTC_DATA_ATTR static int slide = 0;

// Draw slide 0: horizontal stripes alternating black and white
static void drawSlide0(Inkplate *display) {
    const int stripeH = 60;
    for (int y = 0; y < SCREEN_H; y += stripeH) {
        uint8_t color = ((y / stripeH) % 2 == 0) ? BLACK : WHITE;
        display->fillRect(0, y, SCREEN_W, stripeH, color);
    }
}

// Draw slide 1: checkerboard pattern
static void drawSlide1(Inkplate *display) {
    const int cellSize = 80;
    for (int row = 0; row * cellSize < SCREEN_H; row++) {
        for (int col = 0; col * cellSize < SCREEN_W; col++) {
            uint8_t color = ((row + col) % 2 == 0) ? BLACK : WHITE;
            display->fillRect(col * cellSize, row * cellSize, cellSize, cellSize, color);
        }
    }
}

// Draw slide 2: concentric rectangles
static void drawSlide2(Inkplate *display) {
    int cx = SCREEN_W / 2;
    int cy = SCREEN_H / 2;
    int steps = 10;
    int stepW = cx / steps;
    int stepH = cy / steps;
    for (int i = 0; i < steps; i++) {
        uint8_t color = (i % 2 == 0) ? BLACK : WHITE;
        int x = cx - (steps - i) * stepW;
        int y = cy - (steps - i) * stepH;
        int w = 2 * (steps - i) * stepW;
        int h = 2 * (steps - i) * stepH;
        display->fillRect(x, y, w, h, color);
    }
}

extern "C" void app_main(void) {
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();

    // Draw the current slide
    switch (slide) {
    case 0:
        drawSlide0(&display);
        break;
    case 1:
        drawSlide1(&display);
        break;
    case 2:
        drawSlide2(&display);
        break;
    default:
        drawSlide0(&display);
        break;
    }

    // Print slide number in the top-left corner
    display.setTextColor(BLACK, WHITE);
    display.setTextSize(3);
    display.setCursor(10, 10);
    display.print("Slide: ");
    display.print(slide + 1);
    display.print(" / 3");

    display.display(); // Full refresh

    // Advance to the next slide (wraps around)
    slide++;
    if (slide > 2)
        slide = 0;

    // Enable timer wake-up and enter deep sleep
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    esp_deep_sleep_start();
}
