#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://onlinejpgtools.com/images/examples-onlinejpgtools/sunflower.jpg"

extern "C"
void app_main(void)
{
    Inkplate display;
    if (display.touch.ping())
    {
        ESP_LOGI(TAG, "asd");
    }
    while(1)
    {
    if (display.touch.available())
    {
        ESP_LOGI(TAG, "avail");
        uint8_t n;
        uint16_t x[2], y[2];

        // See how many fingers are detected (max 2) and copy x and y position of each finger on touchscreen
        n = display.touch.getData(x, y);
        if (n != 0)
        {
            // Print number of fingers to serial monitor, along with their coordinates
            ESP_LOGI(TAG, "%d finger ", n);
            for (int i = 0; i < n; i++)
                ESP_LOGI(TAG, "X=%d Y=%d ", x[i], y[i]);
        }
        else
        {
            // If touchscreen driver returns us a zero, it means that there are no more touch events pressent on the
            // screen
            x[0] = 0;
            x[1] = 0;
            y[0] = 0;
            y[1] = 0;
            ESP_LOGI(TAG, "Release");
        }
        
    } else
    {
        ESP_LOGI(TAG, "notavail");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    }
    ESP_LOGI(TAG, "return");
    return;

    display.wifi.begin();
    display.wifi.waitForConnect();

    //ESP_ERROR_CHECK(display.sdCardInit());

    //display.image.setDitherKernel(FloydSteinberg);
    display.image.draw(IMAGE_PATH, 0, 0, false, false);
    display.display();
}