/**
 **************************************************
 * @file        Inkplate4TEMPERA_SD_Pictures.ino
 * @brief       Display images from SD card on Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to load image files from an SD card and display
 *              them on the Inkplate 4TEMPERA e-paper display. The example shows how to
 *              read supported image formats from a FAT-formatted SD card and
 *              render them using the Inkplate graphics library.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable, microSD card
 * - Extra:      SD card with compatible image files
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - SD card format: FAT / FAT32
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Copy supported image files to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Upload the sketch to Inkplate 4TEMPERA.
 * 4) The image is read from the SD card and displayed on the e-paper screen.
 *
 * Expected output:
 * - Selected image from the SD card is shown on the Inkplate display.
 *
 * Notes:
 * - Supported formats include BMP, JPEG, and PNG (with library limitations).
 * - Supported color depths: 1-bit (BW), 4-bit, 8-bit, and 24-bit.
 * - Maximum supported resolution is 600 × 600 pixels.
 * - Images larger than the display resolution will not fit on screen.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2023-02-17
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "Inkplate.h"            // Include Inkplate library to the sketch
Inkplate display(INKPLATE_3BIT); // Create an object on Inkplate library and also set library into 3 Bit mode
SdFile file;                     // Create SdFile object used for accessing files on SD card

void setup()
{
    display.begin();             // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay();      // Clear frame buffer of display
    display.setTextColor(BLACK); // Set text color to black
    display.setTextSize(3);      // Set font size to 3
    display.setCursor(30, 30); // Set the text a bit further from the corner so it's visible

    // Init SD card. Display if SD card is init propery or not.
    if (display.sdCardInit())
    {
        display.println("SD Card OK! Reading image...");
        display.display();

        // If card is properly init, try to load image and display it on e-paper at position X=0, Y=0
        // NOTE: Both drawImage methods allow for an optional fifth "invert" parameter. Setting this parameter
        // to true will flip all colors on the image, making black white and white black. This may be necessary when
        // exporting bitmaps from certain softwares.
        if (!display.image.draw("image1.bmp", 0, 0, 1))
        {
            // If is something failed (wrong filename or wrong bitmap format), write error message on the screen.
            // REMEMBER! You can only use Windows Bitmap file with color depth of 1, 4, 8 or 24 bits with no
            // compression! You can turn of dithering for somewhat faster image load by changing the last 1 to 0, or
            // removing the parameter completely
            display.setCursor(30, 100); // Set the text a bit further
            display.println("Image open error");
        }
        display.display();
        delay(5000);

        // Now try to load image using SdFat library class (for more advanced users) and display image on epaper.
        display.clearDisplay();
        if (file.open("image2.bmp", O_RDONLY))
        {
            display.image.drawBitmapFromSd(&file, 0, 0, 1);
        }
        else
        {
            display.println("Image open error");
        }
        display.display();
        delay(5000);

        // Now draw a JPEG
        display.clearDisplay();
        if (!display.image.draw("pyramid.jpg", 0, 0, true))
        {
            // If is something failed (wrong filename or wrong format), write error message on the screen.
            // You can turn off dithering for somewhat faster image load by changing the fourth parameter to false, or
            // removing the parameter completely
            display.setCursor(30, 100); // Set the text a bit further
            display.println("Image open error");
        }
        display.display();
    }
    else
    {
        // If SD card init not success, display error on screen
        display.println("SD Card error!");
        display.display();
    }

    // Turn off the MOSFET that powers the SD card
    display.sdCardSleep();
}

void loop()
{
    // Nothing...
}
