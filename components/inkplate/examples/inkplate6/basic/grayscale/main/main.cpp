/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Grayscale (3-bit) drawing demo using Adafruit GFX on Soldered
 * Inkplate 6.
 *
 * @details     Demonstrates drawing and text rendering on the Inkplate 6 in
 *              3-bit grayscale mode using Adafruit GFX-compatible functions.
 *              In 3-bit mode the display supports 8 shades (0-7), where 0 is
 *              black, 7 is white, and values in between are gray levels.
 *              The example cycles through drawing primitives (pixels, lines,
 *              shapes, polygons), renders a grayscale bitmap, and shows text in
 *              different sizes and shades.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6, USB cable
 * - Extra:      Optional bitmap header file (e.g. image_ex.h)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6
 *
 * How to use:
 * 1) Build and flash to Inkplate 6.
 * 2) The example cycles through multiple drawing demonstrations.
 * 3) Each demo renders to the framebuffer and then updates the e-paper display.
 *
 * Expected output:
 * - A sequence of graphics demos: pixels, lines, rectangles, circles,
 * triangles, rounded rectangles, ellipses, polygons, bitmap drawing, and text
 * rendering.
 * - Final part continuously rotates and displays text.
 *
 * Notes:
 * - Inkplate library is compatible with Adafruit GFX drawing functions.
 * - Grayscale (3-bit) mode supports 8 shades: 0 (black) to 7 (white).
 * - Avoid refreshing the full display too often; long delays are used for demo
 * clarity.
 * - Partial update is primarily intended for 1-bit mode; see partial update
 * examples.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6 in the boards menu."
#endif

#include "Inkplate.h"
#include "image_ex.h" //Include image file that holds gray image data.

#include "esp_random.h"
#include "math.h"

// Helper to keep it clean
int random(int min, int max) { return (esp_random() % (max - min + 1)) + min; }

#define DELAY_MS 5000

void displayCurrentAction(Inkplate &display, const char *text) {
  display.setTextSize(2);
  display.setCursor(2, 580);
  display.print(text);
}

