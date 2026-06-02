/**
 **************************************************
 * @file        Inkplate4TEMPERA_SD_TXT_Read.ino
 * @brief       Read and display text file from SD card on Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to open a .txt file from a FAT-formatted SD card
 *              and display its contents on the Inkplate 4TEMPERA e-paper display.
 *              The example reads a file named "text.txt" from the SD card and
 *              prints its content on screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable, microSD card
 * - Extra:      text.txt file on SD card (≤ 200 characters)
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 * - Additional library required:
 *   https://github.com/e-radionicacom/Inkplate-6-SDFat-Arduino-Library
 * - SD card format: FAT / FAT32
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Copy a text file named "text.txt" (max 200 characters) to a FAT-formatted SD card.
 * 2) Insert the SD card into the Inkplate.
 * 3) Install the required SDFat library.
 * 4) Upload the sketch to Inkplate 4TEMPERA.
 * 5) The file contents will be read and displayed on the e-paper screen.
 *
 * Expected output:
 * - The contents of text.txt are displayed on the Inkplate display.
 *
 * Notes:
 * - File name must be exactly "text.txt" for this example.
 * - Text length should not exceed 200 characters.
 * - Ensure the required SDFat library is properly installed.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2021-02-11
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "Inkplate.h"            //Include Inkplate library to the sketch
               //Include library for SD card
Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)
SdFile file;                     // Create SdFile object used for accessing files on SD card

void setup()
{
    display.begin();        // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear frame buffer of display
    display.display();      // Put clear image on display
    display.frontlight.setState(15);
    // Init SD card. Display if SD card is init propery or not.
    if (display.sdCardInit())
    {
        display.setTextSize(3);    // Set text to be 3 times bigger than classic 5x7 px text
        display.setCursor(30, 30); // Set position of text
        display.println("SD Card OK! Reading data...");
        display.partialUpdate();
        delay(3000); // Wait a bit so the user reads the message

        // Try to load text with max lenght of 200 chars.
        if (!file.open("/text.txt", O_RDONLY))
        { // If it fails to open, send error message to display, otherwise read the file.
            display.setCursor(30, 70);
            display.println("File open error!");
            display.display();
        }
        else
        {
            display.clearDisplay();    // Clear everything that is stored in frame buffer of epaper
            display.setCursor(0, 0);   // Set print position at the begining of the screen
            display.setTextSize(1);    // reset text size
            char text[3001];           // Array where data from SD card is stored (max 200 chars here)
            int len = file.fileSize(); // Read how big is file that we are opening
            if (len > 3000)
                len = 3000;       // If it's more than 200 bytes (200 chars), limit to max 200 bytes
            file.read(text, len); // Read data from file and save it in text array
            text[len] = 0;        // Put null terminating char at the and of data
            display.setTextSize(2);
            display.print(text);   // Print data/text
            display.sdCardSleep(); // Put sd card in sleep mode
            display.display();     // Do a full refresh of display
        }
    }
    else
    { // If card init was not successful, display error on screen, put sd card in sleep mode, and stop the program
        // (using infinite loop)
        display.setCursor(30, 30);
        display.setTextSize(3); // Set text to be 3 times bigger than classic 5x7 px text
        display.println("SD Card error!");
        display.partialUpdate();
        display.sdCardSleep();
        while (true)
            ;
    }
}

void loop()
{
    // Nothing...
}
