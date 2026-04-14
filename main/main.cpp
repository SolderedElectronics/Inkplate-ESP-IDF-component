#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://raw.githubusercontent.com/SolderedElectronics/Inkplate-Arduino-library/refs/heads/dev/examples/Inkplate2/Advanced/WEB_WiFi/Inkplate2_Show_Pictures_From_Web/mountain.png"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.clearDisplay();    // Clear the software frame buffer (does NOT clear the physical screen)
    display.setTextSize(2);
    display.setRotation(3);

    display.wifi.begin();
    display.wifi.waitForConnect();

    display.image.draw(IMAGE_PATH, 0, 0, true, false);
    //display.print("Hello world");
    display.display();         // Refresh the e-paper display to show changes
}