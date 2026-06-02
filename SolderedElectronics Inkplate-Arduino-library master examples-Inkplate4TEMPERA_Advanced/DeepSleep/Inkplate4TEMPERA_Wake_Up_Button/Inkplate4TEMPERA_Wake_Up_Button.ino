/**
 **************************************************
 * @file        Inkplate10_Wake_Up_Button.ino
 * @brief       Wake-up button and timer deep sleep example for Soldered Inkplate 10.
 *
 * @details     Demonstrates how to wake the ESP32 from deep sleep on Inkplate 10
 *              using an external interrupt (WakeUp button) and a fallback timer.
 *              The example stores a boot counter in RTC memory, shows the boot
 *              count on the e-paper display, and prints the wake-up reason
 *              (button press vs. timer wake-up).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Hardware:   Inkplate 10, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate10
 * - Serial settings: Not required
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/10/quick-start-guide/
 *
 * How to use:
 * 1) Upload the sketch to Inkplate 10.
 * 2) After displaying boot info, the board enters deep sleep.
 * 3) Wake the board by pressing the WakeUp button, or wait 30 seconds for timer wake-up.
 * 4) On each wake, the display updates with the new boot count and wake-up reason.
 *
 * Expected output:
 * - Inkplate display shows an incrementing boot count.
 * - Wake-up reason is shown as either WakeUp button or timer.
 *
 * Notes:
 * - Deep sleep restarts the program from the beginning on every wake-up.
 * - bootCount is stored in RTC memory (RTC_DATA_ATTR) so it persists across deep sleep.
 * - WakeUp button wake uses EXT0 wake-up on GPIO36.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2022-12-14
 * @license     GNU GPL V3
 **************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Soldered Inkplate4TEMPERA in the boards menu."
#endif

// Include Inkplate library to the sketch
#include <Inkplate.h>

// Create an object on Inkplate library and also set library into 1-bit mode (BW)
Inkplate display(INKPLATE_1BIT);

// Conversion factor for micro seconds to seconds
#define uS_TO_S_FACTOR 1000000ULL

// Time ESP32 will go to sleep (in seconds)
#define TIME_TO_SLEEP 30

// Store int in rtc data, to remain persistent during deep sleep
RTC_DATA_ATTR int bootCount = 0;

void setup()
{
    // Init Inkplate library (you should call this function ONLY ONCE)
    display.begin();

    ++bootCount;

    // Our function declared below
    displayInfo();

    // Go to sleep for TIME_TO_SLEEP seconds
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    // Enable wakeup from deep sleep on gpio 36 (wake button)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

    // Start deep sleep (this function does not return). Program stops here.
    esp_deep_sleep_start();
}

void loop()
{
    // Never here! If you use deep sleep, the whole program should be in setup() because the board restarts each
    // time. loop() must be empty!
}

// Function that will write number of boots and boot reason to screen
void displayInfo()
{
    // First, lets delete everything from frame buffer
    display.clearDisplay();

    // Set text cursor and size
    display.setCursor(30, 40);
    display.setTextSize(2);

    display.print(F("Boot count: "));
    display.println(bootCount, DEC); // Print the number

    // Set next line cursor position
    display.setCursor(30, 80);

    // Display wake up reason
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch (wakeup_reason)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        display.println("Wakeup caused by WakeUp button");
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        display.println("Wakeup caused by timer");
        break;
    default:
        display.println("Wakeup was not caused by deep sleep");
        break;
    }

    // Show everything on the screen
    display.display();
}
