/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads the on-board LSM6DS3 IMU and visualizes motion by rotating
 *              a wireframe 3D cube on the e-paper display (Inkplate 4TEMPERA).
 *
 * @details     Demonstrates how to use the built-in LSM6DS3 accelerometer and
 *              gyroscope on Inkplate 4TEMPERA. Raw accelerometer axes (X/Y/Z)
 *              and gyroscope axes (X/Y/Z) are read continuously. The numeric
 *              readings are printed on the lower half of the display for
 *              reference, and a rotating wireframe cube is drawn on the upper
 *              half. Cube rotation angles are derived from the accelerometer
 *              values and smoothed by averaging with the previous frame.
 *
 *              Each cube edge is projected from 3D to 2D using basic rotation
 *              matrices with a simple perspective projection, then drawn as
 *              lines. Partial updates keep the animation responsive; a full
 *              refresh is forced periodically to reduce ghosting.
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
 * 2) Tilt and rotate the device; the cube rotation changes with acceleration.
 * 3) Watch live accelerometer and gyroscope readings printed below the cube.
 *
 * Expected output:
 * - A wireframe cube rendered near the center of the display, rotating as the
 *   device is moved.
 * - Text readouts for ACC X/Y/Z and GYRO X/Y/Z updated continuously.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - This is a visualization example, not a calibrated orientation filter.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

static const char *TAG = "ACCEL_GYRO";

#define NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH 35
#define ANGLE_MODIFIER 0.0008f

static float cube[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}};

static int edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // bottom face
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // top face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // vertical edges
};

static void project(float *v, float ax, float ay, float az, int *px, int *py) {
  // Rotate around X
  float yr = v[1] * cosf(ax) - v[2] * sinf(ax);
  float zr = v[1] * sinf(ax) + v[2] * cosf(ax);
  float xr = v[0];
  // Rotate around Y
  float xrr = xr * cosf(ay) + zr * sinf(ay);
  float zrr = -xr * sinf(ay) + zr * cosf(ay);
  float yrr = yr;
  // Rotate around Z
  float xrrr = xrr * cosf(az) - yrr * sinf(az);
  float yrrr = xrr * sinf(az) + yrr * cosf(az);
  float zrrr = zrr;
  // Perspective projection, centered on 600x600 display
  float z = 4.0f / (4.0f + zrrr);
  *px = (int)(xrrr * z * 100.0f) + 300;
  *py = (int)(yrrr * z * 100.0f) + 300;
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.setTextSize(2);
  display.setTextColor(BLACK);

  int numRefreshes = 0;
  float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
  float prevX = 0.0f, prevY = 0.0f, prevZ = 0.0f;

  ESP_LOGI(TAG, "Starting accelerometer/gyroscope demo");

  while (true) {
    display.clearDisplay();

    float accelX = display.lsm.readRawAccelX();
    float accelY = display.lsm.readRawAccelY();
    float accelZ = display.lsm.readRawAccelZ();
    float gyroX = display.lsm.readFloatGyroX();
    float gyroY = display.lsm.readFloatGyroY();
    float gyroZ = display.lsm.readFloatGyroZ();

    display.setCursor(30, 430);
    display.print("ACC X: ");
    display.print(accelX, 4);
    display.setCursor(30, 450);
    display.print("ACC Y: ");
    display.print(accelY, 4);
    display.setCursor(30, 470);
    display.print("ACC Z: ");
    display.print(accelZ, 4);
    display.setCursor(30, 490);
    display.print("GYRO X: ");
    display.print(gyroX, 4);
    display.setCursor(30, 510);
    display.print("GYRO Y: ");
    display.print(gyroY, 4);
    display.setCursor(30, 530);
    display.print("GYRO Z: ");
    display.print(gyroZ, 4);

    // Compute angles from accelerometer, smooth by averaging with previous
    angleX = (accelX * ANGLE_MODIFIER + prevX) / 2.0f;
    angleY = (accelY * ANGLE_MODIFIER + prevY) / 2.0f;
    angleZ = (accelZ * ANGLE_MODIFIER + prevZ) / 2.0f;
    prevX = angleX;
    prevY = angleY;
    prevZ = angleZ;

    // Draw cube — note axis order matches board orientation (Y, Z, X)
    for (int i = 0; i < 12; i++) {
      int x1, y1, x2, y2;
      project(cube[edges[i][0]], angleY, angleZ, angleX, &x1, &y1);
      project(cube[edges[i][1]], angleY, angleZ, angleX, &x2, &y2);
      display.drawLine(x1, y1, x2, y2, BLACK);
    }

    if (numRefreshes > NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH) {
      display.display();
      numRefreshes = 0;
    } else {
      display.partialUpdate(false, true);
      numRefreshes++;
    }

    vTaskDelay(pdMS_TO_TICKS(30));
  }
}
