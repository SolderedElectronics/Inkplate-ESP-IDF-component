/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Uses the onboard LSM6DS3 accelerometer as a hardware pedometer
 *              and displays the step count with a small walking animation
 *              (Inkplate 4TEMPERA).
 *
 * @details     Demonstrates how to use the LSM6DS3's embedded pedometer
 *              feature on Inkplate 4TEMPERA. The accelerometer is configured
 *              for +-2g / 26Hz operation and its embedded pedometer algorithm
 *              is enabled by writing directly to the LSM6DS3 control
 *              registers (the same low-level register access used by the
 *              original Arduino sketch). The example then polls the sensor's
 *              internal 16-bit step counter register; whenever the count
 *              changes, the on-screen number is updated and a small walking
 *              animation advances by one frame.
 *
 *              The display runs in 1-bit (black & white) mode to allow
 *              partial updates for fast, low-flicker refreshes. Most updates
 *              use partialUpdate(false, true) to keep the panel powered
 *              between refreshes; a full refresh is triggered after each
 *              complete animation cycle to limit ghosting.
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
 * 2) After boot, the screen shows "Start walking!" briefly.
 * 3) Walk with the device; the step count updates when new steps are
 *    detected.
 * 4) Watch the small animation advance as steps are counted.
 *
 * Expected output:
 * - E-paper: "Steps taken: <number>" plus a small animated walking icon that
 *   changes frames as you walk.
 *
 * Notes:
 * - Display mode is 1-bit (BW). Partial updates are supported only in BW
 *   mode.
 * - The LSM6DS3's embedded pedometer is not instantaneous; it may take a few
 *   steps of walking before it starts/resumes counting. This is expected
 *   behavior of the sensor's internal algorithm/filtering, not a bug in this
 *   example.
 * - This example reads the LSM6DS3 step counter registers directly.
 *   Re-enabling the embedded functions (done once at startup here) clears
 *   the step counter, so it is intentionally only done once during
 *   initialization.
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
#include "animationFrames.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "fonts/Inter16pt7b.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "PEDOMETER";

// How often to poll the LSM6DS3 step counter register (matches the original
// sketch's polling rate).
#define SAMPLING_INTERVAL_MS 200

// Number of frames in the walking animation, defined in animationFrames.h
static const int kNumAnimationFrames = sizeof(frames) / sizeof(frames[0]);

static Inkplate *g_display = nullptr;

static uint16_t s_stepsTaken = 0;
static int s_animationFrameIndex = 0;

/**
 * @brief Configures the LSM6DS3 to run its embedded pedometer algorithm.
 *
 * @return true on success, false if any register write/read failed.
 */
static bool configurePedometer() {
  uint8_t errorAccumulator = 0;
  uint8_t dataToWrite = 0;

  // Configure the accelerometer: +-2g range, 26Hz output data rate
  dataToWrite |= LSM6DS3_ACC_GYRO_FS_XL_2g;
  dataToWrite |= LSM6DS3_ACC_GYRO_ODR_XL_26Hz;
  errorAccumulator += g_display->lsm.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL,
                                                    dataToWrite) != IMU_SUCCESS;

  // Clear the ODR bit in CTRL4_C
  errorAccumulator +=
      g_display->lsm.readRegister(&dataToWrite, LSM6DS3_ACC_GYRO_CTRL4_C) !=
      IMU_SUCCESS;
  dataToWrite &= ~((uint8_t)LSM6DS3_ACC_GYRO_BW_SCAL_ODR_ENABLED);

  // Enable embedded functions -- this also clears the pedometer step count
  errorAccumulator +=
      g_display->lsm.writeRegister(LSM6DS3_ACC_GYRO_CTRL10_C, 0x3E) !=
      IMU_SUCCESS;
  // Enable the pedometer algorithm
  errorAccumulator +=
      g_display->lsm.writeRegister(LSM6DS3_ACC_GYRO_TAP_CFG1, 0x40) !=
      IMU_SUCCESS;

  return errorAccumulator == 0;
}

/**
 * @brief Reads the current 16-bit step count from the LSM6DS3's embedded
 *        pedometer step counter register.
 */
static uint16_t readStepCounter() {
  uint8_t readDataByte = 0;
  uint16_t steps = 0;

  // Read the 16-bit value as two 8-bit register reads
  g_display->lsm.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_H);
  steps = ((uint16_t)readDataByte) << 8;
  g_display->lsm.readRegister(&readDataByte, LSM6DS3_ACC_GYRO_STEP_COUNTER_L);
  steps |= readDataByte;

  return steps;
}

/**
 * @brief Draws the updated step count and advances the walking animation by
 *        one frame. Performs a full refresh once a complete animation cycle
 *        has been shown, otherwise uses a fast partial update.
 */
static void renderStepUpdate() {
  g_display->setCursor(175, 330);
  g_display->print("Steps taken: ");

  // Draw a white rectangle to clear the previously written number so a full
  // clearDisplay() (and its associated full redraw) isn't needed every time
  g_display->fillRect(359, 295, 290, 45, WHITE);
  g_display->print(s_stepsTaken);

  // Draw the next frame of the walking animation in place
  g_display->drawBitmap(275, 240, frames[s_animationFrameIndex], 50, 50,
                        BLACK, WHITE);
  s_animationFrameIndex++;

  if (s_animationFrameIndex >= kNumAnimationFrames) {
    // A full animation cycle is complete; do a full refresh to limit
    // ghosting and reset back to the first frame
    g_display->display();
    s_animationFrameIndex = 0;
  } else {
    // Otherwise, do a fast partial update while keeping the panel powered
    g_display->partialUpdate(false, true);
  }
}

extern "C" void app_main(void) {
  static Inkplate display;
  g_display = &display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.setTextColor(BLACK);
  display.setFont(&Inter16pt7b);

  if (!configurePedometer()) {
    ESP_LOGE(TAG, "Failed to configure LSM6DS3 pedometer");

    display.setCursor(50, 50);
    display.print("ERROR: can't config LSM6DS3!");
    display.display();

    // Nothing more we can do without the accelerometer; go to sleep
    esp_deep_sleep_start();
  }

  // Setup is complete, show a message to the user to start walking
  display.setCursor(200, 280);
  display.print("Start walking!");
  display.display();
  display.clearDisplay(); // Clear the frame buffer for the step readout

  ESP_LOGI(TAG, "Pedometer demo running");

  while (true) {
    uint16_t newStepsTaken = readStepCounter();

    // Only touch the display when the step count has actually changed
    if (newStepsTaken != s_stepsTaken) {
      s_stepsTaken = newStepsTaken;
      renderStepUpdate();
      ESP_LOGI(TAG, "Steps taken: %u", s_stepsTaken);
    }

    vTaskDelay(pdMS_TO_TICKS(SAMPLING_INTERVAL_MS));
  }
}
