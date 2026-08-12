/**
 * @file        Gui.h
 * @author      Fran Fodor for Soldered
 * @brief       Draws the day/agenda calendar view on Inkplate 13SPECTRA.
 *
 * Ported from the Inkplate13SPECTRA_Google_Calendar Arduino example. Drawing
 * calls stay the same Adafruit_GFX-compatible API (setCursor/setFont/print/
 * fillRect/drawRect/...); only the String-based helpers were rewritten
 * around fixed char[] buffers.
 */
#pragma once

#include "Inkplate.h"
#include "calendarData.h"

// Event titles longer than this are truncated with "..." before drawing.
// Matches the original sketch's value for the 1200x1600 panel (much larger
// than the 6Color/448x600 port's MAX_SUMMARY_LENGTH of 20 characters).
#define MAX_SUMMARY_LENGTH 70

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

  // Color (an INKPLATE_* macro) used to highlight the currently ongoing
  // event. Defaults to INKPLATE_RED; call setHighlightColor() to change.
  // Note: Inkplate 13SPECTRA supports only 6 colors (there is no
  // INKPLATE_ORANGE on this board, unlike Inkplate 6Color).
  int highlightColor;
  void setHighlightColor(int color);

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
