#include "NetworkFunctions.h"

#include <time.h>

void NetworkFunctions::begin() {
  // Nothing to do here. The wall clock is synced once via
  // display.wifi.setCurrentTime() (SNTP) in main.cpp; every city's local
  // time is then derived offline from that shared UTC epoch. See the
  // header comment for why this replaces the original timeapi.io lookup.
}

bool NetworkFunctions::getData(int utcOffsetMinutes, int *hours,
                                int *minutes) {
  if (!hours || !minutes) {
    return false;
  }

  // time() always returns seconds since the epoch in UTC, regardless of the
  // process' TZ setting, so this is safe to call for every city even though
  // display.wifi.setCurrentTime() also sets a (single, fixed) local TZ.
  time_t nowSecs = time(nullptr) + (time_t)utcOffsetMinutes * 60;

  struct tm timeInfo;
  gmtime_r(&nowSecs, &timeInfo);

  *hours = timeInfo.tm_hour;
  *minutes = timeInfo.tm_min;

  return true;
}
