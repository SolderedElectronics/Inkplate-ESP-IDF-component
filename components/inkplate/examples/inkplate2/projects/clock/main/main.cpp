/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Multi-style clock (digital, binary, or analog) for Soldered
 *              Inkplate 2, synced over WiFi/NTP.
 *
 * @details     Demonstrates three different clock renderings on Inkplate 2:
 *              - Digital clock: 4 large 7-segment digits (HH:MM) drawn from
 *                bitmap assets.
 *              - Binary clock: hours, minutes, day, and month shown as
 *                binary bits using filled/outlined circles.
 *              - Analog clock: clock face with hour and minute hands.
 *
 *              On boot, the example connects to WiFi (credentials configured
 *              via menuconfig) and syncs the system clock over NTP using
 *              display.wifi.setCurrentTime(). The selected clock style is
 *              chosen with CLOCK_MODE. The clock is redrawn and the e-paper
 *              display fully refreshed every REFRESH_INTERVAL_MS.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi connection with Internet access (for NTP)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> Example Configuration -> WiFi SSID/Password
 * - main.cpp   -> TIMEZONE_OFFSET_HOURS (UTC offset for your location)
 * - main.cpp   -> CLOCK_MODE (0 = digital, 1 = binary, 2 = analog)
 *
 * How to use:
 * 1) Set your WiFi SSID/password via menuconfig.
 * 2) Set TIMEZONE_OFFSET_HOURS and CLOCK_MODE below to taste.
 * 3) Build and flash to Inkplate 2.
 * 4) The device connects to WiFi, syncs time over NTP, and draws the clock.
 * 5) The clock redraws every REFRESH_INTERVAL_MS.
 *
 * Expected output:
 * - CLOCK_MODE 0 (digital): large HH:MM digits in red with a black colon.
 * - CLOCK_MODE 1 (binary): four columns showing HH, MM, DD, MM (month) as
 *   binary circles with labels and bit-value markers.
 * - CLOCK_MODE 2 (analog): a clock face with a black hour hand and a red
 *   minute hand.
 * - On WiFi failure: an error message is shown on the display and the
 *   example stops (reset the board to try again).
 *
 * Notes:
 * - Inkplate 2 has no onboard RTC chip (unlike Inkplate 4/5/6/10/13), so
 *   this example relies purely on WiFi + NTP for timekeeping. Time is not
 *   retained across a power cycle without a network connection.
 * - Inkplate 2 does not support partial updates, so every redraw is a full
 *   refresh via display.display().
 * - The original Arduino example used deep sleep between updates to save
 *   power. This port keeps the ESP32 awake and simply delays in a loop
 *   (matching this component's RTC timer example convention) since deep
 *   sleep is out of scope for this port.
 * - display.wifi.setCurrentTime() sets the system TZ internally, but
 *   time() always returns UTC seconds regardless of TZ, so this example
 *   applies TIMEZONE_OFFSET_HOURS manually (same approach as the original
 *   sketch) instead of relying on that TZ value.
 * - Bitmap digits for the digital clock were pre-generated with the
 *   Soldered Image Converter.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Bitmaps for the 7-segment digital clock digits. Generated with the
// Soldered Image Converter: https://tools.soldered.com/tools/image-converter/
#include "fonts/eight.h"
#include "fonts/five.h"
#include "fonts/four.h"
#include "fonts/nine.h"
#include "fonts/one.h"
#include "fonts/seven.h"
#include "fonts/six.h"
#include "fonts/three.h"
#include "fonts/two.h"
#include "fonts/zero.h"

static const char *TAG = "CLOCK";

//---------- CHANGE HERE -------------:

// Clock style to display:
// 0 = digital (large 7-segment style digits, HH:MM)
// 1 = binary  (hour/minute/day/month shown as binary dots)
// 2 = analog  (clock face with hour/minute hands)
#define CLOCK_MODE 1

// TODO: fill in your local timezone as a UTC offset in hours
// (e.g. 2 for UTC+2, -5 for UTC-5). WiFi SSID/password are configured
// separately via "idf.py menuconfig" -> Example Configuration.
#define TIMEZONE_OFFSET_HOURS 2

// How often to redraw and refresh the display, in milliseconds.
#define REFRESH_INTERVAL_MS (5UL * 60UL * 1000UL)

// How long to wait for a WiFi connection before giving up.
#define WIFI_CONNECT_TIMEOUT_MS (15UL * 1000UL)

//----------------------------------

// Bitmaps for the digital clock digits, indexed 0-9.
static const uint8_t *const kDigitBitmaps[10] = {zero, one, two,   three, four,
                                                  five, six, seven, eight, nine};

// Draws the digital clock: 4 large 7-segment digits (HH:MM).
static void drawDigitalClock(Inkplate &display, const tm &t) {
  display.drawBitmap(0, 10, kDigitBitmaps[t.tm_hour / 10], 48, 84,
                     INKPLATE2_RED); // First digit of hours
  display.drawBitmap(50, 10, kDigitBitmaps[t.tm_hour % 10], 48, 84,
                     INKPLATE2_RED); // Second digit of hours
  display.drawBitmap(112, 10, kDigitBitmaps[t.tm_min / 10], 48, 84,
                     INKPLATE2_RED); // First digit of minutes
  display.drawBitmap(162, 10, kDigitBitmaps[t.tm_min % 10], 48, 84,
                     INKPLATE2_RED); // Second digit of minutes

  // Colon separator between hours and minutes.
  display.fillRect(101, 32, 8, 8, INKPLATE2_BLACK);
  display.fillRect(101, 64, 8, 8, INKPLATE2_BLACK);
}

// Draws the binary clock: hour, minute, day and month as binary dots.
static void drawBinaryClock(Inkplate &display, const tm &t) {
  for (int i = 0; i < 4; i++) {
    uint8_t tens = 0, ones = 0;
    switch (i) {
    case 0: // Hours
      tens = t.tm_hour / 10;
      ones = t.tm_hour % 10;
      break;
    case 1: // Minutes
      tens = t.tm_min / 10;
      ones = t.tm_min % 10;
      break;
    case 2: // Day of month
      tens = t.tm_mday / 10;
      ones = t.tm_mday % 10;
      break;
    case 3: // Month
      tens = (t.tm_mon + 1) / 10;
      ones = (t.tm_mon + 1) % 10;
      break;
    }

    for (int bit = 3; bit >= 0; bit--) {
      // "Ones" digit column: filled red circle for a set bit, black outline
      // for a clear bit.
      if (ones & (1 << (3 - bit))) {
        display.fillCircle(30 + 50 * i, 10 + 16 * bit, 6, INKPLATE2_RED);
      } else {
        display.drawCircle(30 + 50 * i, 10 + 16 * bit, 6, INKPLATE2_BLACK);
      }

      // "Tens" digit column only needs 3 bits (max value is 5 for minutes),
      // so it's skipped on the last row.
      if (bit > 0) {
        if (tens & (1 << (3 - bit))) {
          display.fillCircle(14 + 50 * i, 10 + 16 * bit, 6, INKPLATE2_RED);
        } else {
          display.drawCircle(14 + 50 * i, 10 + 16 * bit, 6, INKPLATE2_BLACK);
        }
      }
    }
  }

  // Labels for what each column represents.
  display.setTextSize(2);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);
  display.setCursor(10, 72);
  display.print("HH");
  display.setCursor(62, 72);
  display.print("MM");
  display.setCursor(112, 72);
  display.print("DD");
  display.setCursor(162, 72);
  display.print("MM");
  display.fillRect(45, 21, 5, 5, INKPLATE2_BLACK);
  display.fillRect(45, 44, 5, 5, INKPLATE2_BLACK);
  display.fillRect(145, 58, 5, 5, INKPLATE2_BLACK);

  // Bit-value markers (8/4/2/1) for the right-hand ("ones") columns.
  display.setTextSize(1);
  display.setCursor(190, 7);
  display.print("8");
  display.setCursor(190, 23);
  display.print("4");
  display.setCursor(190, 39);
  display.print("2");
  display.setCursor(190, 55);
  display.print("1");

  display.drawLine(100, 0, 100, 104, INKPLATE2_BLACK);

  // Bit-value markers (8/4/2/1) for the left-hand ("tens") columns.
  display.setCursor(90, 7);
  display.print("8");
  display.setCursor(90, 23);
  display.print("4");
  display.setCursor(90, 39);
  display.print("2");
  display.setCursor(90, 55);
  display.print("1");
}

