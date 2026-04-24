#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Inkplate.h"

static const char *TAG = "MAIN";

extern "C"
void app_main(void)
{
    Inkplate display;

    display.clearDisplay();

    // Init SD card
    if (display.sdCardInit() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card init failed");
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.print("SD card error!");
        display.display();
        return;
    }

    // Open the PNG file
    FILE *f = fopen("/sdcard/image.png", "rb");
    if (!f)
    {
        ESP_LOGE(TAG, "Cannot open image.png");
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.print("Cannot open image.png");
        display.display();
        display.sdCardSleep();
        return;
    }

    // Get file size
    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fileSize <= 0)
    {
        ESP_LOGE(TAG, "Invalid file size: %ld", fileSize);
        fclose(f);
        display.sdCardSleep();
        return;
    }

    ESP_LOGI(TAG, "PNG file size: %ld bytes", fileSize);

    // Allocate buffer
    uint8_t *buf = (uint8_t *)malloc((size_t)fileSize);
    if (!buf)
    {
        ESP_LOGE(TAG, "Not enough RAM for image buffer! Free heap: %lu",
                 (unsigned long)esp_get_free_heap_size());
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.print("Not enough RAM!");
        display.display();
        fclose(f);
        display.sdCardSleep();
        return;
    }

    // Read entire file into buffer
    size_t bytesRead = fread(buf, 1, (size_t)fileSize, f);
    fclose(f);
    display.sdCardSleep();

    if (bytesRead != (size_t)fileSize)
    {
        ESP_LOGE(TAG, "Read mismatch: expected %ld, got %zu", fileSize, bytesRead);
        free(buf);
        return;
    }

    ESP_LOGI(TAG, "File read OK, decoding PNG...");

    // Draw PNG from buffer
    if (!display.image.draw(buf, (uint32_t)fileSize, 0, 0, true, false))
    {
        ESP_LOGE(TAG, "PNG decode error");
        display.setTextSize(2);
        display.setCursor(10, 10);
        display.print("PNG decode error");
    }

    free(buf);
    display.display();

    ESP_LOGI(TAG, "Done");
}