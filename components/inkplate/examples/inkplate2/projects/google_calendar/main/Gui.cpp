/**
 * @file        Gui.cpp
 * @brief       Renders calendar events / errors on the Inkplate 2 e-paper
 *              display.
 */

#include "Gui.h"

#include "calendarData.h"

#include <cstdlib>
#include <ctime>

// font
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans7pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate) {}

std::string Gui::getDayName(int dayIndex) {
  const char *days[] = {"Sunday",   "Monday", "Tuesday",  "Wednesday",
                        "Thursday", "Friday", "Saturday"};
  return days[dayIndex];
}

std::string Gui::getMonthName(int monthIndex) {
  const char *months[] = {"January", "February", "March",     "April",
                          "May",     "June",     "July",      "August",
                          "September", "October", "November", "December"};
  return months[monthIndex];
}

std::string Gui::formatHour(const char *isoDateTime) {
  std::string s(isoDateTime);
  if (s.length() < 16)
    return "";
  return s.substr(11, 5); // "HH:MM" from ISO 8601
}

std::string Gui::formatDate(const char *isoDateTime) {
  std::string s(isoDateTime);
  if (s.length() < 10)
    return "";
  return s.substr(8, 2); // "DD" from "YYYY-MM-DD..."
}

void Gui::wifiError() {
  inkplate.clearDisplay();
  inkplate.setTextColor(INKPLATE2_BLACK);
  inkplate.setFont(&FreeSans7pt7b);
  inkplate.setCursor(10, 20);
  inkplate.print("WiFi connection failed.");
  inkplate.setCursor(10, 50);
  inkplate.print("Check credentials or try again.");
  inkplate.display();
}

std::string Gui::getShortDayName(int dayIndex) {
  const char *shortDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  return shortDays[dayIndex];
}

void Gui::showCalendar(calendarData *calendar, const struct tm &nowLocal) {
  inkplate.clearDisplay();

  // Note: the original checked getLocalTime() here and bailed out to
  // showError() if the clock wasn't set yet. That check is dropped in this
  // port because app_main() already blocks on display.wifi.setCurrentTime()
  // before ever calling showCalendar(), so `nowLocal` is always valid.

  // === Calendar Events ===
  Event *events = calendar->getEvents();
  int eventCount = calendar->getEventCount();
  int y = 20;
  int x = 40;

  std::string lastDate = "";

  int counter = 0;

  for (int i = 0; i < eventCount; i++) {
    inkplate.setTextColor(INKPLATE2_BLACK); // black text again

    std::string eventDate = formatDate(events[i].startTime);

    // Draw section header if date changes
    if (eventDate != lastDate) {
      if (i != 0) {
        y += 20;
        if (y >= 85) {
          break;
        }
      }

      // Get day of week from date string (assumes format "YYYY-MM-DD").
      // mktime() normalizes tm_wday using the process's local TZ, same as
      // the original sketch relied on; only the day-of-week matters here.
      struct tm timeStruct = {};
      timeStruct.tm_year = nowLocal.tm_year; // use current year as fallback
      timeStruct.tm_mon = nowLocal.tm_mon;   // use current month as fallback
      timeStruct.tm_mday = atoi(eventDate.c_str()); // parse day from string
      mktime(&timeStruct); // normalize to fill in wday

      // Date (big, bold)
      inkplate.setFont(&FreeSans9pt7b);
      inkplate.setTextColor(INKPLATE2_BLACK);
      inkplate.setCursor(5, y + 5);
      inkplate.println(eventDate.c_str());

      // Short Day (under date)
      inkplate.setFont(&FreeSans7pt7b);
      inkplate.setCursor(5, y + 18);
      inkplate.println(getShortDayName(timeStruct.tm_wday).c_str());

      lastDate = eventDate;
    }

    inkplate.setFont(&FreeSans7pt7b);
    int yLineStart = y;
    int xTime = 170;

    // Draw event summary and time
    inkplate.setCursor(x, y);
    std::string summary = events[i].summary;
    if (summary.length() > MAX_SUMMARY_LENGTH) {
      summary = summary.substr(0, MAX_SUMMARY_LENGTH) + "...";
    }
    inkplate.println(summary.c_str());
    inkplate.setCursor(xTime, y);
    inkplate.setFont(&FreeSans7pt7b);
    inkplate.println(formatHour(events[i].startTime).c_str());
    inkplate.setTextColor(INKPLATE2_RED);
    y += 12;
    inkplate.setFont(&FreeSans7pt7b);
    inkplate.setCursor(xTime, y);
    inkplate.println(formatHour(events[i].endTime).c_str());
    y += 16;

    inkplate.drawLine(33, yLineStart - 12, 33, y - 12, 1);

    counter = i;

    if (y >= 85) // Stop drawing if out of vertical space
    {
      break;
    }
  }

  // Show end message
  if (counter == eventCount - 1 && y < 90) {
    inkplate.setTextColor(INKPLATE2_BLACK);
    inkplate.setFont(&FreeSans7pt7b);
    inkplate.setCursor(10, y + 5);
    inkplate.println("No more events in the next 2 weeks!");
  }

  inkplate.display();
}

// Shows an error message on the display
void Gui::showError(const char *message) {
  inkplate.clearDisplay();
  inkplate.setFont(&FreeSans7pt7b);
  inkplate.setTextColor(INKPLATE2_BLACK);
  inkplate.setCursor(10, 10);
  inkplate.println("Error:");
  inkplate.println(message);
  inkplate.display();
}