// Draws the analog clock: dial face with hour and minute hands.
static void drawAnalogClock(Inkplate &display, const tm &t) {
  // Outer ring.
  display.drawCircle(106, 52, 50, INKPLATE2_BLACK);
  display.drawCircle(106, 52, 51, INKPLATE2_BLACK);

  // Tick marks at 5, 10, 20, 25, 35, 40, 50, 55 minutes.
  display.drawThickLine(63, 27, 67, 30, INKPLATE2_BLACK, 1);
  display.drawThickLine(145, 74, 150, 77, INKPLATE2_BLACK, 1);
  display.drawThickLine(63, 77, 67, 75, INKPLATE2_BLACK, 1);
  display.drawThickLine(145, 30, 149, 27, INKPLATE2_BLACK, 1);
  display.drawThickLine(81, 95, 83, 91, INKPLATE2_BLACK, 1);
  display.drawThickLine(129, 13, 131, 9, INKPLATE2_BLACK, 1);
  display.drawThickLine(81, 9, 83, 13, INKPLATE2_BLACK, 1);
  display.drawThickLine(129, 91, 131, 95, INKPLATE2_BLACK, 1);

  // 3, 6, 9, 12 numerals.
  display.setTextSize(1);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);
  display.setCursor(150, 49);
  display.print('3');
  display.setCursor(103, 92);
  display.print('6');
  display.setCursor(58, 49);
  display.print('9');
  display.setCursor(101, 6);
  display.print("12");

  // Center hub.
  display.fillCircle(106, 52, 5, INKPLATE2_BLACK);

  // Angle (in radians) of the minute and hour hands.
  const float minuteAngle = (t.tm_min / 60.0f) * 2.0f * (float)M_PI;
  const float hourAngle =
      (t.tm_hour / 12.0f + t.tm_min / 720.0f) * 2.0f * (float)M_PI;

  const int xMinute = 106 + (int)(40.0f * sinf(minuteAngle));
  const int yMinute = 52 - (int)(40.0f * cosf(minuteAngle));
  const int xHour = 106 + (int)(30.0f * sinf(hourAngle));
  const int yHour = 52 - (int)(30.0f * cosf(hourAngle));

  display.drawThickLine(106, 52, xMinute, yMinute, INKPLATE2_RED,
                        2); // Minute hand
  display.drawThickLine(106, 52, xHour, yHour, INKPLATE2_BLACK,
                        3); // Hour hand
}

