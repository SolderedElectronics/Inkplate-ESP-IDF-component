#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "Inkplate.h"

static const char *TAG = "MAIN";

//#define SD_CARD

// Image file on the SD card root (change extension to match your file)
#define IMAGE_PATH "https://upload.wikimedia.org/wikipedia/commons/c/c2/Auckland_Skyline_800x600.jpg"
#include "logo.h"

int logo_w = 1100;
int logo_h = 221;
extern "C"
void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE);

    display.image.draw(logo, 50, 301, logo_w, logo_h, BLACK); // Arguments are: array variable name, start X, start Y, size X, size Y, color
    display.display();

    return;


#ifdef SD_CARD
    if (display.sdCardInit() != ESP_OK)
    {
      ESP_LOGE(TAG, "SD card init failed");
      return;
    }
#else
    display.wifi.begin();
    display.wifi.waitForConnect();
#endif

    display.image.draw(IMAGE_PATH, 0, 0, true, false);
    display.display();
    
}