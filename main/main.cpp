#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define SD_IMAGE_PATH "https://upload.wikimedia.org/wikipedia/commons/c/c2/Auckland_Skyline_800x600.jpg"

extern "C"
void app_main(void)
{
    Inkplate display;

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();

    if (display.sdCardInit() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card init failed");
        return;
    }

    if (!display.image.draw(SD_IMAGE_PATH, 0,0, false, true))
        ESP_LOGE(TAG, "Image draw failed");

    display.display();
    vTaskDelay(1000);

    if (!display.image.draw(SD_IMAGE_PATH, 0,0, false))
        ESP_LOGE(TAG, "Image draw failed");

    display.sdCardSleep();
    display.display();
}
