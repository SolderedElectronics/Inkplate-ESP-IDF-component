/**
 * @file        Gui.cpp
 * @brief       Draws the day/agenda calendar view on Inkplate 10.
 *
 * Layout note: the Inkplate10_Google_Calendar Arduino sketch this was ported
 * from already lays its coordinates out for the Inkplate 10's 825x1200
 * portrait screen (setRotation(1) turns the native 1200x825 panel into
 * 825 wide x 1200 tall) rather than reusing the Inkplate 6 sketch's
 * 605x800 numbers, so this port keeps those Inkplate-10-specific values
 * (wider header bar, more left/right margin, taller event list) instead of
 * copying the Inkplate 6 port's coordinates.
 */
#include "Gui.h"
#include "includes.h" // getLocalTimeAdjusted()

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Fonts
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"
#include "fonts/FreeSansBold48pt7b.h"

namespace {

/**
 * @brief Convert struct tm fields (interpreted as-is, ignoring any host TZ)
 *        into a Unix epoch value.
 *
 * Used instead of mktime()/timegm() so the result never depends on the C
 * library's TZ environment variable - every timestamp handled by this file
 * is already a "local" wall-clock value (see includes.h).
 */
time_t tmFieldsToEpoch(const struct tm *t) {
  static const int cumulativeDays[] = {0,   31,  59,  90,  120, 151,
                                       181, 212, 243, 273, 304, 334};
  auto isLeap = [](int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
  };

  int year = t->tm_year + 1900;
  int month = t->tm_mon; // 0-11

  long days = 0;
  if (year >= 1970) {
    for (int y = 1970; y < year; y++)
      days += isLeap(y) ? 366 : 365;
  } else {
    for (int y = year; y < 1970; y++)
      days -= isLeap(y) ? 366 : 365;
  }
  days += cumulativeDays[month];
  if (month > 1 && isLeap(year))
    days += 1;
  days += t->tm_mday - 1;

  return (time_t)days * 86400L + t->tm_hour * 3600L + t->tm_min * 60L +
        t->tm_sec;
}

/// Day-of-week (0=Sunday) for a given date, via Sakamoto's algorithm.
int computeWeekday(int year, int month1to12, int day) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  int y = year;
  if (month1to12 < 3)
    y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[month1to12 - 1] + day) % 7;
}

/// Parse "YYYY-MM-DDTHH:MM" out of an RFC3339 timestamp into a struct tm.
/// Ignores any trailing timezone offset/"Z", matching the original sketch's
/// behavior (it never accounted for the offset suffix either).
bool parseIsoDateTime(const char *iso, struct tm *out) {
  if (!iso || strlen(iso) < 16)
    return false;
  memset(out, 0, sizeof(*out));
  int year, mon, day, hour, min;
  if (sscanf(iso, "%4d-%2d-%2dT%2d:%2d", &year, &mon, &day, &hour, &min) != 5)
    return false;
  out->tm_year = year - 1900;
  out->tm_mon = mon - 1;
  out->tm_mday = day;
  out->tm_hour = hour;
  out->tm_min = min;
  return true;
}

} // namespace

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate) {}

const char *Gui::getDayName(int dayIndex) {
  static const char *days[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                               "Thursday", "Friday", "Saturday"};
  return days[dayIndex % 7];
}

const char *Gui::getMonthName(int monthIndex) {
  static const char *months[] = {"January",   "February", "March",
                                 "April",     "May",      "June",
                                 "July",      "August",   "September",
                                 "October",   "November", "December"};
  return months[monthIndex % 12];
}

const char *Gui::getShortDayName(int dayIndex) {
  static const char *shortDays[] = {"Sun", "Mon", "Tue", "Wed",
                                    "Thu", "Fri", "Sat"};
  return shortDays[dayIndex % 7];
}

void Gui::formatHour(const char *isoDateTime, char *out, size_t outSize) {
  out[0] = '\0';
  if (!isoDateTime || strlen(isoDateTime) < 16)
    return;
  snprintf(out, outSize, "%.5s", isoDateTime + 11); // "HH:MM"
}

void Gui::formatDate(const char *isoDateTime, char *out, size_t outSize) {
  out[0] = '\0';
  if (!isoDateTime || strlen(isoDateTime) < 10)
    return;
  snprintf(out, outSize, "%.2s", isoDateTime + 8); // day-of-month digits
}

void Gui::wifiError() {
  inkplate.clearDisplay();
  inkplate.setTextColor(0);
  inkplate.setFont(&FreeSans18pt7b);
  inkplate.setCursor(50, 150);
  inkplate.print("WiFi connection failed.");
  inkplate.setCursor(50, 200);
  inkplate.print("Check credentials or try again.");
  inkplate.display();
}

void Gui::drawHeader(const char *title) {
  inkplate.clearDisplay();
  inkplate.setTextSize(3);
  inkplate.setTextColor(0);
  inkplate.setCursor(10, 10);
  inkplate.println(title);
}

bool Gui::isCurrentEvent(const char *startTimeStr, const char *endTimeStr) {
  struct tm timeInfo;
  if (!getLocalTimeAdjusted(&timeInfo))
    return false;
  time_t now = tmFieldsToEpoch(&timeInfo);

  struct tm startTm, endTm;
  if (!parseIsoDateTime(startTimeStr, &startTm) ||
      !parseIsoDateTime(endTimeStr, &endTm))
    return false;

  time_t start = tmFieldsToEpoch(&startTm);
  time_t end = tmFieldsToEpoch(&endTm);

  return (now >= start && now <= end);
}

