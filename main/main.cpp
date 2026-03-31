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
    inkplate.setTextSize(5);
    inkplate.setTextColor(0); // black
    inkplate.setCursor(200, 10);
    inkplate.print("hello");

    inkplate.fillElipse(200, 200, 400, 600, 4); // Arguments are: start X, start Y, size X, size Y, color


    // Push framebuffer to the e-ink display
    inkplate.display3b();
}
