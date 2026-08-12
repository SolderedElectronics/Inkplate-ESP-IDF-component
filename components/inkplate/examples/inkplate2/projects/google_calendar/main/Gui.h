/**
 * @file        Gui.h
 * @author      Fran Fodor for Soldered
 * @brief       Renders calendar events / errors on the Inkplate 2 e-paper
 *              display.
 *
 * @details     Ported from the original Arduino Gui class. Drawing calls
 *              are Adafruit_GFX-compatible and stay essentially unchanged;
 *              Arduino String parameters/return values are replaced with
 *              const char* / std::string, and getLocalTime() is replaced
 *              with a `struct tm` passed in by the caller (app_main()),
 *              since ESP-IDF has no Arduino-core getLocalTime() helper.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

#include "Inkplate.h"

#include <ctime>
#include <string>

class calendarData;

#define MAX_SUMMARY_LENGTH 15

/**
 * @brief Draws the calendar / error / WiFi-failure screens on Inkplate 2.
 */
class Gui {
public:
  Gui(Inkplate &inkplate);

  /**
   * @brief Draw the agenda view: one section header per day, followed by
   * that day's events (title + start/end time), until the display runs out
   * of vertical space.
   *
   * @param calendar Events to draw (already fetched via NetworkFunctions).
   * @param nowLocal Current local time (already offset-adjusted by the
   *        caller); used as a fallback year/month when deriving each
   *        event's day-of-week.
   */
  void showCalendar(calendarData *calendar, const struct tm &nowLocal);

  /// Show a generic error message.
  void showError(const char *message);

  /// Show the "WiFi connection failed" screen.
  void wifiError();

private:
  Inkplate &inkplate;

  std::string getDayName(int dayIndex);
  std::string getMonthName(int monthIndex);
  std::string getShortDayName(int dayIndex);
  std::string formatHour(const char *isoDateTime);
  std::string formatDate(const char *isoDateTime);
};
