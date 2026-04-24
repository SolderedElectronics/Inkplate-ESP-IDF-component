#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#include "Inkplate.h"

static const char *TAG = "MAIN";

extern "C"
void app_main(void)
{
    Inkplate display;

    if (display.sdCardInit() != ESP_OK)
    {
        ESP_LOGE(TAG, "SD card init failed");
        return;
    }

    // write to file
    FILE *f = fopen("/sdcard/message.txt", "w");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello from Inkplate!\n");
    fclose(f);
    ESP_LOGI(TAG, "Write successful");

    display.sdCardSleep();

}