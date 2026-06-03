/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive black & white graphics demo for Soldered Inkplate 6 Flick.
 *
 * @details     Cycles through Adafruit GFX drawing primitives in 1-bit
 *              (black & white) mode: pixels, lines, thick lines, grids,
 *              rectangles, circles, rounded rectangles, triangles, ellipses,
 *              polygons, bitmaps, and text at multiple sizes. Each shape is
 *              shown for 5 seconds. The demo ends with text that rotates
 *              indefinitely.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      logo.h header with logo bitmap included in the project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) The display cycles through all drawing primitives automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in
 *   black & white, ending with rotating text.
 *
 * Notes:
 * - display.clearDisplay() clears only the internal framebuffer.
 * - display.display() must be called to update the physical e-paper panel.
 * - DELAY_MS controls how long each shape is shown (default 5000 ms).
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
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "logo.h"
#include "math.h"

#define DELAY_MS 5000

int random(int min, int max) { return (esp_random() % (max - min + 1)) + min; }

void displayCurrentAction(Inkplate &display, const char *text) {
  display.setTextSize(2);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(2, 738);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.clearDisplay();
  display.display();

  display.setCursor(150, 320);
  display.setTextSize(4);
  display.print("Welcome to Inkplate 6 Flick!");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(5000));

  while (true) {
    // Single pixel
    display.clearDisplay();
    displayCurrentAction(display, "Drawing a pixel");
    display.drawPixel(100, 50, BLACK);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random pixels
    display.clearDisplay();
    for (int i = 0; i < 600; i++)
      display.drawPixel(random(0, 1023), random(0, 757), BLACK);
    displayCurrentAction(display, "Drawing 600 random pixels");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 1023, 757, BLACK);
    display.drawLine(1023, 0, 0, 757, BLACK);
    displayCurrentAction(display, "Drawing two diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // 50 random lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawLine(random(0, 1023), random(0, 757), random(0, 1023),
                       random(0, 757), BLACK);
    displayCurrentAction(display, "Drawing 50 random lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // 50 random thick lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawThickLine(random(0, 1023), random(0, 757), random(0, 1023),
                            random(0, 757), BLACK, (float)random(1, 20));
    displayCurrentAction(display, "Drawing 50 random thick lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(100, 100, 600, BLACK);
    displayCurrentAction(display, "Drawing one horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(100, 100, 400, BLACK);
    displayCurrentAction(display, "Drawing one vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Grid
    display.clearDisplay();
    for (int i = 0; i < 1024; i += 8)
      display.drawFastVLine(i, 0, 758, BLACK);
    for (int i = 0; i < 758; i += 4)
      display.drawFastHLine(0, i, 1024, BLACK);
    displayCurrentAction(display, "Drawing a grid");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rectangle outline
    display.clearDisplay();
    display.drawRect(200, 200, 400, 300, BLACK);
    displayCurrentAction(display, "Drawing rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawRect(random(0, 1023), random(0, 757), 100, 150, BLACK);
    displayCurrentAction(display, "Drawing many rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled rectangle
    display.clearDisplay();
    display.fillRect(200, 200, 400, 300, BLACK);
    displayCurrentAction(display, "Drawing black rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.fillRect(random(0, 1023), random(0, 757), 30, 30, BLACK);
    displayCurrentAction(display, "Drawing many filled rectangles randomly");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Circle
    display.clearDisplay();
    display.drawCircle(400, 300, 75, BLACK);
    displayCurrentAction(display, "Drawing a circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many circles
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
      display.drawCircle(random(0, 1023), random(0, 757), 25, BLACK);
    displayCurrentAction(display, "Drawing many circles randomly");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled circle
    display.clearDisplay();
    display.fillCircle(400, 300, 75, BLACK);
    displayCurrentAction(display, "Drawing black-filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled circles
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
      display.fillCircle(random(0, 1023), random(0, 757), 15, BLACK);
    displayCurrentAction(display, "Drawing many filled circles randomly");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(200, 200, 400, 300, 10, BLACK);
    displayCurrentAction(display, "Drawing rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawRoundRect(random(0, 1023), random(0, 757), 100, 150, 5,
                            BLACK);
    displayCurrentAction(display, "Drawing many rounded rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled rounded rectangle
    display.clearDisplay();
    display.fillRoundRect(200, 200, 400, 300, 10, BLACK);
    displayCurrentAction(display, "Drawing filled rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.fillRoundRect(random(0, 1023), random(0, 757), 30, 30, 3, BLACK);
    displayCurrentAction(display, "Random filled rounded rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Triangle + filled triangle
    display.clearDisplay();
    display.drawTriangle(250, 400, 550, 400, 400, 100, BLACK);
    display.fillTriangle(300, 350, 500, 350, 400, 150, BLACK);
    displayCurrentAction(display, "Drawing filled triangle inside existing one");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Logo bitmap (900x181, positioned at x=62, y=288)
    display.clearDisplay();
    display.image.draw(logo, 62, 288, logo_w, logo_h, BLACK);
    displayCurrentAction(display, "Drawing Soldered logo");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Text in different sizes
    display.clearDisplay();
    for (int i = 0; i < 6; i++) {
      display.setTextSize(i + 1);
      display.setCursor(200, (i * i * 8));
      display.print("Inkplate 6 Flick!");
    }
    displayCurrentAction(display, "Text in different sizes");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Inverted text on top of the previous (no clearDisplay)
    display.setTextColor(WHITE, BLACK);
    for (int i = 0; i < 6; i++) {
      display.setTextSize(i + 1);
      display.setCursor(200, 300 + (i * i * 8));
      display.print("Inkplate 6 Flick!");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    display.setTextColor(BLACK, WHITE);

    // Ellipse
    display.clearDisplay();
    display.drawElipse(100, 200, 400, 300, BLACK);
    displayCurrentAction(display, "Drawing an ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled ellipse
    display.clearDisplay();
    display.fillElipse(100, 200, 400, 300, BLACK);
    displayCurrentAction(display, "Drawing a filled ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Polygon (CCW-sorted random points)
    int xt[10], yt[10];
    int n = 10;
    for (int i = 0; i < n; i++) {
      xt[i] = random(100, 700);
      yt[i] = random(100, 500);
    }
    for (int i = 0; i < n - 1; i++)
      for (int j = i + 1; j < n; j++)
        if (atan2(yt[j] - 300, xt[j] - 400) <
            atan2(yt[i] - 300, xt[i] - 400)) {
          int k = xt[i]; xt[i] = xt[j]; xt[j] = k;
          k = yt[i]; yt[i] = yt[j]; yt[j] = k;
        }

    display.clearDisplay();
    display.drawPolygon(xt, yt, n, BLACK);
    displayCurrentAction(display, "Drawing a polygon");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    display.clearDisplay();
    display.fillPolygon(xt, yt, n, BLACK);
    displayCurrentAction(display, "Drawing a filled polygon");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rotate text indefinitely
    int r = 0;
    display.setTextSize(8);
    display.setTextColor(WHITE, BLACK);
    while (true) {
      display.clearDisplay();
      display.setCursor(100, 100);
      display.setRotation(r);
      display.print("INKPLATE 6FLICK");
      display.display();
      r++;
      vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
  }
}
