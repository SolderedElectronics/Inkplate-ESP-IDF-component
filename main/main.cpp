#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define SD_IMAGE_PATH "image.png"

extern "C"
void app_main(void)
{
    Inkplate display;

    if (display.sdCardInit() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card init failed");
        return;
    }

    display.setDisplayMode(GRAYSCALE);
    display.clearDisplay();

    if (!display.image.draw(SD_IMAGE_PATH, 100, 300))
        ESP_LOGE(TAG, "Image draw failed");

    display.sdCardSleep();
    display.display();
}
