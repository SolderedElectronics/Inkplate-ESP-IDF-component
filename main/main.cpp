#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://thumb.photo-ac.com/96/968b9b6fb0db2b364491b87fb1e792ee_t.jpeg"
extern "C"
void app_main(void)
{
    Inkplate display;

    display.frontlight.setState(true);
    display.frontlight.setBrightness(62);

    display.wifi.begin();
    display.wifi.waitForConnect();

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();     // Clear the frame buffer (does NOT clear the physical screen)

    display.display();

    if (!display.image.draw(IMAGE_PATH, 0, 0, false, true))
        ESP_LOGE(TAG, "Image draw failed");

    display.display();
}