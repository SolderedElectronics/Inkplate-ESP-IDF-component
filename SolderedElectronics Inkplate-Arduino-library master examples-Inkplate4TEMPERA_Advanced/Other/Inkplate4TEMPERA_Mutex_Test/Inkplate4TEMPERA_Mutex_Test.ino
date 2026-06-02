/**
 **************************************************
 * @file        Inkplate4TEMPERA_Mutex_Test.ino
 * @brief       Two-task display mutex test for Inkplate 4TEMPERA.
 *
 * @details     Spawns two FreeRTOS tasks that independently draw and
 *              partial-update opposite sides of the screen. Each task
 *              acquires the display mutex before touching the framebuffer
 *              or calling partialUpdate(), preventing interleaved writes
 *              that would corrupt the output.
 *
 *              Left task:  bouncing square on the left half  (x 0..299)
 *              Right task: bouncing square on the right half (x 300..599)
 *
 *              To observe what happens without locking, comment out the
 *              display.displayLock() / displayUnlock() calls.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 *
 * @author      Soldered
 * @license     GNU GPL V3
 ***************************************************/

#if !defined(ARDUINO_INKPLATE4TEMPERA)
#error "Wrong board selection — select Soldered Inkplate4TEMPERA in the boards menu."
#endif

#include "Inkplate.h"

Inkplate display(INKPLATE_1BIT);

static const int SQ = 90;
static const int DY = 5;

void taskLeft(void *pvParameters)
{
    int y = 40, dy = DY;
    while (true)
    {
        display.displayLock();
        display.fillRect(0, 0, 300, display.height(), WHITE);
        display.fillRect(105, y, SQ, SQ, BLACK);
        display.partialUpdate(false, true);
        display.displayUnlock();

        y += dy;
        if (y < 0 || y + SQ > display.height())
            dy = -dy;
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void taskRight(void *pvParameters)
{
    int y = display.height() / 2, dy = -DY;
    while (true)
    {
        display.displayLock();
        display.fillRect(300, 0, 300, display.height(), WHITE);
        display.fillRect(405, y, SQ, SQ, BLACK);
        display.partialUpdate(false, true);
        display.displayUnlock();

        y += dy;
        if (y < 0 || y + SQ > display.height())
            dy = -dy;
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void setup()
{
    Serial.begin(115200);
    display.begin();
    display.clearDisplay();
    display.drawFastVLine(300, 0, display.height(), BLACK);
    display.display();

    xTaskCreatePinnedToCore(taskLeft,  "TaskLeft",  4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(taskRight, "TaskRight", 4096, NULL, 1, NULL, 1);
}

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
