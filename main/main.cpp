#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://upload.wikimedia.org/wikipedia/commons/c/c2/Auckland_Skyline_800x600.jpg"
extern "C"
void app_main(void)
{
    Inkplate display;

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();     // Clear the frame buffer (does NOT clear the physical screen)

    display.image.draw(IMAGE_PATH, 0,0, false, true);

    display.setTextSize(5);
    display.setCursor(900, 300);
    display.print(32);

    display.display();
}