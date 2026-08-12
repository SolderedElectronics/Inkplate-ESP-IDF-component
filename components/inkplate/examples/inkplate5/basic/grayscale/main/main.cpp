/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive grayscale graphics demo for Soldered Inkplate 5.
 *
 * @details     Cycles through Adafruit GFX drawing primitives in 3-bit
 *              grayscale mode (8 shades, 0 = black, 7 = white). Includes
 *              pixels, lines, thick lines, gradient lines, grids, rectangles,
 *              circles, rounded rectangles, triangles, grayscale bitmap images,
 *              and text in different sizes and shades. Each shape is shown for
 *              5 seconds. The demo ends with rotating text.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      image_ex.h header with a grayscale image array included in the
 *               project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Build and flash to Inkplate 5.
 * 2) The display cycles through all drawing primitives in grayscale
 *    automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in
 *   8-shade grayscale.
 *
 * Notes:
 * - Grayscale (3-bit) mode supports 8 shades: 0 (black) to 7 (white).
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
#include "image_ex.h"
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
  display.setDisplayMode(GRAYSCALE);
  display.setTextColor(0, 7);
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
    display.drawPixel(640, 360, 0); // 0 = black in GRAYSCALE mode
    displayCurrentAction(display, "Drawing a pixel");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random pixels in random shades
    display.clearDisplay();
    for (int i = 0; i < 1000; i++)
      display.drawPixel(random(0, 1279), random(0, 719), random(0, 7));
    displayCurrentAction(display, "Drawing 1000 random pixels in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 1279, 719, 0);
    display.drawLine(1279, 0, 0, 719, 0);
    displayCurrentAction(display, "Diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random lines in random shades
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawLine(random(0, 1279), random(0, 719), random(0, 1279),
                       random(0, 719), random(0, 7));
    displayCurrentAction(display, "50 random lines in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Thick lines in random shades
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawThickLine(random(0, 1279), random(0, 719), random(0, 1279),
                            random(0, 719), random(0, 7), (float)random(1, 20));
    displayCurrentAction(display, "50 random thick lines in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Gradient lines
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      int startColor = random(0, 7);
      int endColor = random(startColor, 7);
      display.drawGradientLine(random(0, 1279), random(0, 719), random(0, 1279),
                               random(0, 719), startColor, endColor,
                               (float)random(1, 20));
    }
    displayCurrentAction(display, "50 random gradient lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(200, 360, 880, 0);
    displayCurrentAction(display, "Horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(640, 100, 520, 0);
    displayCurrentAction(display, "Vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Colored grid
    display.clearDisplay();
    for (int i = 0; i < 1280; i += 8)
      display.drawFastVLine(i, 0, 720, (i / 8) & 0x07);
    for (int i = 0; i < 720; i += 8)
      display.drawFastHLine(0, i, 1280, (i / 8) & 0x07);
    displayCurrentAction(display, "Grid in different shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rectangle
    display.clearDisplay();
    display.drawRect(390, 210, 500, 300, 0);
    displayCurrentAction(display, "Rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled gray rectangle
    display.clearDisplay();
    display.fillRect(390, 210, 500, 300, 4);
    displayCurrentAction(display, "Gray filled rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Circle + filled circle
    display.clearDisplay();
    display.drawCircle(640, 360, 150, 0);
    display.fillCircle(640, 360, 80, 0);
    displayCurrentAction(display, "Circle + filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(390, 210, 500, 300, 20, 0);
    display.fillRoundRect(420, 240, 440, 240, 25, 0);
    displayCurrentAction(display, "Rounded rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Triangle
    display.clearDisplay();
    display.drawTriangle(390, 600, 640, 100, 890, 600, 0);
    display.fillTriangle(490, 520, 640, 200, 790, 520, 0);
    displayCurrentAction(display, "Triangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Grayscale image (500x332, centered)
    display.clearDisplay();
    display.image.draw(picture1, 390, 194, 500, 332, 0);
    displayCurrentAction(display, "Grayscale image");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Text in different sizes and shades
    display.clearDisplay();
    for (int i = 0; i < 6; i++) {
      display.setTextColor(i);
      display.setTextSize(i + 1);
      display.setCursor(70, (i * i * 16) + 10);
      display.print("Inkplate 5!");
    }
    displayCurrentAction(display, "Text in different sizes and shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Ellipse
    display.clearDisplay();
    display.drawElipse(400, 200, 640, 360, 0);
    display.fillElipse(200, 100, 640, 360, 0);
    displayCurrentAction(display, "Ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rotate text forever
    int r = 0;
    display.setTextSize(5);
    display.setTextColor(7, 0);
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
