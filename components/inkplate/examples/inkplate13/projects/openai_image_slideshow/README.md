# OpenAI Image Slideshow

Generate an image from a text prompt using OpenAI's image generation API (DALL-E), download it, and display it on Soldered Inkplate 13SPECTRA. Repeats periodically via RTC-scheduled deep sleep.

## Overview

Connects Inkplate 13SPECTRA to WiFi, sends a JSON request to OpenAI's `/v1/images/generations` endpoint with a text prompt, parses the JSON response to extract the generated image's URL, then downloads and renders that image on the 6-color e-paper display.

The HTTPS POST request is built with `esp_http_client` using the low-level `esp_http_client_open` -> `esp_http_client_write` -> `esp_http_client_fetch_headers` -> `esp_http_client_read` sequence, with a JSON body (`{"prompt": ..., "n": 1, "size": "1024x1024"}`) and an `Authorization: Bearer <key>` header. The request body is built with cJSON, and the response is parsed with cJSON to extract `data[0].url`. Once the URL is known, `display.image.draw()` downloads and renders it.

Status messages ("Connecting...", "Generating image...", etc.) and the final downloaded image are all shown with full refreshes (`display.display()`). The board then schedules its next wake-up using the on-board RTC alarm and enters deep sleep, so the whole flow repeats from `app_main()` on every wake-up.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable (battery optional)
- Stable WiFi connection (2.4 GHz), internet access
- An OpenAI API key

## Setup

### 1. Set your OpenAI API key and prompt

In `main/main.cpp`, fill in:

```cpp
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"
#define IMAGE_PROMPT "Generate an image at 1024x1024 resolution with a lot of flowers " \
                      "including blue, red, yellow, orange and green colors. They should " \
                      "be on a green hill with a clear blue sky in the background."
```

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13**
- **WiFi Configuration → Enter your SSID and password**

### 3. Adjust the sleep interval (optional)

`SLEEP_DURATION_SECONDS` in `main/main.cpp` controls how long the board sleeps between generated images (default: 30 minutes).

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- During startup: short status messages on the display, each pushed with a full refresh.
- After generation: the downloaded image rendered on the e-paper display, dithered down to the panel's 6-color palette (black, white, yellow, red, blue, green). Note that Inkplate 13SPECTRA does not have orange in its palette, unlike Inkplate 6Color's 7-color palette.
- Serial output includes the OpenAI HTTP status/response and the resolved image URL.
- The board then deep-sleeps and wakes up periodically to generate the next image.

## Notes

- Color image path: Inkplate 13SPECTRA uses `ImageColor` instead of the plain grayscale `Image` class, so `display.image.draw()` dithers/quantizes the downloaded image down to the panel's 6 colors rather than grayscale shades. The `draw()` signature is the same shape as on other boards: `draw(url, x, y, dither, invert)`. `ImageColor` already knows the correct 6-color RGB palette for this board internally (see `components/inkplate/src/graphics/ImageColor.cpp`, which branches on `CONFIG_INKPLATE_BOARD_INKPLATE13`), so no palette handling is required in this example.
- Image placement: the original Inkplate13SPECTRA Arduino sketch draws the downloaded (1024x1024) image at `(0, 0)` — top-left aligned on the 1200x1600 panel, not centered — despite its code comment claiming otherwise. This example matches that real behavior rather than centering the image, unlike the Inkplate 6Color port (which centers using a 512 px half-width/half-height offset).
- Request body: the original Inkplate13SPECTRA sketch's OpenAI request only sets `prompt`, `n`, and `size` (no `model`/`style` fields), so it targets OpenAI's default image model rather than explicitly requesting `dall-e-3` with `vivid` style like the Inkplate 6Color sketch does. This example mirrors that.
- Display mode: every status update and the final image use a full `display.display()` refresh, matching the original Arduino sketch's behavior for this board.
- Deep sleep restarts the ESP32 on every wake-up; no state is preserved.
- `api.openai.com` is signed by a well-known public CA, so this example verifies the server certificate using the ESP-IDF certificate bundle (`esp_crt_bundle_attach`) rather than disabling TLS verification like the original Arduino sketch (`client.setInsecure()`) did. This requires `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` (already set in `sdkconfig.defaults`) and `REQUIRES "mbedtls"` in `main/CMakeLists.txt`.
- `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` are kept enabled in `sdkconfig.defaults` for parity with the other ported examples, but are not actually relied on here since `esp_http_client_config_t.crt_bundle_attach` is set and the connection is verified against the CA bundle.
- `display.image.draw()` auto-detects the image format from the downloaded data, so no explicit image format is passed (unlike the Arduino `ImageColor::PNG` argument in the original sketch).
- RAM and bandwidth: downloading/decoding large images can be slow and memory intensive. If decoding fails, reduce the requested image size.
- Wake-up is configured via RTC alarm epoch and an external wake on GPIO 18 (tied to the RTC interrupt line on Inkplate 13SPECTRA), matching the other Inkplate 13 deep-sleep/RTC examples in this component.
- WiFi connection uses a bounded timeout (`waitForConnect`) instead of the original's infinite retry loop. On failure, the device still schedules the RTC alarm and deep-sleeps, so it automatically retries on the next wake-up cycle.
- Protect your API key — do not commit a real key to a public repository. OpenAI API usage and quotas apply.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
