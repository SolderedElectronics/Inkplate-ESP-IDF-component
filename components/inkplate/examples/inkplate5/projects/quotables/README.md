# Quotables

Fetch a random quote from a public API and display it on Soldered Inkplate 5, refreshing periodically via deep sleep.

## Overview

Connects Inkplate 5 to WiFi, performs an HTTP GET against the [Quotable](https://github.com/lukePeavey/quotable) public API (no API key required), and parses the JSON response with cJSON to extract the quote text and author. The quote is rendered inside a word-wrapped text box using a large monospace font, and the author name is printed in the bottom-right corner.

After updating the display, the board enters deep sleep and wakes every `REFRESH_INTERVAL_US` (default: 5 minutes) to fetch and show a new quote. Because deep sleep resets the ESP32, all logic lives in `app_main()`, which runs again from scratch on every wake.

If the WiFi connection fails, an error message is shown and the device sleeps briefly before automatically retrying.

## Hardware Required

- Soldered Inkplate 5
- USB cable (battery optional)
- Stable WiFi connection with Internet access

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate5**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: the fetched quote centered in a text box using a large monospace font (FreeMonoBold24pt7b); the author printed in the bottom-right corner, prefixed with "-".
- Serial Monitor: WiFi connection status and quote-fetch logs.
- On WiFi failure: an "Unable to connect to WiFi!" message is shown, then the board sleeps briefly and retries automatically.
- Every `REFRESH_INTERVAL_US` (default 5 minutes), the board wakes, fetches a new quote, and updates the display.

## Notes

- Display mode is 1-bit (BW); this example uses a full refresh (`display.display()`).
- Deep sleep resets the ESP32 on every wake — there is no `loop()`; all logic runs once per boot inside `app_main()`.
- `drawTextBox()` word-wraps automatically and truncates with "..." if the quote exceeds the box height.
- The API is public and requires no key; its response format may change over time — update `QuotablesNetwork.cpp` if parsing starts failing.
- Update period is configured via `REFRESH_INTERVAL_US`; WiFi retry period via `WIFI_RETRY_INTERVAL_US` (both in microseconds).

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
