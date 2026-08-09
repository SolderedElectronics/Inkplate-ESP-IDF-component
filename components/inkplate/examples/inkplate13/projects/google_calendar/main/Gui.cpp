/**
 * @file        Gui.cpp
 * @brief       Draws the day/agenda calendar view on Inkplate 13SPECTRA.
 */
#include "Gui.h"
#include "includes.h" // getLocalTimeAdjusted()

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

// Fonts
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

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

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate), highlightColor(INKPLATE_RED) {}

void Gui::setHighlightColor(int color) { highlightColor = color; }

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
  inkplate.setTextColor(INKPLATE_BLACK);
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
  inkplate.setTextColor(INKPLATE_BLACK);
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
  // The board defaults to rotation 3 (landscape) right after construction
  // (see Inkplate::Inkplate() for CONFIG_INKPLATE_BOARD_INKPLATE13). The
  // original sketch's Gui.cpp explicitly switches to rotation 0 here so the
  // 1200x1600 portrait coordinates below line up with width()/height().
  inkplate.setRotation(0);
  inkplate.drawLine(0, 10, 1600, 10, INKPLATE_BLACK);

  // === Top Section (Black Header Box) ===
  inkplate.fillRect(0, 0, 1200, 95, INKPLATE_BLACK); // black box

  // Get current time
  struct tm timeInfo;
  if (!getLocalTimeAdjusted(&timeInfo)) {
    showError("Time not available");
    return;
  }

  inkplate.setFont(&FreeSansBold24pt7b);
  // === Big Date Number (white) ===
  inkplate.setTextColor(INKPLATE_WHITE);
  inkplate.setCursor(10, 50);
  inkplate.println(timeInfo.tm_mday);

  inkplate.setFont(&FreeSans12pt7b);

  // === Day of the Week (white) ===
  inkplate.setCursor(75, 35);
  inkplate.println(getDayName(timeInfo.tm_wday));

  // === Month + Year (white) ===
  char monthYear[32];
  snprintf(monthYear, sizeof(monthYear), "%s %d",
           getMonthName(timeInfo.tm_mon), 1900 + timeInfo.tm_year);
  inkplate.setCursor(75, 50);
  inkplate.println(monthYear);

  // === Last Updated Section (Top Right) ===
  inkplate.setFont(&FreeSans9pt7b);
  inkplate.setCursor(1030, 25);
  inkplate.println("Last Updated:");

  char timeString[6]; // HH:MM
  snprintf(timeString, sizeof(timeString), "%02d:%02d", timeInfo.tm_hour,
           timeInfo.tm_min);

  inkplate.setCursor(1140, 25);
  inkplate.println(timeString);

  // === Calendar Events ===
  Event *events = calendar->getEvents();
  int eventCount = calendar->getEventCount();
  int y = 105;
  int x = 100;

  char lastDate[8] = "";
  int counter = 0;

  for (int i = 0; i < eventCount; i++) {
    inkplate.setFont(&FreeSans18pt7b);
    inkplate.setTextColor(INKPLATE_BLACK); // black text again

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
      inkplate.setFont(&FreeSans12pt7b);
      inkplate.setTextColor(INKPLATE_BLACK);
      inkplate.setCursor(15, y);
      inkplate.println(eventDate);

      // Short Day (under date)
      inkplate.setFont(&FreeSans9pt7b);
      inkplate.setCursor(15, y + 30);
      inkplate.println(getShortDayName(weekday));

      snprintf(lastDate, sizeof(lastDate), "%s", eventDate);
    }

    inkplate.setFont(&FreeSans12pt7b);
    int yLineStart = y;
    int xTime = 1100;

    // Highlight if it's happening now
    bool isNow = isCurrentEvent(events[i].startTime, events[i].endTime);
    if (isNow) {
      inkplate.fillRoundRect(x - 10, y - 30, 1080, 55, 10, highlightColor); // Draw highlight
      inkplate.setTextColor(INKPLATE_WHITE);
    } else {
      inkplate.setTextColor(INKPLATE_BLACK);
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
    y += 15;
    inkplate.setFont(&FreeSans9pt7b);
    inkplate.setCursor(xTime + 13, y);
    inkplate.println(endHour);
    y += 50;

    // margin drawing
    int xMarg = 70;
    int yMarg = 30;
    int yMargGap = yMarg + 10;
    int margColor = INKPLATE_BLACK;

    inkplate.drawLine(xMarg, yLineStart - yMarg, 70, y - yMargGap, margColor);
    inkplate.drawLine(xMarg + 1, yLineStart - yMarg, xMarg + 1, y - yMargGap, margColor);
    inkplate.drawLine(xMarg + 2, yLineStart - yMarg, xMarg + 2, y - yMargGap, margColor);

    counter = i;

    if (y >= inkplate.height() - 20) // Stop drawing if out of vertical space
      break;
  }

  // Show end message
  if (counter == eventCount - 1 && y < 1575) {
    // Original sketch used color index 3, which on the 6-color Inkplate
    // 13SPECTRA palette (0=black,1=white,2=yellow,3=red,5=blue,6=green) is
    // INKPLATE_RED.
    inkplate.setTextColor(INKPLATE_RED);
    inkplate.setFont(&FreeSans12pt7b);
    inkplate.setCursor(390, y + 15);
    inkplate.println("No more events in the next 2 weeks!");
  }

  inkplate.display();
}

// Shows an error message on the display
void Gui::showError(const char *message) {
  inkplate.clearDisplay();
  inkplate.setTextSize(2);
  inkplate.setTextColor(INKPLATE_BLACK);
  inkplate.setCursor(10, 10);
  inkplate.println("Error:");
  inkplate.println(message);
  inkplate.display();
}
