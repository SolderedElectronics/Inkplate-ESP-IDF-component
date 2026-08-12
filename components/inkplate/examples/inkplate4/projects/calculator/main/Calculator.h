/**
 * @file        Calculator.h
 * @author      Fran Fodor for Soldered
 * @brief       Keypad layout and screen drawing helpers for the touchscreen
 *              calculator example (Soldered Inkplate 4TEMPERA).
 *
 * @details     Holds the on-screen geometry for the calculator UI (keypad
 *              button rectangles/labels, the two function buttons, and the
 *              two background display boxes) plus the mutable text state for
 *              the current expression line and the last "expr = result"
 *              history line. mainDraw() renders the whole UI in one pass and
 *              is called by main.cpp after every state change.
 *
 *              This header is included by exactly one translation unit
 *              (main.cpp), so the state variables below are defined here
 *              directly rather than behind extern declarations, mirroring
 *              the layout of the original Arduino sketch.
 *
 *              `display` is NOT a global here: it's constructed as a local
 *              in app_main() and passed by reference into mainDraw() (and,
 *              in main.cpp, into every other function that touches it).
 *              A global `Inkplate display;` would race the library's own
 *              global I2C/PCAL peripheral objects (in BoardCommon.cpp) —
 *              C++ leaves cross-translation-unit static init order
 *              unspecified, so the Inkplate ctor can run before the I2C
 *              bus/expander objects it depends on, leaving the touchscreen
 *              controller I2C handle uninitialized.
 *
 * @license     GNU GPL V3
 */

#pragma once

#include "Inkplate.h"
#include "fonts/FreeSansBold12pt7b.h"
#include "fonts/Roboto_Light_36.h"

#include <string>

// Horizontal step (in pixels) applied per typed character so the expression
// line grows leftward from a fixed right edge, matching the original
// Arduino sketch's behavior.
#define X_REZ_OFFSET 15

// A single keypad key: its touch/draw rectangle and its printed label.
struct KeypadKey {
  int x, y, w, h;
  int labelX, labelY;
  const char *label;
};

// 4x4 numeric/operator keypad (bottom of the screen).
// Order/geometry matches rect0..rect15 / text2..text17 in the original
// Arduino Calculator.h.
static const KeypadKey kKeypad[16] = {
    {30, 180, 135, 97, 90, 240, "7"},
    {168, 180, 135, 97, 225, 240, "8"},
    {300, 180, 135, 97, 360, 240, "9"},
    {435, 180, 135, 97, 495, 240, "/"},

    {30, 277, 135, 97, 90, 337, "4"},
    {165, 277, 135, 97, 225, 337, "5"},
    {300, 277, 135, 97, 360, 337, "6"},
    {435, 277, 135, 97, 495, 337, "x"},

    {30, 374, 135, 97, 90, 434, "1"},
    {165, 374, 135, 97, 225, 434, "2"},
    {300, 374, 135, 97, 360, 434, "3"},
    {435, 374, 135, 97, 495, 435, "-"},

    {30, 471, 135, 97, 90, 531, "."},
    {165, 471, 135, 97, 225, 531, "0"},
    {300, 471, 135, 97, 360, 531, "="},
    {435, 471, 135, 97, 495, 531, "+"},
};

// The 3 stacked function buttons (top-left of the screen).
// Matches rect18..rect20 / text20..text22 in the original.
static const KeypadKey kFunctionKeys[3] = {
    {30, 130, 150, 50, 40, 160, "Refresh"},
    {30, 80, 150, 50, 40, 110, "Clear Input"},
    {30, 30, 150, 50, 40, 60, "Clear result"},
};

// Background boxes behind the history line and the current-expression line.
// Drawn as plain (unfilled) borders; rect17 is the outer panel border and
// rect16 doubles as the divider between the history row and the input row.
static constexpr int kHistoryBoxX = 200, kHistoryBoxY = 30, kHistoryBoxW = 370,
                      kHistoryBoxH = 75;
static constexpr int kDisplayBoxX = 200, kDisplayBoxY = 30, kDisplayBoxW = 370,
                      kDisplayBoxH = 150;

// Current expression being typed (e.g. "12 + 7"), right-aligned: the cursor
// x-position is decremented by X_REZ_OFFSET for every character appended so
// the text grows to the left from a fixed right edge.
inline std::string exprContent = "";
inline int exprCursorX = 550;
inline int exprCursorY = 144;

// Last completed calculation, shown as "<expr> = <result>".
inline std::string historyContent = "";
inline int historyCursorX = 240;
inline int historyCursorY = 81;

// Draws the full calculator UI: keypad, function buttons, display boxes and
// the current expression/history text. Called after every state change;
// the caller is responsible for clearDisplay()/display()/partialUpdate().
inline void mainDraw(Inkplate &display) {
  // Numeric/operator keypad.
  display.setFont(&Roboto_Light_36);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);
  for (const KeypadKey &key : kKeypad) {
    display.drawRect(key.x, key.y, key.w, key.h, BLACK);
    display.setCursor(key.labelX, key.labelY);
    display.print(key.label);
  }

  // History box + text.
  display.drawRect(kHistoryBoxX, kHistoryBoxY, kHistoryBoxW, kHistoryBoxH, BLACK);
  // Outer display panel border (also visually divides history/input rows).
  display.drawRect(kDisplayBoxX, kDisplayBoxY, kDisplayBoxW, kDisplayBoxH, BLACK);

  display.setFont(&FreeSansBold12pt7b);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);

  display.setCursor(historyCursorX, historyCursorY);
  display.print(historyContent.c_str());

  display.setCursor(exprCursorX, exprCursorY);
  display.print(exprContent.c_str());

  // Function buttons (Refresh / Clear Input / Clear result).
  for (const KeypadKey &key : kFunctionKeys) {
    display.drawRect(key.x, key.y, key.w, key.h, BLACK);
    display.setCursor(key.labelX, key.labelY);
    display.print(key.label);
  }
}
