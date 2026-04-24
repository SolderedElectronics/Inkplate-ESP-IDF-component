#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "time.h"

static const char *TAG = "RTC_SIMPLE";

#define REFRESH_DELAY_MS 1000

// Set clock
static uint8_t  s_hour    = 12;
static uint8_t  s_minutes = 50;
static uint8_t  s_seconds = 30;

// Set date (weekday: 0=Sunday, 1=Monday, ...)
static uint8_t  s_weekday = 4;
static uint8_t  s_day     = 11;
static uint8_t  s_month   = 11;
static uint16_t s_year    = 2021;

static void print2Digits(Inkplate &display, uint8_t d)
{
    if (d < 10)
        display.print('0');
    display.print(d);
}

static void printTime(Inkplate &display, uint8_t hour, uint8_t minutes, uint8_t seconds,
                      uint8_t day, uint8_t weekday, uint8_t month, uint16_t year)
{
    const char *wday[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                          "Thursday", "Friday", "Saturday"};

    print2Digits(display, hour);
    display.print(':');
    print2Digits(display, minutes);
    display.print(':');
    print2Digits(display, seconds);
    display.print(' ');
    display.print(wday[weekday % 7]);
    display.print(", ");
    print2Digits(display, day);
    display.print('/');
    print2Digits(display, month);
    display.print('/');
    display.print(year);
}

extern "C" void app_main(void)
{
    Inkplate display;

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();
    display.display();
    display.setTextSize(5);
    display.setTextColor(BLACK, WHITE);

    tm t = {};
    t.tm_hour = s_hour;
    t.tm_min  = s_minutes;
    t.tm_sec  = s_seconds;
    t.tm_wday = s_weekday;
    t.tm_mday = s_day;
    t.tm_mon  = s_month;  // driver expects 1-based
    t.tm_year = s_year;   // driver expects full year

    esp_err_t err = display.rtc.setTime(t);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

    int n = 0;

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

        tm now = {};
        err = display.rtc.getTime(&now);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
            continue;
        }

        display.clearDisplay();
        display.setCursor(100, 300);
        printTime(display,
                  now.tm_hour,
                  now.tm_min,
                  now.tm_sec,
                  now.tm_mday,
                  now.tm_wday,
                  now.tm_mon,   // driver returns 1-based
                  now.tm_year); // driver returns full year

        if (n > 9)
        {
            display.display();
            n = 0;
        }
        else
        {
            display.partialUpdate(false, true);
            n++;
        }
    }
}