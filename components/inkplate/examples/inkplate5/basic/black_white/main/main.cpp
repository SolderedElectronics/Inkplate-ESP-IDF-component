/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive black & white graphics demo for Soldered Inkplate 5.
 *
 * @details     Cycles through Adafruit GFX drawing primitives in 1-bit
 *              (black & white) mode: pixels, lines, thick lines, grids,
 *              rectangles, circles, rounded rectangles, triangles, ellipses,
 *              bitmaps, and text at multiple sizes. Each shape is shown for
 *              5 seconds. The demo ends with text that rotates indefinitely.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      logo.h header with logo bitmap included in the project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Build and flash to Inkplate 5.
 * 2) The display cycles through all drawing primitives automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in
 *   black & white.
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

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE5
#error \
    "Wrong board selection for this example, please select Inkplate5 in the boards menu."
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
  display.setCursor(20, 680);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.clearDisplay();
  display.display();

  display.setCursor(200, 340);
  display.setTextSize(4);
  display.print("Welcome to Inkplate 5!");
  display.display();
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

  while (true) {
    // Single pixel
    display.clearDisplay();
    displayCurrentAction(display, "Drawing a pixel");
    display.drawPixel(640, 360, BLACK);
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random pixels
    display.clearDisplay();
    for (int i = 0; i < 600; i++)
      display.drawPixel(random(0, 1279), random(0, 719), BLACK);
    displayCurrentAction(display, "Drawing random pixels");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 1279, 719, BLACK);
    display.drawLine(1279, 0, 0, 719, BLACK);
    displayCurrentAction(display, "Diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawLine(random(0, 1279), random(0, 719), random(0, 1279),
                       random(0, 719), BLACK);
    displayCurrentAction(display, "50 random lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Thick lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawThickLine(random(0, 1279), random(0, 719), random(0, 1279),
                            random(0, 719), BLACK, (float)random(1, 20));
    displayCurrentAction(display, "50 random thick lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(200, 360, 880, BLACK);
    displayCurrentAction(display, "Horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(640, 100, 520, BLACK);
    displayCurrentAction(display, "Vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Grid
    display.clearDisplay();
    for (int i = 0; i < 1280; i += 10)
      display.drawFastVLine(i, 0, 720, BLACK);
    for (int i = 0; i < 720; i += 10)
      display.drawFastHLine(0, i, 1280, BLACK);
    displayCurrentAction(display, "Grid");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rectangle
    display.clearDisplay();
    display.drawRect(390, 210, 500, 300, BLACK);
    displayCurrentAction(display, "Rectangle outline");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled rectangle
    display.clearDisplay();
    display.fillRect(390, 210, 500, 300, BLACK);
    displayCurrentAction(display, "Filled rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Circle + filled circle
    display.clearDisplay();
    display.drawCircle(640, 360, 150, BLACK);
    display.fillCircle(640, 360, 80, BLACK);
    displayCurrentAction(display, "Circle + filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(390, 210, 500, 300, 20, BLACK);
    display.fillRoundRect(420, 240, 440, 240, 25, BLACK);
    displayCurrentAction(display, "Rounded rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Triangle
    display.clearDisplay();
    display.drawTriangle(390, 600, 640, 100, 890, 600, BLACK);
    display.fillTriangle(490, 520, 640, 200, 790, 520, BLACK);
    displayCurrentAction(display, "Triangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Logo (800x160, centered at y=280)
    display.clearDisplay();
    display.image.draw(logo, 240, 280, logo_w, logo_h, BLACK);
    displayCurrentAction(display, "Logo");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Text in different sizes
    display.clearDisplay();
    for (int i = 0; i < 5; i++) {
      display.setTextSize(i + 1);
      display.setCursor(100, (i * i * 20) + 10);
      display.print("Inkplate 5");
    }
    displayCurrentAction(display, "Text sizes");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Ellipse
    display.clearDisplay();
    display.drawElipse(400, 200, 640, 360, BLACK);
    display.fillElipse(200, 100, 640, 360, BLACK);
    displayCurrentAction(display, "Ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rotate text forever
    int r = 0;
    display.setTextSize(5);
    display.setTextColor(BLACK, WHITE);
    while (true) {
      display.clearDisplay();
      display.setCursor(200, 300);
      display.setRotation(r);
      display.print("Inkplate 5");
      display.display();
      r++;
      vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
  }
}
