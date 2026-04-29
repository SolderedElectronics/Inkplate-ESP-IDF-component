#include "Inkplate.h"

extern "C"
void app_main(void) {
  Inkplate display;

  display.clearDisplay();      // Clear the frame buffer (does NOT clear the physical screen)
  display.setCursor(10, 10);   // Set the text position to (10, 10) pixels
  display.setTextSize(3);      // Set text size to 3 (default is 1)
  display.setTextColor(BLACK); // Set text color to black (default is white)
  display.print("Hello World!"); // Print "Hello World!" at the set position
  display.display();           // Refresh the e-paper display to show changes
}