#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "time.h"

static const char *TAG = "RTC_INT_ALARM";

#define REFRESH_DELAY_MS  700
#define RTC_INT_PIN       GPIO_NUM_39

static volatile int s_alarmFlag = 0;

static void IRAM_ATTR alarmISR(void *arg)
{
    s_alarmFlag = 1;
}

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

    // Configure RTC interrupt pin
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << RTC_INT_PIN);
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.intr_type    = GPIO_INTR_NEGEDGE; // FALLING edge like attachInterrupt
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(RTC_INT_PIN, alarmISR, NULL);

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();
    display.display();
    display.setTextSize(5);
    display.setTextColor(BLACK, WHITE);

    // Set time via epoch (1589610300 = 2020-05-16 06:25:00 UTC)
    esp_err_t err = display.rtc.setTime((time_t)1589610300);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

    // Set alarm 10 seconds later
    err = display.rtc.setAlarmEpoch((time_t)(1589610300 + 10));
    if (err != ESP_OK)
        ESP_LOGE(TAG, "setAlarmEpoch failed: %s", esp_err_to_name(err));

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
        display.setCursor(100, 100);
        printTime(display,
                  now.tm_hour,
                  now.tm_min,
                  now.tm_sec,
                  now.tm_mday,
                  now.tm_wday,
                  now.tm_mon,   // driver returns 1-based
                  now.tm_year); // driver returns full year e.g. 2020

        if (s_alarmFlag)
        {
            // s_alarmFlag = 0; // uncomment to clear flag like original
            display.rtc.clearAlarmFlag();
            display.setCursor(200, 200);
            display.print("ALARM");
            ESP_LOGI(TAG, "Alarm triggered!");
        }

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