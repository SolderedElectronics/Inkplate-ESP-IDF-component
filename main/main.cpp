#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "coast.jpg"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setRotation(2);

    //display.wifi.begin();
    //display.wifi.waitForConnect();

    ESP_ERROR_CHECK(display.sdCardInit());

    display.image.setDitherKernel(FloydSteinberg);
    display.image.draw(IMAGE_PATH, 0, 0, false, true);
    display.display();
}