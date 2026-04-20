#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "coast.jpg"

#define DRAW_LINE
#ifdef DRAW_LINE
uint16_t xOld, yOld;
#endif

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    while(1)
    {
        // Check if there is any touch detected
    if (display.touchscreen.available())
    {
        uint8_t n;
        uint16_t x[2], y[2];
        // See how many fingers are detected (max 2) and copy x and y position of each finger on touchscreen
        n = display.touchscreen.getData(x, y);
        if (n != 0)
        {
#ifdef DRAW_LINE // Draw line from old point to new
            display.drawLine(xOld, yOld, x[0], y[0], 1);

            // Save coordinates to use as old next time
            xOld = x[0];
            yOld = y[0];
#endif

#ifdef DRAW_CIRCLE // Draw circle on touch event coordinates
            display.fillCircle(x[0], y[0], 20, BLACK);
#endif
            display.partialUpdate(false, true);
        }
    }
    }

    return;
    //isplay.wifi.begin();
    //display.wifi.waitForConnect();

    ESP_ERROR_CHECK(display.sdCardInit());

    //display.image.setDitherKernel(FloydSteinberg);
    display.image.draw(IMAGE_PATH, 0, 0, false, true);
    display.display();
}