extern "C" void app_main(void) {
  Inkplate display;
  display.clearDisplay(); // Clear any data that may have been in (software)
                          // frame buffer.
                          //(NOTE! This does not clean image on screen, it only
                          // clears it in the frame buffer inside
                          // ESP32).
  display.setDisplayMode(GRAYSCALE);
  display.display(); // Clear everything that has previously been on a screen
  display.setTextColor(0, 7);
  display.setCursor(167, 305);
  display.setTextSize(4);
  display.print("Welcome to Inkplate 6!");
  display.display();                   // Write hello message
  vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

  while (true) {
    // Example will demostrate funcionality one by one. You always first set
    // everything in the frame buffer and afterwards you show it on the screen
    // using display.display().

    // Let's start by drawing pixel at x = 100 and y = 50 location
    display.clearDisplay(); // Clear everytning that is inside frame buffer in
                            // ESP32
    display.drawPixel(100, 50, 0); // Draw one black pixel at X = 100, Y = 50
                                   // position in 0 (BLACK) color
    displayCurrentAction(
        display,
        "Drawing a pixel"); // Function which writes small text at bottom left
                            // indicating what's currently done NOTE: you do not
                            // need displayCurrentAction function to use
                            // Inkplate!
    display.display(); // Send image to display. You need to call this one each
                       // time you want to transfer frame buffer to the screen.
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait a little bit

    // Now, let's draw some random pixels!
    display.clearDisplay(); // Clear everything that is inside frame buffer in
                            // ESP32
    for (int i = 0; i < 1000;
         i++) { // Write 1000 random colored pixels at random locations
      display.drawPixel(random(0, 799), random(0, 599),
                        random(0, 7)); // We are setting color of the pixels
                                       // using numbers from 0 to 7,
    } // where 0 means black, 7 white and gray is in between
    displayCurrentAction(display, "Drawing 600 random pixels in random colors");
    display.display(); // Write everything from frame buffer to screen
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS)); // Wait

    // Draw two diagonal lines accros screen
    display.clearDisplay();
    display.drawLine(
        0, 0, 799, 599,
        0); // All of those drawing fuctions originate from Adafruit GFX
            // library, so maybe you are already familiar
    display.drawLine(799, 0, 0, 599, 0); // with those. Arguments are: start X,
                                         // start Y, ending X, ending Y, color.
    displayCurrentAction(display, "Drawing two diagonal lines");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // And again, let's draw some random lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 100; i++) {
      display.drawLine(random(0, 799), random(0, 599), random(0, 799),
                       random(0, 599), random(0, 7));
    }
    displayCurrentAction(display, "Drawing 50 random lines in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Let's draw some random thick lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 100; i++) {
      display.drawThickLine(random(0, 799), random(0, 599), random(0, 799),
                            random(0, 599), random(0, 7), (float)random(1, 20));
    }
    displayCurrentAction(
        display, "Drawing 50 random lines in random colors and thickness");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Let's draw some random gradient thick lines on screen!
    display.clearDisplay();
    for (int i = 0; i < 100; i++) {
      int startColor = random(0, 7);
      int endColor = random(startColor, 7);
      display.drawGradientLine(random(0, 799), random(0, 599), random(0, 799),
                               random(0, 599), startColor, endColor,
                               (float)random(1, 20));
    }
    displayCurrentAction(
        display,
        "Drawing 50 random gradient lines in random colors and thickness");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Now draw one horizontal...
    display.clearDisplay();
    display.drawFastHLine(
        100, 100, 600,
        0); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction(display, "Drawing one horizontal line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    //... and one vertical line
    display.clearDisplay();
    display.drawFastVLine(
        100, 100, 600,
        0); // Arguments are: starting X, starting Y, length, color
    displayCurrentAction(display, "Drawing one vertical line");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Now, let' make a grid using only horizontal and vertical lines in random
    // colors!
    display.clearDisplay();
    for (int i = 0; i < 800; i += 8) {
      display.drawFastVLine(i, 0, 600, (i / 8) & 0x0F);
    }
    for (int i = 0; i < 600; i += 4) {
      display.drawFastHLine(0, i, 800, (i / 8) & 0x0F);
    }
    displayCurrentAction(display, "Drawing a grid using horizontal and "
                                  "vertical lines in different colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rectangle at X = 133, Y = 145 and size of 400x300 pixels
    display.clearDisplay();
    display.drawRect(
        133, 145, 400, 300,
        0); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction(display, "Drawing rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rectangles on random location, size 100x150 pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawRect(random(0, 799), random(0, 599), 100, 150, random(0, 7));
    }
    displayCurrentAction(display, "Drawing many rectangles in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled gray rectangle at X = 133, Y = 145, size of 467x364 pixels
    display.clearDisplay();
    display.fillRect(
        133, 145, 467, 364,
        4); // Arguments are: start X, start Y, size X, size Y, color
    displayCurrentAction(display, "Drawing gray rectangle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled random colored rectangles on random location, size of 30x30
    // pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.fillRect(random(0, 799), random(0, 599), 30, 30, random(0, 7));
    }
    displayCurrentAction(
        display, "Drawing many filled rectangles randomly in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw circle at center of a screen with radius of 50 pixels
    display.clearDisplay();
    display.drawCircle(400, 300, 50,
                       0); // Arguments are: start X, start Y, radius, color
    displayCurrentAction(display, "Drawing a circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw some random colored circles at random location with radius of 25
    // pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 40; i++) {
      display.drawCircle(random(0, 799), random(0, 599), 25, random(0, 7));
    }
    displayCurrentAction(display,
                         "Drawing many circles randomly in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw black filled circle at center of a screen with radius of 50 pixels
    display.clearDisplay();
    display.fillCircle(400, 300, 50,
                       0); // Arguments are: start X, start Y, radius, color
    displayCurrentAction(display, "Drawing black-filled circle");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw some random colored filled circles at random location with radius of
    // 15 pixels
    display.clearDisplay();
    for (int i = 0; i < 40; i++) {
      display.fillCircle(random(0, 799), random(0, 599), 15, random(0, 7));
    }
    displayCurrentAction(
        display, "Drawing many filled circles randomly in random colors");
    display.display(); // To show stuff on screen, you always need to call
                       // display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rounded rectangle at X = 133, Y = 145 and size of 467x364 pixels and
    // radius of 10 pixels
    display.clearDisplay();
    display.drawRoundRect(
        133, 145, 467, 364, 10,
        0); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction(display, "Drawing rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw rounded rectangles on random location, size 100x150 pixels, radius
    // of 5 pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.drawRoundRect(random(0, 799), random(0, 599), 100, 150, 5,
                            random(0, 7));
    }
    displayCurrentAction(display, "Drawing many rounded edges rectangles");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled rounded rectangle at X = 133, Y = 145, size of 467x364
    // pixels and radius of 10 pixels
    display.clearDisplay();
    display.fillRoundRect(
        133, 145, 467, 364, 10,
        0); // Arguments are: start X, start Y, size X, size Y, radius, color
    displayCurrentAction(display,
                         "Drawing filled rectangle with rounded edges");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled random colored rectangle on random location, size of 30x30
    // pixels, radius of 3 pixels in random color
    display.clearDisplay();
    for (int i = 0; i < 50; i++) {
      display.fillRoundRect(random(0, 799), random(0, 599), 30, 30, 3,
                            random(0, 7));
    }
    displayCurrentAction(
        display,
        "Drawing many filled rectangle with rounded edges in random colors");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw simple triangle
    display.clearDisplay();
    display.drawTriangle(167, 436, 367, 436, 400, 73,
                         0); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draw filled triangle inside simple triangle (so no display.clearDisplay()
    // this time)
    display.fillTriangle(275, 255, 333, 255, 400, 109,
                         0); // Arguments are: X1, Y1, X2, Y2, X3, Y3, color
    displayCurrentAction(display,
                         "Drawing filled triangle inside exsisting one");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Display some grayscale image on screen. We are going to display Soldered
    // logo on display at location X = 67, Y = 73 Image size is 333x241 pixels.
    display.clearDisplay();
    display.image.draw(picture1, 67, 73, 333,
                       241, BLACK); // Arguments are: array variable name, size X, size
                             // Y, start X, start Y
    displayCurrentAction(display, "Drawing a bitmap image");
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write some text on screen with different sizes and color
    display.clearDisplay();
    for (int i = 0; i < 6; i++) {
      display.setTextColor(i);
      display.setTextSize(i + 1); // textSize parameter starts at 0 and goes up
                                  // to 10 (larger won't fit Inkplate 6 screen)
      display.setCursor(
          133, (i * i *
                8)); // setCursor works as same as on LCD displays - sets "the
                     // cursor" at the place you want to write someting next
      display.print("INKPLATE6!"); // The actual text you want to show on
                                   // e-paper as String
    }
    displayCurrentAction(display, "Text in different sizes and shadings");
    display.display(); // To show stuff on screen, you always need to call
                       // display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write same text on different location, but now invert colors (text is
    // white, text background is black)
    display.setTextColor(7, 0); // First argument is text color, while second
                                // argument is background color. In greyscale,
    for (int i = 0; i < 6;
         i++) { // you are able to choose from 8 different colors (0-7)
      display.setTextSize(i + 1);
      display.setCursor(133, 300 + (i * i * 8));
      display.print("INKPLATE6!");
    }
    display.display();
    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Draws an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.drawElipse(67, 300, 400, 300, 0);
    displayCurrentAction(display, "Drawing an elipse");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Fills an elipse with x radius, y radius, center x, center y and color
    display.clearDisplay();
    display.fillElipse(67, 300, 400, 300, 0);
    displayCurrentAction(display, "Drawing a filled elipse");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Code block for generating random points and sorting them in a counter
    // clockwise direction.
    int xt[10];
    int yt[10];
    int n = 10;
    for (int i = 0; i < n; ++i) {
      xt[i] = random(67, 733);
      yt[i] = random(67, 527);
    }
    int k;
    for (int i = 0; i < n - 1; ++i)
      for (int j = i + 1; j < n; ++j)
        if (atan2((double)(yt[j] - 300), (double)(xt[j] - 400)) < atan2((double)(yt[i] - 300), (double)(xt[i] - 400))) {
          k = xt[i], xt[i] = xt[j], xt[j] = k;
          k = yt[i], yt[i] = yt[j], yt[j] = k;
        }

    // Draws a polygon, from x and y coordinate arrays of n points in color c
    display.clearDisplay();
    display.drawPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a polygon");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Fills a polygon, from x and y coordinate arrays of n points in color c,
    // Points need to be counter clockwise sorted
    // Method can be quite slow for now, probably will improve
    display.clearDisplay();
    display.fillPolygon(xt, yt, n, 0);
    displayCurrentAction(display, "Drawing a filled polygon");
    display.display();

    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

    // Write text and rotate it by 90 deg. forever
    int r = 0;
    display.setTextSize(8);
    display.setTextColor(7, 0);
    while (true) {
      display.setCursor(100, 100);
      display.clearDisplay();
      display.setRotation(
          r); // Set rotation will sent rotation for the entire display, so you
              // can use it sideways or upside-down
      display.print("INKPLATE6");
      display.display();
      r++;
      vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }

    // Did you know that you can change between BW and greyscale mode anytime?
    // Just call display.selectDisplayMode(mode)
  }
}