// Clears the framebuffer and draws the selected clock style.
static void drawClock(Inkplate &display, const tm &t) {
  display.clearDisplay();

  switch (CLOCK_MODE) {
  case 0:
    drawDigitalClock(display, t);
    break;
  case 1:
    drawBinaryClock(display, t);
    break;
  case 2:
    drawAnalogClock(display, t);
    break;
  default:
    ESP_LOGE(TAG, "Invalid CLOCK_MODE: %d", CLOCK_MODE);
    break;
  }
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setTextWrap(true);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);

  // Connect to WiFi (SSID/password configured via
  // "idf.py menuconfig" -> Example Configuration).
  if (display.wifi.begin() != ESP_OK ||
      !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
    ESP_LOGE(TAG, "WiFi connection failed");

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Unable to connect to WiFi.\nPlease check SSID and "
                  "password\nin menuconfig!");
    display.display();
    return;
  }
  ESP_LOGI(TAG, "WiFi connected");

  // Sync the system clock over NTP. This blocks until synced.
  ESP_LOGI(TAG, "Syncing time over NTP...");
  display.wifi.setCurrentTime();
  ESP_LOGI(TAG, "Time synced");

  while (true) {
    // time() always returns UTC seconds since the epoch regardless of the
    // TZ environment variable set by setCurrentTime(), so the local UTC
    // offset is applied manually here (same approach as the original
    // sketch's NetworkFunctions::getTime()).
    time_t nowUtc = time(nullptr) + (time_t)TIMEZONE_OFFSET_HOURS * 3600;
    tm t;
    gmtime_r(&nowUtc, &t);

    drawClock(display, t);
    display.display();

    vTaskDelay(pdMS_TO_TICKS(REFRESH_INTERVAL_MS));
  }
}
