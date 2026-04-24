#include "Inkplate.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TIME_TO_SLEEP_US (30ULL * 1000000ULL)

RTC_DATA_ATTR static int bootCount = 0;

static void displayInfo(Inkplate &display)
{
    display.clearDisplay();

    display.setCursor(10, 280);
    display.setTextSize(2);

    display.print("Boot count: ");
    display.print(bootCount);

    display.setCursor(10, 320);

    switch (esp_sleep_get_wakeup_cause())
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        display.print("Wakeup caused by WakeUp button");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        display.print("Wakeup caused by timer");
        break;
    default:
        display.print("Wakeup was not caused by deep sleep");
        break;
    }

    display.display();
}

extern "C" void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    ++bootCount;

    displayInfo(display);

    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_US);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

    esp_deep_sleep_start();
}