void Gui::showCalendar(calendarData *calendar) {
  inkplate.clearDisplay();

  // === Top Section (Black Header Box) ===
  // Full display width (825), taller than the Inkplate 6 port's 125px bar
  // since the panel has 400 extra vertical pixels to spend.
  inkplate.fillRect(0, 0, 825, 170, 0); // black box

  // Get current time
  struct tm timeInfo;
  if (!getLocalTimeAdjusted(&timeInfo)) {
    showError("Time not available");
    return;
  }

  inkplate.setFont(&FreeSansBold48pt7b);
  // === Big Date Number (white) ===
  inkplate.setTextColor(7);
  inkplate.setCursor(25, 103);
  inkplate.println(timeInfo.tm_mday);

  inkplate.setFont(&FreeSansBold24pt7b);

  // === Day of the Week (white) ===
  inkplate.setCursor(150, 70);
  inkplate.println(getDayName(timeInfo.tm_wday));

  // === Month + Year (white) ===
  char monthYear[32];
  snprintf(monthYear, sizeof(monthYear), "%s %d",
           getMonthName(timeInfo.tm_mon), 1900 + timeInfo.tm_year);
  inkplate.setCursor(150, 120);
  inkplate.println(monthYear);

  // === Last Updated Section (Top Right) ===
  inkplate.setFont(&FreeSans12pt7b);
  inkplate.setCursor(575, 50);
  inkplate.println("Last Updated:");

  char timeString[6]; // HH:MM
  snprintf(timeString, sizeof(timeString), "%02d:%02d", timeInfo.tm_hour,
           timeInfo.tm_min);

  inkplate.setCursor(725, 50);
  inkplate.println(timeString);

  // === Calendar Events ===
  Event *events = calendar->getEvents();
  int eventCount = calendar->getEventCount();
  int y = 200;
  int x = 150;

  char lastDate[8] = "";
  int counter = 0;

  for (int i = 0; i < eventCount; i++) {
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(0); // black text again

    char eventDate[8];
    formatDate(events[i].startTime, eventDate, sizeof(eventDate));

    // Draw section header if date changes
    if (strcmp(eventDate, lastDate) != 0) {
      y += 50;

      // Get day of week from the day-of-month digits (assumes the event
      // falls within the current month, matching the original sketch's
      // fallback-to-current-month/year behavior).
      int weekday = computeWeekday(1900 + timeInfo.tm_year,
                                   timeInfo.tm_mon + 1, atoi(eventDate));

      // Date (big, bold)
      inkplate.setFont(&FreeSans18pt7b);
      inkplate.setTextColor(0);
      inkplate.setCursor(25, y);
      inkplate.println(eventDate);

      // Short Day (under date)
      inkplate.setFont(&FreeSans12pt7b);
      inkplate.setCursor(25, y + 30);
      inkplate.println(getShortDayName(weekday));

      snprintf(lastDate, sizeof(lastDate), "%s", eventDate);
    }

    inkplate.setFont(&FreeSans18pt7b);
    int yLineStart = y;
    int xTime = 700;

    // Highlight if it's happening now
    bool isNow = isCurrentEvent(events[i].startTime, events[i].endTime);
    if (isNow) {
      inkplate.fillRoundRect(x - 10, y - 35, 660, 70, 10, 5); // Draw highlight
    }

    // Draw event summary and time
    inkplate.setCursor(x, y);
    char summary[EVENT_SUMMARY_LEN + 4];
    if (strlen(events[i].summary) > MAX_SUMMARY_LENGTH) {
      snprintf(summary, sizeof(summary), "%.*s...", MAX_SUMMARY_LENGTH,
               events[i].summary);
    } else {
      snprintf(summary, sizeof(summary), "%s", events[i].summary);
    }
    inkplate.println(summary);

    char startHour[8], endHour[8];
    formatHour(events[i].startTime, startHour, sizeof(startHour));
    formatHour(events[i].endTime, endHour, sizeof(endHour));

    inkplate.setCursor(xTime, y);
    inkplate.println(startHour);
    inkplate.setTextColor(2);
    y += 25;
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setCursor(xTime + 25, y);
    inkplate.println(endHour);
    y += 50;

    // margin drawing
    inkplate.drawLine(100, yLineStart - 43, 100, y - 43, 0);
    inkplate.drawLine(101, yLineStart - 43, 101, y - 43, 0);
    inkplate.drawLine(102, yLineStart - 43, 102, y - 43, 0);

    counter = i;

    if (y >= 1125) // Stop drawing if out of vertical space
      break;
  }

  // Show end message
  if (counter == eventCount - 1 && y < 1175) {
    inkplate.setTextColor(2);
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setCursor(225, y + 25);
    inkplate.println("No more events in the next 2 weeks!");
  }

  inkplate.display();
}

// Shows an error message on the display
void Gui::showError(const char *message) {
  inkplate.clearDisplay();
  inkplate.setTextSize(2);
  inkplate.setTextColor(0);
  inkplate.setCursor(10, 10);
  inkplate.println("Error:");
  inkplate.println(message);
  inkplate.display();
}
