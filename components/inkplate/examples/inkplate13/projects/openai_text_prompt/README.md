# OpenAI Text Prompt

Send a text prompt to the OpenAI Chat Completions API over HTTPS and display the response on Soldered Inkplate 13SPECTRA.

## Overview

Connects Inkplate 13SPECTRA to WiFi, sends a fixed text prompt to OpenAI's `/v1/chat/completions` endpoint, and renders the returned text on the e-paper screen inside a word-wrapped text box using a custom font (`FreeMonoBold18pt7b`).

The HTTPS POST request is built with `esp_http_client` using the low-level `esp_http_client_open` -> `esp_http_client_write` -> `esp_http_client_fetch_headers` -> `esp_http_client_read` sequence, with a JSON body (`{"model": ..., "messages": [...]}`) and an `Authorization: Bearer <key>` header. The request body is built with cJSON so the prompt text is escaped correctly, and the response is parsed with cJSON to extract `choices[0].message.content`.

This is a one-shot example: connect, send the prompt, display the response, done. The original Arduino sketch also fetched live weather data from Open-Meteo to auto-generate the prompt and then entered deep sleep to repeat periodically; both are dropped in this port to keep the focus on the OpenAI request/response/display flow. The prompt is a fixed compile-time placeholder (`PROMPT_TEXT`) instead.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable WiFi connection (2.4 GHz), internet access
- An OpenAI API key

## Setup

### 1. Set your OpenAI API key and prompt

In `main/main.cpp`, fill in:

```cpp
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"
#define PROMPT_TEXT "Give me a one sentence fun fact about e-paper displays."
```

### 2. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: the OpenAI text response, word-wrapped inside a text box and rendered with the `FreeMonoBold18pt7b` font.
- On failure: `"WiFi failed!"` or `"OpenAI request failed."` on the display, with details in the Serial Monitor log.

## Notes

- `api.openai.com` is signed by a well-known public CA, so this example verifies the server certificate using the ESP-IDF certificate bundle (`esp_crt_bundle_attach`) instead of pinning a single certificate like the `https_with_certificate` example does. This requires `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` (already set in `sdkconfig.defaults`) and `REQUIRES "mbedtls"` in `main/CMakeLists.txt`.
- `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` are kept enabled in `sdkconfig.defaults` for parity with the other ported examples, but are not actually relied on here since `esp_http_client_config_t.crt_bundle_attach` is set and the connection is verified against the CA bundle.
- Inkplate 13SPECTRA is a 6-color e-paper board (1200x1600): black, white, yellow, red, blue, green. Unlike Inkplate 6Color (7 colors, which adds orange), there is no `INKPLATE_ORANGE` here — this example only ever needs `INKPLATE_BLACK` / `INKPLATE_WHITE` for text, so the missing color has no effect on this port.
- The reply is rendered with `drawTextBox()` at `(50, 60)`-`(800, 900)` with 58px line spacing and a 32pt font size. Those values come directly from the original `Inkplate13SPECTRA_OpenAI_Text_Prompt.ino` sketch, tuned for this board's 1200x1600 canvas — they are not a rescale of the Inkplate 6Color port's box (`(50, 50)`-`(550, 433)`, 38px spacing, 28pt), which is sized for that board's much smaller 600x448 canvas.
- Protect your API key — do not commit a real key to a public repository. OpenAI API usage and quotas apply.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
