#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

// link to image
#define IMAGE_PATH "https://upload.wikimedia.org/wikipedia/commons/c/c2/Auckland_Skyline_800x600.jpg"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE);

    display.wifi.begin();
    display.wifi.waitForConnect();

    display.image.draw(IMAGE_PATH, 0, 0, true, false);
    display.display();
}