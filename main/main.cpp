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
    inkplate.setTextSize(5);
    inkplate.setCursor(200, 10);
    inkplate.print("hello");



    // Push framebuffer to the e-ink display
    inkplate.display();
}
