#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Inkplate.h"

static const char *TAG = "MAIN";

extern "C" void app_main(void)
{
    Inkplate display;
//return;
    ESP_LOGI(TAG, "%f", display.bme.readTemperature());
}