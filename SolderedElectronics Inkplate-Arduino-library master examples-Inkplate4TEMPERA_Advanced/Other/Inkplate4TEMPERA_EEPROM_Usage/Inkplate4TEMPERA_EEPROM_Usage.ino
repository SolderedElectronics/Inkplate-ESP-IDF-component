/**
 **************************************************
 * @file        Inkplate4TEMPERA_EEPROM_Usage.ino
 * @brief       EEPROM read/write example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to use the built-in EEPROM on Inkplate 4TEMPERA to
 *              store data that persists across resets and power cycles.
 *              The example shows how to safely clear, write, and read user data
 *              from EEPROM, while respecting reserved address ranges used by
 *              the e-paper waveform data.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate4TEMPERA
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/4TEMPERA/quick-start-guide/
 *
 * How to use:
 * 1) Upload the sketch to Inkplate 4TEMPERA.
 * 2) The display will first clear user EEPROM data.
 * 3) Sample data is written to EEPROM.
 * 4) Stored data is read back and shown on the display.
 *
 * Expected output:
 * - Messages indicating EEPROM clearing, writing, and reading.
 * - A list of values read from EEPROM displayed on the screen.
 *
 * Notes:
 * - EEPROM addresses 0–75 are reserved for waveform data.
 * - DO NOT read from or write to addresses below 76.
 * - Changing EEPROM size may erase waveform data and affect display operation.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2022-03-13
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Soldered Inkplate4TEMPERA in the boards menu."
#endif

#include "EEPROM.h"   // Include ESP32 EEPROM library
#include "Inkplate.h" // Include Inkplate library to the sketch

#define EEPROM_SIZE 128 // How much data to write to EEPROM in this example

// Create an object on Inkplate library and also set library into 1-bit mode (BW)
Inkplate display(INKPLATE_1BIT);

void setup()
{
    // Init library (you should call this function ONLY ONCE)
    display.begin();

    // Init serial communication
    Serial.begin(115200);

    // Init EEPROM library with 128 of EEPROM size.
    EEPROM.begin(EEPROM_SIZE);

    display.setTextSize(3);                  // Set text size
    display.setCursor(10, 10);
    display.println("Clearing EEPROM...\n"); // Print message
    Serial.println("Clearing EEPROM...\n");  // Print message
    clearEEPROM();                           // Clear user EEPROM data
    delay(500);                              // Wait a little bit...

    display.setCursor(10, 50);
    display.println("Writing data to EEPROM...\n"); // Print message
    Serial.println("Writing data to EEPROM...\n");  // Print message
    writeEEPROM();                                  // Write some data to EEPROM
    delay(500);                                     // Wait a little bit...

    display.setCursor(10, 90);
    display.println("Reading data from EEPROM:\n"); // Print message
    Serial.println("Reading data from EEPROM:\n");  // Print message
    display.setTextSize(2);                         // Use smaller text so everything can fit on display
    printEEPROM();                                  // Read data from EEPROM and display it on screen
    display.display();
}

void loop()
{
    // Empty...
}

// Function for clearing EEPROM data
void clearEEPROM()
{
    // Go through each address and clear it (write 0)
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

// Function writes data to EEPROM
void writeEEPROM()
{
    // Go through each address and write current index
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, i);
    }
    EEPROM.commit();
}

// Function reads back previously written data and displays it on screen
void printEEPROM()
{
    // Go through each address and read a value from it
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        // Print read value in decimal
        display.print(EEPROM.read(i), DEC);
        Serial.print(EEPROM.read(i));

        // Print a comma after each number except the last one
        if (i != EEPROM_SIZE - 1)
        {
            display.print(", ");
            Serial.print(", ");
        }
    }
}
