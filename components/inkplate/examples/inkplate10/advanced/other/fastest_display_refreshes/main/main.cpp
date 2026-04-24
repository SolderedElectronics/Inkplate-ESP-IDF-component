#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "Inkplate.h"

static const char *TAG = "MAIN";

extern "C"
void app_main(void)
{
    Inkplate display;

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();
    display.display(); // Full refresh once at start

    display.setTextColor(BLACK, WHITE); // Set text color to be black and background color to be white
    display.setTextWrap(false);         // Disable text wraping
    display.setTextSize(4);

    const char *text = "Inkplate partial update scrolling demo";
    int textY = 300;
    int textSpeed = 8;


    while (true)
    {
        int16_t x1, y1;
        uint16_t w, h;

        // Measure text bounds
        display.getTextBounds(text, 0, textY, &x1, &y1, &w, &h);

        int x = -(int)w; // Start off-screen left

        // Keep panel powered for faster partial updates
        display.einkOn();

        while (x < display.width())
        {
            // Clear previous text area
            display.fillRect(0, textY - h, display.width(), h + 10, WHITE);

            // Draw text at new x position
            display.setCursor(x, textY);
            display.print(text);

            // Partial update, panel stays on
            display.partialUpdate(0, 1);

            x += textSpeed;
            vTaskDelay(pdMS_TO_TICKS(80));
        }

        // Power off panel to save energy
        display.einkOff();

        ESP_LOGI(TAG, "Scroll complete, pausing...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}