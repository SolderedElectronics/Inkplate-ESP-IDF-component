#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://wallpaperaccess.com/full/1140575.png"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setRotation(3);
    display.wifi.begin();
    display.wifi.waitForConnect();

    //if (display.sdCardInit() != ESP_OK)
    {
      //  ESP_LOGE(TAG, "SD card init failed");
    //return;
    }

    display.image.setDitherKernel(Stucki);
    display.image.draw(IMAGE_PATH, 0, 0, false, true);
    display.display();
}