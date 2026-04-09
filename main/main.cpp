#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://cimvhrforum.ca/wp-content/uploads/2026/01/forum-2026-present-en-600x600.png"
extern "C"
void app_main(void)
{
    Inkplate display;

    display.wifi.begin();
    display.wifi.waitForConnect();

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();     // Clear the frame buffer (does NOT clear the physical screen)

    if (!display.image.draw(IMAGE_PATH, 0, 0, false, true))
        ESP_LOGE(TAG, "Image draw failed");

    display.display();
}