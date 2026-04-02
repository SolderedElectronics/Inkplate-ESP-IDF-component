#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

#define IMAGE_URL "https://samplelib.com/png/sample-bumblebee-400x300.png"

extern "C"
void app_main(void)
{
    Inkplate display;

    if (!display.wifi.waitForConnect())
    {
        ESP_LOGE(TAG, "WiFi connection timed out");
        return;
    }

    display.setDisplayMode(GRAYSCALE);
    display.clearDisplay();

    if (!display.image.draw(IMAGE_URL, 100, 0))
    {
        ESP_LOGE(TAG, "Failed to draw image");
        return;
    }

    display.display();
}
