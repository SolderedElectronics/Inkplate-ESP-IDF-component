/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads gesture, proximity, RGB color, and ambient light from the
 *              on-board APDS9960 sensor (Inkplate 4TEMPERA).
 *
 * @details     Demonstrates how to use the built-in APDS9960 sensor on
 *              Inkplate 4TEMPERA. The example enables and configures proximity
 *              sensing (reduced gain), gesture sensing (reduced gain), RGB
 *              color sensing, and ambient light sensing.
 *
 *              The sketch continuously polls the sensor and updates the
 *              e-paper framebuffer only when a value changes. Gesture events
 *              (Up/Down/Left/Right) are shown when detected. The display runs
 *              in 1-bit black/white mode and uses partial updates for fast,
 *              low-flicker refreshes. A full refresh is performed periodically
 *              to reduce ghosting.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Present a hand/object near the sensor to change proximity and trigger
 *    swipe gestures.
 * 3) Shine colored/bright light toward the sensor to change light readings.
 *
 * Expected output:
 * - Live text fields for Proximity, Gesture, R/G/B channels, and Ambient light.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - All APDS9960 sub-features must be explicitly enabled before use.
 * - Example uses polling (not interrupts).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "background.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "APDS9960";

#define NUM_UPDATES_BEFORE_FULL_REFRESH 30

static bool s_dataChanged = true;
static int s_numUpdates = 0;

static uint8_t s_lastProximity = 0;
static uint16_t s_lastRed = 0, s_lastGreen = 0, s_lastBlue = 0;
static uint16_t s_lastAmbient = 0;
static char s_lastGesture[8] = "---";

static Inkplate *g_display = nullptr;

static void drawBackground() {
  g_display->clearDisplay();
  g_display->drawBitmap(0, 0, background, 600, 600, WHITE, BLACK);
}

static void updateValues() {
  g_display->setTextColor(BLACK, WHITE);

  // Proximity (top left cell)
  g_display->fillRect(21, 72, 249, 118, WHITE);
  g_display->setTextSize(4);
  g_display->setCursor(75, 125);
  g_display->print(s_lastProximity);

  // Gesture (top right cell)
  g_display->fillRect(323, 72, 263, 118, WHITE);
  g_display->setTextSize(4);
  g_display->setCursor(423, 125);
  g_display->print(s_lastGesture);

  // RGB (bottom left cell)
  g_display->fillRect(21, 409, 249, 126, WHITE);
  g_display->setTextSize(3);
  g_display->setCursor(32, 409);
  g_display->print("Red: ");
  g_display->print(s_lastRed);
  g_display->setCursor(32, 434);
  g_display->print("Green: ");
  g_display->print(s_lastGreen);
  g_display->setCursor(32, 459);
  g_display->print("Blue: ");
  g_display->print(s_lastBlue);

  // Ambient (bottom right cell)
  g_display->fillRect(323, 409, 249, 126, WHITE);
  g_display->setTextSize(4);
  g_display->setCursor(413, 425);
  g_display->print(s_lastAmbient);
}

extern "C" void app_main(void) {
  static Inkplate display;
  g_display = &display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  // Enable and configure APDS9960 sensing blocks
  display.apds.enableProximitySensor(false);
  display.apds.setProximityGain(1);
  display.apds.enableGestureSensor();
  display.apds.setGestureGain(0);
  display.apds.enableLightSensor(false);

  drawBackground();
  updateValues();
  display.display();

  ESP_LOGI(TAG, "APDS9960 demo running");

  while (true) {
    // Proximity
    uint8_t proximity = 0;
    display.apds.readProximity(proximity);
    if (proximity != s_lastProximity) {
      s_lastProximity = proximity;
      s_dataChanged = true;
    }

    // Gesture
    if (display.apds.isGestureAvailable()) {
      int g = display.apds.readGesture();
      const char *name = "---";
      if (g == DIR_UP)
        name = "Up";
      else if (g == DIR_DOWN)
        name = "Down";
      else if (g == DIR_LEFT)
        name = "Left";
      else if (g == DIR_RIGHT)
        name = "Right";
      snprintf(s_lastGesture, sizeof(s_lastGesture), "%s", name);
      s_dataChanged = true;
    }

    // RGB + Ambient
    uint16_t r = 0, gr = 0, b = 0, amb = 0;
    display.apds.readRedLight(r);
    display.apds.readGreenLight(gr);
    display.apds.readBlueLight(b);
    display.apds.readAmbientLight(amb);
    if (r != s_lastRed || gr != s_lastGreen || b != s_lastBlue) {
      s_lastRed = r;
      s_lastGreen = gr;
      s_lastBlue = b;
      s_dataChanged = true;
    }
    if (amb != s_lastAmbient) {
      s_lastAmbient = amb;
      s_dataChanged = true;
    }

    if (s_dataChanged) {
      updateValues();
      if (s_numUpdates >= NUM_UPDATES_BEFORE_FULL_REFRESH) {
        drawBackground();
        updateValues();
        display.display();
        s_numUpdates = 0;
      } else {
        display.partialUpdate(false, true);
        s_numUpdates++;
      }
      s_dataChanged = false;
    }

    vTaskDelay(pdMS_TO_TICKS(250));
  }
}
