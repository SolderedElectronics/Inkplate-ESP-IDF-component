#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

static void displayTestImage(Inkplate &display)
{
    display.clearDisplay();

    double vcom = display.getVCOM();
    ESP_LOGI(TAG, "displayTestImage getVCOM() = %.2f", vcom);

    char buf[32];
    snprintf(buf, sizeof(buf), "Stored VCOM: %.2f V", vcom);

    display.setTextSize(2);
    display.setCursor(5, 5);
    display.print(buf);

    for (int i = 0; i < 8; i++)
    {
        int x = (display.width() / 8) * i;
        display.fillRect(x, 40, display.width() / 8, display.height() - 40, i);
    }

    display.display(true);
}

extern "C"
void app_main(void)
{
    Inkplate display;

    display.setDisplayMode(GRAYSCALE);

    ESP_LOGI(TAG, "getVCOM before setVCOM: %.2f", display.getVCOM());

    esp_err_t ret = display.setVCOM(-2.4);
    ESP_LOGI(TAG, "setVCOM returned: %s", esp_err_to_name(ret));

    ESP_LOGI(TAG, "getVCOM after setVCOM: %.2f", display.getVCOM());

    displayTestImage(display);
}
