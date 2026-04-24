#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "picture1.h"
#include "picture2.h"
#include "picture3.h"

#define TIME_TO_SLEEP_US (20ULL * 1000000ULL)

static const uint8_t *pictures[] = {picture1, picture2, picture3};

RTC_DATA_ATTR static int slide = 0;

extern "C" void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE);

    display.clearDisplay();
    display.image.draw((uint8_t*)pictures[slide], 50, 0, 1100, 825, BLACK);
    display.display();

    slide++;
    if (slide > 3)
        slide = 0;

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    esp_deep_sleep_start();
}