/**
 * @file        Calculator.h
 * @author      Fran Fodor for Soldered
 * @brief       Keypad layout and screen drawing helpers for the touchscreen
 *              calculator example (Soldered Inkplate 6 Flick).
 *
 * @details     Holds the on-screen geometry for the calculator UI (the 4x4
 *              numeric/operator keypad, the three function buttons, and the
 *              two background display boxes) plus the mutable text state for
 *              the current expression line and the running calculation
 *              history. mainDraw() renders the whole UI in one pass and is
 *              called by main.cpp after every state change.
 *
 *              This header is included by exactly one translation unit
 *              (main.cpp), so the state variables below are defined here
 *              directly rather than behind extern declarations, mirroring
 *              the layout of the original Arduino sketch.
 *
 * @license     GNU GPL V3
 */

#pragma once

#include "Inkplate.h"
#include "fonts/FreeSansBold24pt7b.h"

#include <string>

// Horizontal step (in pixels) applied per typed character so the expression
// line grows leftward from a fixed right edge, matching the original
// Arduino sketch's behavior.
#define X_REZ_OFFSET 15

// Defined in main.cpp.
extern Inkplate display;

// A single keypad key: its draw rectangle and its printed label. Touch hit
// areas are checked separately in main.cpp with the exact coordinates used
// by the original Arduino sketch (some function buttons there use a touch
// area that is slightly larger/smaller than the drawn rectangle).
struct KeypadKey {
  int x, y, w, h;
  int labelX, labelY;
  const char *label;
};

// 4x4 numeric/operator keypad (bottom-right of the screen).
// Geometry/labels match rect0..rect15 / text2..text17 in the original
// Arduino Calculator.h.
static const KeypadKey kKeypad[16] = {
    {600, 350, 100, 100, 640, 420, "7"},
    {700, 350, 100, 100, 740, 420, "8"},
    {800, 350, 100, 100, 840, 420, "9"},
    {900, 350, 100, 100, 940, 420, "/"},

    {600, 450, 100, 100, 640, 520, "4"},
    {700, 450, 100, 100, 740, 520, "5"},
    {800, 450, 100, 100, 840, 520, "6"},
    {900, 450, 100, 100, 940, 520, "x"},

    {600, 550, 100, 100, 640, 620, "1"},
    {700, 550, 100, 100, 740, 620, "2"},
    {800, 550, 100, 100, 840, 620, "3"},
    {900, 550, 100, 100, 940, 620, "-"},

    {600, 650, 100, 100, 640, 720, "."},
    {700, 650, 100, 100, 740, 715, "0"},
    {800, 650, 100, 100, 840, 720, "="},
    {900, 650, 100, 100, 940, 720, "+"},
};

// The 3 function buttons (Refresh / Clear / Clear history).
// Draw geometry matches rect18..rect20 / text20..text22 in the original;
// note the *touch* areas checked in main.cpp are the original sketch's
// touchInArea() values, which for these three buttons don't exactly match
// the drawn rectangle below (a quirk carried over from the source sketch).
static const KeypadKey kFunctionKeys[3] = {
    {800, 20, 200, 53, 820, 60, "Refresh"},
    {600, 20, 200, 53, 620, 60, "Clear"},
    {50, 50, 150, 50, 60, 95, "Clear"},
};

// Background boxes behind the current-expression line and the history panel.
// Drawn as plain (unfilled) borders, matching rect16/rect17 in the original.
static constexpr int kExpressionBoxX = 600, kExpressionBoxY = 100,
                      kExpressionBoxW = 400, kExpressionBoxH = 200;
static constexpr int kHistoryBoxX = 50, kHistoryBoxY = 50, kHistoryBoxW = 500,
                      kHistoryBoxH = 680;

// Current expression being typed (e.g. "12 + 7"), right-aligned: the cursor
// x-position is decremented by X_REZ_OFFSET for every character appended so
// the text grows to the left from a fixed right edge.
inline std::string exprContent = "";
inline int exprCursorX = 800;
inline int exprCursorY = 260;

// Running calculation history. Every completed "expr = result" is appended
// on its own line, indented to line up under the first entry; the cursor's
// y-position is stepped up by 55px per extra line to keep everything inside
// the history panel, matching the original sketch's behavior.
inline std::string historyContent = "";
inline int historyCursorX = 50;
inline int historyCursorY = 700;

// Draws the full calculator UI: keypad, function buttons, display boxes and
// the current expression/history text. Called after every state change;
// the caller is responsible for clearDisplay()/display()/partialUpdate().
inline void mainDraw() {
  display.setFont(&FreeSansBold24pt7b);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(1);

  // Numeric/operator keypad.
  for (const KeypadKey &key : kKeypad) {
    display.drawRect(key.x, key.y, key.w, key.h, BLACK);
    display.setCursor(key.labelX, key.labelY);
    display.print(key.label);
  }

  // Display boxes (current expression + history panel).
  display.drawRect(kExpressionBoxX, kExpressionBoxY, kExpressionBoxW,
                    kExpressionBoxH, BLACK);
  display.drawRect(kHistoryBoxX, kHistoryBoxY, kHistoryBoxW, kHistoryBoxH,
                    BLACK);

  display.setCursor(exprCursorX, exprCursorY);
  display.print(exprContent.c_str());

  display.setCursor(historyCursorX, historyCursorY);
  display.print(historyContent.c_str());

  // Function buttons (Refresh / Clear / Clear history).
  for (const KeypadKey &key : kFunctionKeys) {
    display.drawRect(key.x, key.y, key.w, key.h, BLACK);
    display.setCursor(key.labelX, key.labelY);
    display.print(key.label);
  }
}
