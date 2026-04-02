#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>

#include "Inkplate.h"
#include "Network.h"

static const char *TAG = "MAIN";

// 800x600 24bpp BMP — replace with any BMP URL accessible on your network
#define BMP_URL "https://raw.githubusercontent.com/SolderedElectronics/Inkplate-Arduino-library/refs/heads/dev/examples/Inkplate6/Advanced/microSD/Inkplate6_microSD_Pictures/image2.bmp"
extern "C"
void app_main(void)
{
    Inkplate display;
    WiFi     wifi;

    if (!wifi.waitForConnect())
    {
        ESP_LOGE(TAG, "WiFi connection timed out");
        return;
    }

    int32_t  len  = 800 * 600 * 3 + 54; // worst case 24bpp + BMP header
    uint8_t *data = wifi.downloadFile(BMP_URL, &len);

    if (!data)
    {
        ESP_LOGE(TAG, "Download failed");
        return;
    }

    ESP_LOGI(TAG, "Downloaded %ld bytes, drawing...", len);

    display.setDisplayMode(GRAYSCALE);
    display.clearDisplay();

    display.image.draw(data, 0, 0);

    free(data);

    display.display();
}
