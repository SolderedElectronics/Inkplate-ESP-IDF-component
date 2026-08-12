/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen calculator example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Draws an on-screen calculator keypad (digits, +, -, x, /, .,
 *              =, plus Refresh/Clear Input/Clear result buttons) and lets
 *              you enter numbers and perform the four basic operations
 *              entirely via the onboard touchscreen.
 *
 *              Touch input is handled with touchInArea() checks for each
 *              button, one per call to handleKeypadEvents(). After most
 *              interactions the UI is redrawn and pushed with
 *              partialUpdate() to reduce flashing and improve
 *              responsiveness; a full display() refresh is used only for
 *              the "Refresh" button. The last "expression = result" is kept
 *              on screen as a simple history line until cleared.
 *
 *              GUI layout (button rectangles/labels) and drawing helpers
 *              live in Calculator.h; this file only holds the calculator's
 *              state machine (digit/operator entry and evaluation) and the
 *              touch polling loop.
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
 * 2) Tap digits to enter a number.
 * 3) Tap an operator (+, -, x, /), enter the second number, then tap "=".
 * 4) Use "Clear Input" to reset the current entry, "Clear result" to erase
 *    the history line, and "Refresh" to redraw the full UI.
 *
 * Expected output:
 * - E-paper: calculator UI with a keypad, an input line, and a
 *   history/result line. Tapping buttons updates the UI via partial
 *   updates.
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
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Calculator.h"
#include "Inkplate.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>
#include <cstdlib>

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
static void redrawPartial(Inkplate &display) {
  display.clearDisplay();
  mainDraw(display);
  display.partialUpdate();
}

// Appends a digit ('0'-'9') to the number currently being typed.
static void enterDigit(char digit, Inkplate &display) {
  if (numOfDigitsEntered >= 6 || numOfDecimalDigitsOnCurrentNumber >= 2)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += digit;
  numOfDigitsEntered++;
  if (decimalPointOnCurrentNumber)
    numOfDecimalDigitsOnCurrentNumber++;

  redrawPartial(display);

  if (op == ' ')
    ++rightNumPos;
}

// Appends an operator token (" + ", " - ", " x ", " / ") once a left number
// has been entered and no operator has been chosen yet.
static void enterOperator(char symbol, const char *token, Inkplate &display) {
  if (op != ' ' || rightNumPos <= 0)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += token;
  op = symbol;
  decimalPointOnCurrentNumber = false;
  numOfDecimalDigitsOnCurrentNumber = 0;

  redrawPartial(display);
}

// Appends a decimal point to the number currently being typed.
static void enterDecimalPoint(Inkplate &display) {
  if (decimalPointOnCurrentNumber || numOfDigitsEntered >= 6)
    return;

  exprCursorX -= X_REZ_OFFSET;
  exprContent += ".";

  redrawPartial(display);

  if (op == ' ')
    ++rightNumPos;

  decimalPointOnCurrentNumber = true;
}

// Resets the current-entry state (used by "Clear Input" and after "=").
static void resetEntry() {
  exprContent = "";
  exprCursorX = 550;
  exprCursorY = 144;
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
// touchscreen controller, so no separate "pollTouch()" step is needed.
static void handleKeypadEvents(Inkplate &display) {
  // --- Function buttons ---

  if (display.touchscreen.touchInArea(30, 130, 150, 50)) { // Refresh
    display.clearDisplay();
    mainDraw(display);
    display.display();
    return;
  }

  if (display.touchscreen.touchInArea(30, 80, 150, 50)) { // Clear Input
    resetEntry();
    redrawPartial(display);
    return;
  }

  if (display.touchscreen.touchInArea(30, 30, 150, 50)) { // Clear result
    historyContent = "";
    historyCursorX = 240;
    historyCursorY = 81;
    redrawPartial(display);
    return;
  }

  // --- Calculate ---

  if (display.touchscreen.touchInArea(300, 471, 135, 97) && op != ' ' &&
      atof(exprContent.c_str() + rightNumPos + 3) != 0) { // "="
    result = calculate();

    char resultBuf[32];
    snprintf(resultBuf, sizeof(resultBuf), "%.2f", result);
    historyContent = exprContent + " = " + resultBuf;

    redrawPartial(display);
    resetEntry();
    result = 0;
    return;
  }

  // --- Operators ---

  if (display.touchscreen.touchInArea(435, 471, 135, 97)) { // +
    enterOperator('+', " + ", display);
    return;
  }
  if (display.touchscreen.touchInArea(435, 374, 135, 97)) { // -
    enterOperator('-', " - ", display);
    return;
  }
  if (display.touchscreen.touchInArea(435, 277, 135, 97)) { // x
    enterOperator('x', " x ", display);
    return;
  }
  if (display.touchscreen.touchInArea(435, 180, 135, 97)) { // /
    enterOperator('/', " / ", display);
    return;
  }

  // --- Decimal point ---

  if (display.touchscreen.touchInArea(30, 471, 135, 97)) { // .
    enterDecimalPoint(display);
    return;
  }

  // --- Digits ---

  if (display.touchscreen.touchInArea(165, 471, 135, 97)) { // 0
    enterDigit('0', display);
    return;
  }
  if (display.touchscreen.touchInArea(30, 374, 135, 97)) { // 1
    enterDigit('1', display);
    return;
  }
  if (display.touchscreen.touchInArea(165, 374, 135, 97)) { // 2
    enterDigit('2', display);
    return;
  }
  if (display.touchscreen.touchInArea(300, 374, 135, 97)) { // 3
    enterDigit('3', display);
    return;
  }
  if (display.touchscreen.touchInArea(30, 277, 135, 97)) { // 4
    enterDigit('4', display);
    return;
  }
  if (display.touchscreen.touchInArea(165, 277, 135, 97)) { // 5
    enterDigit('5', display);
    return;
  }
  if (display.touchscreen.touchInArea(300, 277, 135, 97)) { // 6
    enterDigit('6', display);
    return;
  }
  if (display.touchscreen.touchInArea(30, 180, 135, 97)) { // 7
    enterDigit('7', display);
    return;
  }
  if (display.touchscreen.touchInArea(168, 180, 135, 97)) { // 8
    enterDigit('8', display);
    return;
  }
  if (display.touchscreen.touchInArea(300, 180, 135, 97)) { // 9
    enterDigit('9', display);
    return;
  }
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();

  mainDraw(display);
  display.display();

  while (true) {
    // touchInArea() polls the touchscreen controller internally, so this
    // single call per button is the "poll + hit-test" step for that key.
    handleKeypadEvents(display);
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
