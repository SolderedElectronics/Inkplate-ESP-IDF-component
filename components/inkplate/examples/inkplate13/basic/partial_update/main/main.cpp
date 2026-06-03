/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Partial update example for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates partial screen updates on the Inkplate 13SPECTRA.
 *              Draws a grid of 200x200 pixel colored squares covering the entire
 *              screen, then continuously picks a random square and updates only
 *              that square with a new color using displayPartial(), leaving the
 *              rest of the screen untouched.
 *
 *              displayPartial(x, y, w, h, leaveOn) accepts coordinates in the
 *              same user space as all drawing functions (1600 px wide, 1200 px
 *              tall).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Build and flash to Inkplate 13SPECTRA.
 * 2) Initial color grid is drawn with a full refresh.
 * 3) Random squares update every 3 seconds using partial refresh.
 *
 * Expected output:
 * - 8x6 grid of colored squares, randomly changing one at a time.
 *
 * Notes:
 * - displayPartial() refreshes only the specified region of the screen.
 * - leaveOn=true keeps panel power rails on between updates for faster refresh.
 * - display.display() must be called to update the physical e-paper panel.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "partial_update";

#define DISPLAY_W   1600
#define DISPLAY_H   1200
#define SQUARE_SIZE 200
#define GRID_COLS   (DISPLAY_W / SQUARE_SIZE)
#define GRID_ROWS   (DISPLAY_H / SQUARE_SIZE)
#define COLOR_COUNT 6

static const uint8_t COLORS[COLOR_COUNT] = {
    INKPLATE_BLACK, INKPLATE_WHITE, INKPLATE_YELLOW,
    INKPLATE_RED,   INKPLATE_BLUE,  INKPLATE_GREEN,
};

static uint8_t squareColor[GRID_COLS][GRID_ROWS];

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();

  // Fill each square with an initial color, cycling through the palette
  for (int col = 0; col < GRID_COLS; col++) {
    for (int row = 0; row < GRID_ROWS; row++) {
      uint8_t color = COLORS[(col + row) % COLOR_COUNT];
      squareColor[col][row] = color;
      display.fillRect(col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE,
                       SQUARE_SIZE, color);
    }
  }

  display.display();

  ESP_LOGI(TAG, "Initial grid drawn. Waiting 5 seconds before partial updates begin.");
  vTaskDelay(pdMS_TO_TICKS(5000));

  while (true) {
    int col = esp_random() % GRID_COLS;
    int row = esp_random() % GRID_ROWS;

    uint8_t current = squareColor[col][row];
    uint8_t newColor;
    do {
      newColor = COLORS[esp_random() % COLOR_COUNT];
    } while (newColor == current);

    squareColor[col][row] = newColor;

    display.fillRect(col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE,
                     SQUARE_SIZE, newColor);

    display.displayPartial(col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE,
                           SQUARE_SIZE, true);

    ESP_LOGI(TAG, "Updated square (%d, %d) to colour %d", col, row, newColor);

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}
