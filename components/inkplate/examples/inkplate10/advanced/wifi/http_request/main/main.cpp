#include "Inkplate.h"
#include "WiFi.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "HTTP_REQUEST";

extern "C" void app_main(void)
{
    static Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.setTextColor(BLACK, WHITE);
    display.setTextWrap(true);

    // Show initial status
    display.print("Connecting to WiFi...");
    display.display();

    // Connect using existing WiFi class (SSID/pass set via menuconfig)
    WiFi wifi;
    if (wifi.begin() != ESP_OK || !wifi.waitForConnect(10000))
    {
        display.print("WiFi connection failed!");
        display.display();
        ESP_LOGE(TAG, "WiFi connection failed");
        return;
    }

    display.print("Connected!");
    display.display();

    // Fetch the page
    int32_t len = 32768; // max buffer size — adjust if needed
    uint8_t *data = wifi.downloadFile("http://example.com/index.html", &len);

    if (!data || len <= 0)
    {
        display.print("HTTP request failed!");
        display.display();
        ESP_LOGE(TAG, "HTTP request failed");
        return;
    }

    // Null-terminate so we can print as string
    data[len] = '\0';
    ESP_LOGI(TAG, "Received %ld bytes", len);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print((char *)data);
    display.display();

    free(data);
}