#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

static Inkplate inkplate;

extern "C"
void app_main(void)
{
    inkplate.begin();

    // --- Basic drawing demo ---

    inkplate.clearDisplay();

    // Text
    inkplate.setDisplayMode(GRAYSCALE);
    inkplate.setTextSize(3);
    inkplate.setCursor(20, 10);
    inkplate.print("inkplate");

   
 inkplate.fillRect(200, 200, 400, 300, 4); // Arguments are: start X, start Y, size X, size Y, color
    

    // Push framebuffer to the e-ink display
    inkplate.display();
}
