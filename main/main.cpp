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

    display.frontlight.setState(true);
    vTaskDelay(500);
    display.frontlight.setState(false);
    //display.display();
    return;
    display.wifi.begin();
    display.wifi.waitForConnect();

    //if (display.sdCardInit() != ESP_OK)
    {
      //  ESP_LOGE(TAG, "SD card init failed");
    //return;
    }

    display.image.draw(IMAGE_PATH, 0, 0, false, true);
    display.display();
}