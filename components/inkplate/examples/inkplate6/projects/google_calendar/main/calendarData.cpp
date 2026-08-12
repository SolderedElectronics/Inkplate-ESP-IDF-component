/**
 * @file        calendarData.cpp
 * @brief       Fixed-capacity storage for events fetched from Google Calendar.
 */
#include "calendarData.h"

#include <cstdio>

calendarData::calendarData() : eventCount(0) {}

void calendarData::clearEvents() { eventCount = 0; }

void calendarData::addEvent(const char *summary, const char *start,
                            const char *end) {
  if (eventCount >= MAX_EVENTS)
    return;

  snprintf(events[eventCount].summary, sizeof(events[eventCount].summary),
           "%s", summary);
  snprintf(events[eventCount].startTime, sizeof(events[eventCount].startTime),
           "%s", start);
  snprintf(events[eventCount].endTime, sizeof(events[eventCount].endTime),
           "%s", end);
  eventCount++;
}

Event *calendarData::getEvents() { return events; }

int calendarData::getEventCount() { return eventCount; }
