# World Clock

Display analog clock faces for multiple timezones simultaneously on Soldered Inkplate 2.

## Overview

Connects Inkplate 2 to WiFi once at boot to sync the wall clock via SNTP (`display.wifi.setCurrentTime()`), then draws one analog clock face per configured city, each labeled with its city name and an AM/PM indicator. Every city's local time is derived offline from the shared UTC epoch using a fixed UTC offset, so no network connection is required after the initial time sync. The clocks are redrawn once a minute so the displayed time stays current.

The original Arduino sketch queried a remote REST service (timeapi.io) to resolve each city to a full IANA timezone and fetch its current local time. This port replaces that network round-trip with local offset math (the same approach used by this component's `advanced/rtc/alarm` example), avoiding an extra HTTPS + JSON dependency while producing the same result.

## Hardware Required

- Soldered Inkplate 2
- USB cable
- WiFi connection with Internet access (only needed at boot, for NTP)

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate2**
- **WiFi Configuration → Enter your SSID and password**

### Choosing which cities/timezones are shown

Edit the `kCities` array near the top of `main/main.cpp`:

```cpp
static const CityInfo kCities[] = {
    {"Zagreb", 2 * 60},  // CEST, UTC+2 (Croatia, observes DST)
    {"Lima", -5 * 60},   // PET, UTC-5 (Peru, no DST)
};
```

Each entry is a display label and a fixed UTC offset in minutes (negative offsets are supported, as are half-hour/quarter-hour zones). The layout in `drawTime()`/`app_main()` places clocks side by side and is sized for two entries; add more rows/columns yourself if you want to display more cities at once. Because the offsets are fixed, remember to update them by hand for cities that observe daylight saving time.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- One analog clock face per configured city, each showing hour/minute hands, an AM/PM indicator, and the city's label underneath.
- The display refreshes once a minute so all clocks stay in sync with real time.
- Serial Monitor logs WiFi connection status and each city's computed time.

## Notes

- `display.clearDisplay()` clears only the internal framebuffer.
- `display.display()` must be called to push changes to the physical panel; this example does a full refresh every minute.
- UTC offsets are fixed (no automatic DST) - update `kCities` if a tracked city changes clocks.
- If WiFi fails to connect at boot, an error message is shown on the display and the example stops (no local time can be computed without an initial NTP sync).
- This example uses color (black/white/red) display mode.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
