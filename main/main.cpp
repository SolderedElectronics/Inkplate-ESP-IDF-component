#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "Inkplate.h"

extern "C"
void app_main(void)
{
    Inkplate display;

    // Set time once: 2026-04-02 12:00:00
    tm t      = {};
    t.tm_year = 2026;
    t.tm_mon  = 4;
    t.tm_mday = 2;
    t.tm_hour = 12;
    t.tm_min  = 0;
    t.tm_sec  = 0;
    display.rtc.setTime(t);

    display.setDisplayMode(BLACK_AND_WHITE);
    display.setTextSize(5);
    display.setFullUpdateThreshold(20);
    display.clearDisplay();
    display.display();

    while (1)
    {
        tm now;
        display.rtc.getTime(&now);

        char buf[16];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", now.tm_hour, now.tm_min, now.tm_sec);

        display.clearDisplay();
        display.setCursor(260, 270);
        display.print(buf);
        display.partialUpdate(false, true);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
