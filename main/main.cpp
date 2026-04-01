#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

extern "C"
void app_main(void)
{
    const char text[] = "This is partial update";
    int offset = 800;
    int partialUpdates=20;
    Inkplate inkplate;
    inkplate.setTextSize(4);             // Set text to be 4 times bigger than classic 5x7 px text
    inkplate.setDisplayMode(BLACK_AND_WHITE);
    inkplate.setTextWrap(false);
    inkplate.setFullUpdateThreshold(partialUpdates); 
    //inkplate.setTextColor(0, 1);
    inkplate.clearDisplay();         // Clear content in frame buffer
    inkplate.display();

    while (1) {
    inkplate.clearDisplay();         // Clear content in frame buffer
    inkplate.setCursor(offset, 100); // Set new position for text
    inkplate.print(text);            // Write text at new position
    inkplate.partialUpdate(false, true); 
    offset -= 20; // Move text into new position
    if (offset < 0)
        offset = 800; // Text is scrolled till the end of the screen? Get it back on the start!
    
    vTaskDelay(pdMS_TO_TICKS(500));       // Delay between refreshes.
    }
}
