#include "Inkplate.h"

extern "C"
void app_main(void) {
  Inkplate display;

  // Full refresh: white background with a black rectangle
  display.clearDisplay();
  display.fillRect(100, 100, 400, 400, INKPLATE_BLACK);
  display.display();

  vTaskDelay(pdMS_TO_TICKS(2000));

  // Draw new content only in the partial region, then partial update
  display.fillRect(150, 150, 100, 100, INKPLATE_WHITE);
  display.setCursor(155, 190);
  display.setTextSize(2);
  display.setTextColor(INKPLATE_BLACK);
  display.print("Hi!");
  display.displayPartial(150, 150, 100, 100);
}
