/**
 * @file        Gui.h
 * @author      Fran Fodor for Soldered
 * @brief       Draws the day/agenda calendar view on Inkplate 10.
 *
 * Ported from the Inkplate10_Google_Calendar Arduino example. Drawing calls
 * stay the same Adafruit_GFX-compatible API (setCursor/setFont/print/
 * fillRect/drawRect/...); only the String-based helpers were rewritten
 * around fixed char[] buffers.
 */
#pragma once

#include "Inkplate.h"
#include "calendarData.h"

// Event titles longer than this are truncated with "..." before drawing.
// Wider than the Inkplate 6 port's 16 chars, matching the original sketch's
// wider event box on the Inkplate 10's larger 825x1200 portrait screen.
#define MAX_SUMMARY_LENGTH 28

/**
 * @brief Renders the fetched calendarData (and error/status screens) on an
 *        Inkplate display.
 */
class Gui {
public:
  explicit Gui(Inkplate &inkplate);

  /// Draw the day/agenda calendar view for the events in `calendar`.
  void showCalendar(calendarData *calendar);

  /// Draw a generic full-screen error message.
  void showError(const char *message);

  /// Draw the "WiFi connection failed" screen.
  void wifiError();

private:
  Inkplate &inkplate;

  void drawHeader(const char *title);
  const char *getDayName(int dayIndex);
  const char *getMonthName(int monthIndex);
  const char *getShortDayName(int dayIndex);

  // Extracts "HH:MM" from an RFC3339 timestamp into `out` (empty if too short).
  void formatHour(const char *isoDateTime, char *out, size_t outSize);
  // Extracts the two-digit day-of-month from an RFC3339 timestamp into `out`.
  void formatDate(const char *isoDateTime, char *out, size_t outSize);

  bool isCurrentEvent(const char *startTimeStr, const char *endTimeStr);
};
