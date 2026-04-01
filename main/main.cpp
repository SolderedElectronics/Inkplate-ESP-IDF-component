#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Char array where you can store your text that will be scrolled.
const char text[] = "This is partial update on Inkplate 6 e-paper display! :)";

// This variable is used for moving the text (scrolling)
int offset = 800;

//This variable is used to define the number of partial updates before doing a full update
int partialUpdates=15;

extern "C"
void app_main(void)
{
    Inkplate display;

    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display
    display.setDisplayMode(BLACK_AND_WHITE);
    display.setTextSize(4);             // Set text to be 4 times bigger than classic 5x7 px text
    display.setTextWrap(false);         // Disable text wraping
    /*
    Set the number of partial updates before doing a full update
    This function forces a full update as the next update to ensure that the cycle of partial 
    updates starts from a fully updated screen.
    The Inkplate class keeps a internal counter that increments every time partialUpdate() gets called.
    */
    display.setFullUpdateThreshold(partialUpdates); 
    // BASIC USAGE

    while (1) {
    display.clearDisplay();         // Clear content in frame buffer
    display.setCursor(offset, 300); // Set new position for text
    display.print(text);            // Write text at new position
    /*
    //Updates changes parts of the screen without the need to refresh the whole display
    //partialUpdate(bool _forced, bool leaveOn)
	    _forced		Can force partial update in deep sleep (for advanced use)
	    leaveOn 	If set to 1, it will disable turning power supply for eink after display update in order to increase refresh time
    */
    display.partialUpdate(false, true); 
    offset -= 20; // Move text into new position
    if (offset < 0)
        offset = 800; // Text is scrolled till the end of the screen? Get it back on the start!
    vTaskDelay(5);       // Delay between refreshes.
    }
}
