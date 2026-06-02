/**
 **************************************************
 * @file        Inkplate4TEMPERA_Faster_Display_Refreshes.ino
 * @brief       Faster partial refresh demo by keeping the e-paper panel powered on (Inkplate 4TEMPERA).
 *
 * @details     Demonstrates how to speed up consecutive partial updates by
 *              keeping the e-paper panel power enabled during repeated refreshes.
 *              The example scrolls text across the screen using partial updates
 *              while the panel is powered on via einkOn().
 *
 *              Normally, the panel power is automatically enabled before each
 *              refresh and disabled afterward to save energy. Calling einkOn()
 *              keeps the high-voltage e-paper power rails enabled, allowing
 *              multiple partialUpdate() calls to run faster without repeated
 *              power cycling. einkOff() disables the panel power again and should
 *              always be called before long idle periods or deep sleep.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - Serial settings: 115200 baud (optional)
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Upload the sketch to Inkplate 4TEMPERA.
 * 2) A full refresh is performed once at startup.
 * 3) Text scrolls across the display using fast partial updates.
 * 4) The panel is powered off after the animation completes.
 *
 * Expected output:
 * - Smooth scrolling text using faster partial refreshes.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - Keeping the panel powered on increases power consumption.
 * - Always call einkOff() before entering deep sleep.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2021-09-22
 * @license     GNU GPL V3
 **************************************************/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "Inkplate.h"

// 1-bit mode supports partial update
Inkplate display(INKPLATE_1BIT);

const char *text = "Inkplate partial update scrolling demo";
int textY = 300;          // Vertical position of text
int textSpeed = 8;        // Pixels per frame

void setup()
{
    Serial.begin(115200);
    display.begin();

    display.clearDisplay();
    display.display();    // Full refresh once at start
}

void loop()
{
    int16_t x1, y1;
    uint16_t w, h;

    // Measure text width
    display.setTextSize(4);
    display.getTextBounds(text, 0, textY, &x1, &y1, &w, &h);

    // Start off-screen on the left
    int x = -w;

    // Turn E-Ink power ON once for faster partial updates
    display.einkOn();

    while (x < display.width())
    {
        // Clear only the previous text area
        display.fillRect(0, textY - h, display.width(), h + 10, WHITE);

        // Draw text at new position
        display.setCursor(x, textY);
        display.setTextColor(BLACK);
        display.print(text);

        // Partial update, keep panel powered
        display.partialUpdate(0, 1);

        x += textSpeed;
        delay(80); // Controls scroll speed
    }

    // Turn E-Ink power OFF to save energy
    display.einkOff();

    delay(5000); // Pause before repeating
}