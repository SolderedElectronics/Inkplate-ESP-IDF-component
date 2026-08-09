/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen calculator example for Soldered Inkplate 6 Flick.
 *
 * @details     Draws an on-screen calculator keypad (digits, +, -, x, /, .,
 *              =, plus Refresh/Clear/Clear history buttons) and lets you
 *              enter numbers and perform the four basic operations entirely
 *              via the onboard touchscreen.
 *
 *              Touch input is handled with touchInArea() checks for each
 *              button, one per call to handleKeypadEvents(). After most
 *              interactions the UI is redrawn and pushed with
 *              partialUpdate() to reduce flashing and improve
 *              responsiveness; a full display() refresh is used only for
 *              the "Refresh" button. Unlike the Inkplate 4TEMPERA port of
 *              this example, completed calculations are appended to a
 *              running, multi-line history panel rather than replacing a
 *              single history line, matching the original sketch.
 *
 *              GUI layout (button rectangles/labels) and drawing helpers
 *              live in Calculator.h; this file only holds the calculator's
 *              state machine (digit/operator entry and evaluation) and the
 *              touch polling loop.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) Tap digits to enter a number.
 * 3) Tap an operator (+, -, x, /), enter the second number, then tap "=".
 * 4) Use "Clear" to reset the current entry, "Clear history" (top-left) to
 *    erase the history panel, and "Refresh" to redraw the full UI.
 *
 * Expected output:
 * - E-paper: calculator UI with a numeric/operator keypad, a current-entry
 *   line, and a running history panel. Tapping buttons updates the UI via
 *   partial updates.
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode; partial updates
 *   are only supported in that mode.
 * - For best image quality, perform a full refresh periodically (the
 *   "Refresh" button); repeated partial updates can leave artifacts on
 *   e-paper.
 * - Division by zero is guarded before calculating: the right operand must
 *   be non-zero to trigger the "=" action.
 * - Numbers are limited to 6 digits with at most 2 decimal digits, matching
 *   the original sketch's input limits.
 * - The touchscreen (Cypress CY8CTMA140 controller, via TouchCypress) is
 *   initialized and powered on automatically as part of constructing
 *   `Inkplate display;` — no explicit `touchscreen.init()` call is needed
 *   on this port. This differs from the original `.ino`, which called
 *   `display.touchscreen.init(true)` in `setup()`.
 * - `touchInArea(x, y, w, h)` has the same signature here as on the
 *   Inkplate 4TEMPERA (TouchElan) port of this example, but is backed by a
 *   different driver (TouchCypress, for the CY8CTMA140 controller used on
 *   Inkplate 6 Flick) — see include/features/TouchCypress.h.
 * - Board geometry (keypad/button coordinates) uses the 1024x758 layout
 *   from the original Inkplate 6 Flick sketch, not the smaller layout used
 *   by the Inkplate 4TEMPERA port of this same example.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Calculator.h"
#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>
#include <cstdlib>

// Create the Inkplate object (declared extern in Calculator.h).
Inkplate display;

// Calculator state, matching the original Arduino sketch's globals.
static double leftNumber = 0;
static double rightNumber = 0;
static char op = ' ';
static double result = 0;
static int rightNumPos = 0;
static bool decimalPointOnCurrentNumber = false;
static int numOfDecimalDigitsOnCurrentNumber = 0;
static int numOfDigitsEntered = 0;

// Redraws the whole UI and pushes it with a fast partial update. Used after
// every keypad interaction except "Refresh" (which does a full refresh).
static void redrawPartial() {
  display.clearDisplay();
  mainDraw();
  display.partialUpdate();
}

// Appends a digit ('0'-'9') to the number currently being typed.
static void enterDigit(char digit) {
  if (numOfDigitsEntered >= 6 || numOfDecimalDigitsOnCurrentNumber >= 2)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += digit;
  numOfDigitsEntered++;
  if (decimalPointOnCurrentNumber)
    numOfDecimalDigitsOnCurrentNumber++;

  redrawPartial();

  if (op == ' ')
    ++rightNumPos;
}

// Appends an operator token (" + ", " - ", " x ", " / ") once a left number
// has been entered and no operator has been chosen yet.
static void enterOperator(char symbol, const char *token) {
  if (op != ' ' || rightNumPos <= 0)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += token;
  op = symbol;
  decimalPointOnCurrentNumber = false;
  numOfDecimalDigitsOnCurrentNumber = 0;

  redrawPartial();
}

// Appends a decimal point to the number currently being typed.
static void enterDecimalPoint() {
  if (decimalPointOnCurrentNumber || numOfDigitsEntered >= 6)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += ".";

  redrawPartial();

  if (op == ' ')
    ++rightNumPos;

  decimalPointOnCurrentNumber = true;
}

