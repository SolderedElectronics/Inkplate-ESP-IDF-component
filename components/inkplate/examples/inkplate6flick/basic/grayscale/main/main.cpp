/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Comprehensive grayscale graphics demo for Soldered Inkplate 6 Flick.
 *
 * @details     Cycles through Adafruit GFX drawing primitives in 3-bit
 *              grayscale mode (8 shades, 0 = black, 7 = white). Includes
 *              pixels, lines, thick lines, gradient lines, grids, rectangles,
 *              circles, rounded rectangles, triangles, grayscale bitmap images,
 *              ellipses, polygons, and text in different sizes and shades.
 *              Each shape is shown for 5 seconds. The demo ends with rotating
 *              text.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      image.h header with a grayscale image array included in the
 *               project
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) The display cycles through all drawing primitives in grayscale
 *    automatically.
 *
 * Expected output:
 * - A series of screens demonstrating pixels, lines, shapes, and text in
 *   8-shade grayscale, ending with rotating text.
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

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "image_ex.h"
#include "math.h"

#define DELAY_MS 5000

static const int picture_w = 500;
static const int picture_h = 332;

int random(int min, int max) { return (esp_random() % (max - min + 1)) + min; }

void displayCurrentAction(Inkplate &display, const char *text) {
  display.setTextSize(2);
  display.setTextColor(0, 7);
  display.setCursor(2, 738);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(GRAYSCALE);
  display.setTextColor(0, 7);
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
    display.drawPixel(100, 50, 0);
    displayCurrentAction(display, "Drawing a pixel");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Random pixels in random shades
    display.clearDisplay();
    for (int i = 0; i < 1000; i++)
      display.drawPixel(random(0, 1023), random(0, 757), random(0, 7));
    displayCurrentAction(display, "Drawing 1000 random pixels in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Diagonal lines
    display.clearDisplay();
    display.drawLine(0, 0, 1023, 757, 0);
    display.drawLine(1023, 0, 0, 757, 0);
    displayCurrentAction(display, "Drawing two diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // 100 random lines in random shades
    display.clearDisplay();
    for (int i = 0; i < 100; i++)
      display.drawLine(random(0, 1023), random(0, 757), random(0, 1023),
                       random(0, 757), random(0, 7));
    displayCurrentAction(display, "Drawing 100 random lines in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // 100 random thick lines
    display.clearDisplay();
    for (int i = 0; i < 100; i++)
      display.drawThickLine(random(0, 1023), random(0, 757), random(0, 1023),
                            random(0, 757), random(0, 7), (float)random(1, 20));
    displayCurrentAction(display, "Drawing 100 random thick lines in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // 100 random gradient lines
    display.clearDisplay();
    for (int i = 0; i < 100; i++) {
      int startColor = random(0, 7);
      int endColor = random(startColor, 7);
      display.drawGradientLine(random(0, 1023), random(0, 757), random(0, 1023),
                               random(0, 757), startColor, endColor,
                               (float)random(1, 20));
    }
    displayCurrentAction(display, "Drawing 100 random gradient lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Horizontal line
    display.clearDisplay();
    display.drawFastHLine(100, 100, 600, 0);
    displayCurrentAction(display, "Drawing one horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Vertical line
    display.clearDisplay();
    display.drawFastVLine(100, 100, 400, 0);
    displayCurrentAction(display, "Drawing one vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Colored grid
    display.clearDisplay();
    for (int i = 0; i < 1024; i += 8)
      display.drawFastVLine(i, 0, 758, (i / 8) & 0x07);
    for (int i = 0; i < 758; i += 4)
      display.drawFastHLine(0, i, 1024, (i / 8) & 0x07);
    displayCurrentAction(display, "Drawing a grid in different shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rectangle
    display.clearDisplay();
    display.drawRect(200, 200, 400, 300, 0);
    displayCurrentAction(display, "Drawing rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many rectangles in random shades
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawRect(random(0, 1023), random(0, 757), 100, 150, random(0, 7));
    displayCurrentAction(display, "Drawing many rectangles in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Gray filled rectangle
    display.clearDisplay();
    display.fillRect(200, 200, 400, 300, 4);
    displayCurrentAction(display, "Drawing gray rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled rectangles in random shades
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.fillRect(random(0, 1023), random(0, 757), 30, 30, random(0, 7));
    displayCurrentAction(display, "Drawing many filled rectangles in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Circle
    display.clearDisplay();
    display.drawCircle(400, 300, 75, 0);
    displayCurrentAction(display, "Drawing a circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many circles in random shades
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
      display.drawCircle(random(0, 1023), random(0, 757), 25, random(0, 7));
    displayCurrentAction(display, "Drawing many circles in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled circle
    display.clearDisplay();
    display.fillCircle(400, 300, 75, 0);
    displayCurrentAction(display, "Drawing black-filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled circles in random shades
    display.clearDisplay();
    for (int i = 0; i < 40; i++)
      display.fillCircle(random(0, 1023), random(0, 757), 15, random(0, 7));
    displayCurrentAction(display, "Drawing many filled circles in random shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rounded rectangle
    display.clearDisplay();
    display.drawRoundRect(200, 200, 400, 300, 10, 0);
    displayCurrentAction(display, "Drawing rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.drawRoundRect(random(0, 1023), random(0, 757), 100, 150, 5,
                            random(0, 7));
    displayCurrentAction(display, "Drawing many rounded rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled rounded rectangle
    display.clearDisplay();
    display.fillRoundRect(200, 200, 400, 300, 10, 0);
    displayCurrentAction(display, "Drawing filled rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Many filled rounded rectangles
    display.clearDisplay();
    for (int i = 0; i < 50; i++)
      display.fillRoundRect(random(0, 1023), random(0, 757), 30, 30, 3,
                            random(0, 7));
    displayCurrentAction(display, "Drawing many filled rounded rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Triangle + filled triangle
    display.clearDisplay();
    display.drawTriangle(250, 400, 550, 400, 400, 100, 0);
    display.fillTriangle(300, 350, 500, 350, 400, 150, 0);
    displayCurrentAction(display, "Drawing filled triangle inside existing one");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Grayscale image (500x332, positioned at x=100, y=100)
    display.clearDisplay();
    display.image.draw(picture1, 100, 100, picture_w, picture_h, 0);
    displayCurrentAction(display, "Drawing a grayscale image");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Text in different sizes and shades
    display.clearDisplay();
    for (int i = 0; i < 6; i++) {
      display.setTextColor(i);
      display.setTextSize(i + 1);
      display.setCursor(200, (i * i * 8));
      display.print("INKPLATE 6FLICK!");
    }
    displayCurrentAction(display, "Text in different sizes and shades");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Inverted text on top (no clearDisplay)
    display.setTextColor(7, 0);
    for (int i = 0; i < 6; i++) {
      display.setTextSize(i + 1);
      display.setCursor(200, 300 + (i * i * 8));
      display.print("INKPLATE 6FLICK!");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    display.setTextColor(0, 7);

    // Ellipse
    display.clearDisplay();
    display.drawElipse(100, 200, 400, 300, 0);
    displayCurrentAction(display, "Drawing an ellipse");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Filled ellipse
    display.clearDisplay();
    display.fillElipse(100, 200, 400, 300, 0);
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
    display.drawPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a polygon");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    display.clearDisplay();
    display.fillPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a filled polygon");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Rotate text indefinitely
    int r = 0;
    display.setTextSize(8);
    display.setTextColor(7, 0);
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
