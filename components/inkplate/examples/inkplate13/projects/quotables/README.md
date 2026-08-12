# Quotables

Fetch a random quote from a public API and display it on Soldered Inkplate 13SPECTRA, refreshing periodically via deep sleep.

## Overview

Connects Inkplate 13SPECTRA to WiFi, performs an HTTP GET against the [Quotable](https://github.com/lukePeavey/quotable) public API (no API key required), and parses the JSON response with cJSON to extract the quote text and author. The quote is rendered inside a word-wrapped text box using a monospace font, and the author name is printed in the bottom-right corner.

After updating the display, the board enters deep sleep and wakes every `REFRESH_INTERVAL_US` (default: 5 minutes) to fetch and show a new quote. Because deep sleep resets the ESP32, all logic lives in `app_main()`, which runs again from scratch on every wake.

If the WiFi connection fails, an error message is shown and the device sleeps briefly before automatically retrying.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable WiFi (2.4 GHz) connection with Internet access

## Setup

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13** (used for both Inkplate 13 and Inkplate 13SPECTRA)
- **WiFi Configuration → Enter your SSID and password**

No API key or other credentials are required for this example.

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: the fetched quote inside a text box using a monospace font (FreeMonoBold12pt7b) in black on white; the author printed in the bottom-right corner, prefixed with "-".
- Serial Monitor: WiFi connection status and quote-fetch logs.
- On WiFi failure: an "Unable to connect to WiFi!" message is shown, then the board sleeps briefly and retries automatically.
- Every `REFRESH_INTERVAL_US` (default 5 minutes), the board wakes, fetches a new quote, and updates the display.

## Notes

- Display mode: Inkplate 13SPECTRA is a 6-color e-paper board (black, white, yellow, red, blue, green via the `INKPLATE_*` color macros - see `Inkplate13.h`). There is no `setDisplayMode()` call on this board - it always renders in its native color mode, and there is no `INKPLATE_ORANGE` on this board, unlike Inkplate 6Color. This example only draws in black on the (white) background, matching the original sketch.
- Orientation: the board defaults to rotation 3 (landscape) right after construction (`Inkplate::Inkplate()` calls `setRotation(3)` for `CONFIG_INKPLATE_BOARD_INKPLATE13`). The original `Inkplate13SPECTRA_Quotables.ino` doesn't call `setRotation()` either, so this port doesn't add one - the quote box and author layout constants in `main.cpp` are ported as-is from that sketch, which was already tuned for this board's resulting 1600x1200 landscape `width()`/`height()` (unlike some other quotables ports on smaller/monochrome boards, no rescaling was needed here).
- Deep sleep resets the ESP32 on every wake - there is no `loop()`; all logic runs once per boot inside `app_main()`.
- `drawTextBox()` word-wraps automatically and truncates with "..." if the quote exceeds the box height.
- The API is public and requires no key; its response format may change over time - update `QuotablesNetwork.cpp` if parsing starts failing.
- Update period is configured via `REFRESH_INTERVAL_US`; WiFi retry period via `WIFI_RETRY_INTERVAL_US` (both in microseconds).
- `main/fonts/` also includes `exmouth_32pt7b.h` and `FreeMonoBold24pt7b.h`, copied unchanged from the original sketch's `Fonts/` directory for parity, but only `FreeMonoBold12pt7b.h` is used by this example.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