// Resets the current-entry state (used by "Clear" and after "=").
static void resetEntry() {
  exprContent = "";
  exprCursorX = 800;
  exprCursorY = 260;
  op = ' ';
  rightNumPos = 0;
  decimalPointOnCurrentNumber = false;
  numOfDigitsEntered = 0;
  numOfDecimalDigitsOnCurrentNumber = 0;
}

// Performs the pending calculation based on op/leftNumber/rightNumber.
static double calculate() {
  double res = 0;
  rightNumPos += 3;

  leftNumber = atof(exprContent.c_str());
  rightNumber = atof(exprContent.c_str() + rightNumPos);
  rightNumPos = 0;

  switch (op) {
  case '+':
    res = leftNumber + rightNumber;
    break;
  case '-':
    res = leftNumber - rightNumber;
    break;
  case 'x':
    res = leftNumber * rightNumber;
    break;
  case '/':
    res = leftNumber / rightNumber;
    break;
  default:
    break;
  }

  return res;
}

// Checks every keypad/function button's touch area and handles it. Mirrors
// the original sketch's keysEvents(): touchInArea() itself polls the
// touchscreen controller, so no separate "pollTouch()" step is needed. The
// touch areas below are transcribed verbatim from the original .ino (some
// function buttons use a hit area that doesn't exactly match the drawn
// rectangle in Calculator.h; that quirk is carried over unchanged).
static void handleKeypadEvents() {
  // --- Function buttons ---

  if (display.touchscreen.touchInArea(800, 20, 200, 80)) { // Refresh
    display.clearDisplay();
    mainDraw();
    display.display();
    return;
  }

  if (display.touchscreen.touchInArea(600, 20, 200, 80)) { // Clear
    resetEntry();
    redrawPartial();
    return;
  }

  if (display.touchscreen.touchInArea(50, 50, 100, 50)) { // Clear history
    historyContent = "";
    historyCursorX = 50;
    historyCursorY = 700;
    redrawPartial();
    return;
  }

  // --- Calculate ---

  if (display.touchscreen.touchInArea(800, 650, 100, 100) && op != ' ' &&
      atof(exprContent.c_str() + rightNumPos + 3) != 0) { // "="
    result = calculate();

    char resultBuf[32];
    snprintf(resultBuf, sizeof(resultBuf), "%.2f", result);

    if (historyContent.empty()) {
      historyContent = exprContent + " = " + resultBuf;
    } else {
      historyCursorY -= 55;
      historyContent =
          historyContent + '\n' + "    " + exprContent + " = " + resultBuf;
    }

    redrawPartial();
    resetEntry();
    result = 0;
    return;
  }

  // --- Operators ---

  if (display.touchscreen.touchInArea(900, 650, 100, 100)) { // +
    enterOperator('+', " + ");
    return;
  }
  if (display.touchscreen.touchInArea(900, 550, 100, 100)) { // -
    enterOperator('-', " - ");
    return;
  }
  if (display.touchscreen.touchInArea(900, 450, 100, 100)) { // x
    enterOperator('x', " x ");
    return;
  }
  if (display.touchscreen.touchInArea(900, 350, 100, 100)) { // /
    enterOperator('/', " / ");
    return;
  }

  // --- Decimal point ---

  if (display.touchscreen.touchInArea(600, 650, 100, 100)) { // .
    enterDecimalPoint();
    return;
  }

  // --- Digits ---

  if (display.touchscreen.touchInArea(700, 650, 100, 100)) { // 0
    enterDigit('0');
    return;
  }
  if (display.touchscreen.touchInArea(600, 550, 100, 100)) { // 1
    enterDigit('1');
    return;
  }
  if (display.touchscreen.touchInArea(700, 550, 100, 100)) { // 2
    enterDigit('2');
    return;
  }
  if (display.touchscreen.touchInArea(800, 550, 100, 100)) { // 3
    enterDigit('3');
    return;
  }
  if (display.touchscreen.touchInArea(600, 450, 100, 100)) { // 4
    enterDigit('4');
    return;
  }
  if (display.touchscreen.touchInArea(700, 450, 100, 100)) { // 5
    enterDigit('5');
    return;
  }
  if (display.touchscreen.touchInArea(800, 450, 100, 100)) { // 6
    enterDigit('6');
    return;
  }
  if (display.touchscreen.touchInArea(600, 350, 100, 100)) { // 7
    enterDigit('7');
    return;
  }
  if (display.touchscreen.touchInArea(700, 350, 100, 100)) { // 8
    enterDigit('8');
    return;
  }
  if (display.touchscreen.touchInArea(800, 350, 100, 100)) { // 9
    enterDigit('9');
    return;
  }
}

extern "C" void app_main(void) {
  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();

  mainDraw();
  display.display();

  while (true) {
    // touchInArea() polls the touchscreen controller internally, so this
    // single call per button is the "poll + hit-test" step for that key.
    handleKeypadEvents();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
