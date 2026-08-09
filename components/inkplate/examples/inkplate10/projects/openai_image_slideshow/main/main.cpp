/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       OpenAI image generation slideshow for Soldered Inkplate 10.
 *
 * @details     Connects Inkplate 10 to WiFi, sends a text prompt to OpenAI's
 *              image generation API (DALL-E), parses the JSON response to
 *              extract the generated image's URL, downloads that image, and
 *              renders it on the e-paper display. The board then schedules
 *              its next wake-up using the on-board RTC alarm and enters deep
 *              sleep, so the whole flow repeats periodically from
 *              app_main() on every wake-up.
 *
 *              Status messages ("Connecting...", "Generating image...", etc.)
 *              are shown using partial updates in black & white mode for
 *              speed. Once the image URL is obtained, the display switches to
 *              grayscale mode for better image quality and performs a full
 *              refresh to draw the downloaded image.
 *
 *              The HTTPS POST request (JSON body + Authorization header) is
 *              built with esp_http_client using the low-level
 *              open/write/fetch_headers/read sequence. The request body is
 *              built with cJSON, and the response is parsed with cJSON to
 *              pull out data[0].url.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable (battery optional)
 * - Extra:      WiFi (2.4 GHz) connection + Internet access, OpenAI API key
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate10
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - OPENAI_API_KEY and IMAGE_PROMPT below -> fill in your own values
 *
 * How to use:
 * 1) Set OPENAI_API_KEY to a valid OpenAI API key.
 * 2) Set IMAGE_PROMPT to whatever image you want OpenAI to generate.
 * 3) Configure WiFi credentials in menuconfig.
 * 4) Build and flash to Inkplate 10, then open the Serial Monitor.
 * 5) The device connects, requests an image, downloads it, renders it, then
 *    deep-sleeps and wakes periodically to generate the next image.
 *
 * Expected output:
 * - During startup: short status messages on the display via partial
 *   updates.
 * - After generation: the downloaded image rendered on the e-paper display
 *   in grayscale.
 * - Serial output includes the OpenAI HTTP status/response and the resolved
 *   image URL.
 *
 * Notes:
 * - Display mode: status is shown in black & white; the image is rendered in
 *   grayscale. Partial updates are not supported in grayscale mode, so the
 *   image update is a full refresh.
 * - Deep sleep restarts the ESP32 on every wake-up; no state is preserved.
 * - api.openai.com is signed by a well-known public CA, so this example
 *   verifies the server certificate using the ESP-IDF certificate bundle
 *   (esp_crt_bundle_attach) rather than disabling TLS verification like the
 *   original Arduino sketch (client.setInsecure()) did. This requires
 *   CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y (set in sdkconfig.defaults) and
 *   REQUIRES "mbedtls" in main/CMakeLists.txt.
 * - CONFIG_ESP_TLS_INSECURE / CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY are kept
 *   enabled in sdkconfig.defaults for parity with the other ported examples,
 *   but are not relied on here since esp_http_client_config_t.crt_bundle_attach
 *   is set and the connection is verified against the CA bundle.
 * - display.image.draw() auto-detects the image format from the downloaded
 *   data, so no explicit Image::Format is passed (unlike the Arduino
 *   Image::PNG argument in the original sketch).
 * - The requested image size is 1024x1024, but it is centered using a
 *   512 px half-width/half-height offset, matching the original sketch. On
 *   Inkplate 10's 1200x825 panel this fits horizontally (with margin) but
 *   still clips the top/bottom of the image since the panel is only 825 px
 *   tall; this quirk is carried over from the original rather than "fixed"
 *   here.
 * - WiFi connection uses a bounded timeout (waitForConnect) instead of the
 *   original's infinite retry loop. On failure, the device still schedules
 *   the RTC alarm and deep-sleeps, so it will automatically retry on the
 *   next wake-up cycle.
 * - RAM and bandwidth: downloading/decoding large PNGs can be slow and
 *   memory intensive. If decoding fails, reduce image size or use a simpler
 *   format.
 * - RTC alarm vs. wake source: wake-up is configured via RTC alarm epoch and
 *   an external wake on GPIO 39 (tied to the RTC interrupt line on Inkplate
 *   10).
 * - Protect your API key: do not commit a real key to a public repository.
 *   OpenAI API usage and quotas apply.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "OPENAI_IMAGE";

// TODO: fill in your OpenAI API key (https://platform.openai.com/api-keys)
#define OPENAI_API_KEY "YOUR_OPENAI_API_KEY"

// TODO: fill in the image prompt you want to send to OpenAI
#define IMAGE_PROMPT "Generate a cyberpunk city with a lot of vertical layers"

#define OPENAI_IMAGE_MODEL "dall-e-3"
#define OPENAI_IMAGE_STYLE "vivid"
#define OPENAI_IMAGE_SIZE "1024x1024"
#define OPENAI_IMAGE_URL "https://api.openai.com/v1/images/generations"
#define HTTP_TIMEOUT_MS 40000
#define OPENAI_RESPONSE_BUFFER_SIZE 4096
#define IMAGE_URL_MAX_LEN 2048

// Time in seconds the device will sleep between updates (30 minutes).
#define SLEEP_DURATION_SECONDS (30 * 60)

/**
 * Send a prompt to OpenAI's image generation API and extract the URL of the
 * generated image.
 *
 * Builds the JSON request body with cJSON, POSTs it to OPENAI_IMAGE_URL with
 * an Authorization: Bearer header using the esp_http_client
 * open/write/fetch_headers/read sequence, then parses the JSON response with
 * cJSON to pull out data[0].url.
 *
 * @param prompt Prompt text describing the desired image.
 * @param outUrl Buffer that receives the image URL on success.
 * @param outUrlSize Size of outUrl in bytes.
 * @return true on success (outUrl contains the image URL), false on error.
 */
static bool requestOpenAIImage(const char *prompt, char *outUrl,
                               size_t outUrlSize) {
  // Build request body:
  // {"model": "dall-e-3", "prompt": "...", "style": "vivid", "n": 1, "size": "1024x1024"}
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "model", OPENAI_IMAGE_MODEL);
  cJSON_AddStringToObject(root, "prompt", prompt);
  cJSON_AddStringToObject(root, "style", OPENAI_IMAGE_STYLE);
  cJSON_AddNumberToObject(root, "n", 1);
  cJSON_AddStringToObject(root, "size", OPENAI_IMAGE_SIZE);

  char *requestBody = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  if (!requestBody) {
    ESP_LOGE(TAG, "Failed to serialize request JSON");
    return false;
  }
  int requestLen = (int)strlen(requestBody);

  esp_http_client_config_t config = {};
  config.url = OPENAI_IMAGE_URL;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = HTTP_TIMEOUT_MS;
  config.crt_bundle_attach = esp_crt_bundle_attach;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client");
    cJSON_free(requestBody);
    return false;
  }

  char authHeader[160];
  snprintf(authHeader, sizeof(authHeader), "Bearer %s", OPENAI_API_KEY);
  esp_http_client_set_header(client, "Content-Type", "application/json");
  esp_http_client_set_header(client, "Authorization", authHeader);

  bool success = false;
  char *responseBuffer = nullptr;

  esp_err_t err = esp_http_client_open(client, requestLen);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
  } else if (esp_http_client_write(client, requestBody, requestLen) !=
             requestLen) {
    ESP_LOGE(TAG, "Failed to write full request body");
    esp_http_client_close(client);
  } else {
    int64_t contentLength = esp_http_client_fetch_headers(client);
    int statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "OpenAI HTTP status=%d content-length=%lld", statusCode,
             (long long)contentLength);

    responseBuffer = (char *)malloc(OPENAI_RESPONSE_BUFFER_SIZE);
    if (!responseBuffer) {
      ESP_LOGE(TAG, "Failed to allocate response buffer");
      esp_http_client_close(client);
    } else {
      int totalRead = 0;
      int r;
      while (totalRead < OPENAI_RESPONSE_BUFFER_SIZE - 1 &&
             (r = esp_http_client_read(client, responseBuffer + totalRead,
                                        OPENAI_RESPONSE_BUFFER_SIZE - 1 -
                                            totalRead)) > 0) {
        totalRead += r;
      }
      responseBuffer[totalRead] = '\0';
      esp_http_client_close(client);

      if (statusCode != 200) {
        ESP_LOGE(TAG, "OpenAI request failed (status %d): %s", statusCode,
                 responseBuffer);
      }

      cJSON *responseJson = cJSON_Parse(responseBuffer);
      if (!responseJson) {
        ESP_LOGE(TAG, "Failed to parse OpenAI response JSON");
      } else {
        cJSON *errorObj = cJSON_GetObjectItem(responseJson, "error");
        if (errorObj) {
          cJSON *errMsg = cJSON_GetObjectItem(errorObj, "message");
          ESP_LOGE(TAG, "OpenAI error: %s",
                   (errMsg && cJSON_IsString(errMsg))
                       ? cJSON_GetStringValue(errMsg)
                       : "unknown error");
        } else {
          cJSON *data = cJSON_GetObjectItem(responseJson, "data");
          cJSON *firstImage =
              (data && cJSON_IsArray(data)) ? cJSON_GetArrayItem(data, 0)
                                            : nullptr;
          cJSON *urlObj =
              firstImage ? cJSON_GetObjectItem(firstImage, "url") : nullptr;

          if (urlObj && cJSON_IsString(urlObj)) {
            snprintf(outUrl, outUrlSize, "%s", cJSON_GetStringValue(urlObj));
            success = true;
          } else {
            ESP_LOGE(TAG,
                     "Unexpected OpenAI response format (no image URL)");
          }
        }
        cJSON_Delete(responseJson);
      }
    }
  }

  free(responseBuffer);
  cJSON_free(requestBody);
  esp_http_client_cleanup(client);
  return success;
}

extern "C" void app_main(void) {
  Inkplate display;
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setTextColor(BLACK, WHITE);
  display.setTextSize(3);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connecting to WiFi...");
  display.display();

  bool wifiOk =
      display.wifi.begin() == ESP_OK && display.wifi.waitForConnect(15000);

  bool gotUrl = false;
  char imageUrl[IMAGE_URL_MAX_LEN] = {0};

  if (wifiOk) {
    ESP_LOGI(TAG, "WiFi connected");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Connected! Generating image...");
    display.partialUpdate();

    gotUrl = requestOpenAIImage(IMAGE_PROMPT, imageUrl, sizeof(imageUrl));

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(gotUrl ? "Prompt generated!" : "Failed to get image URL.");
    display.partialUpdate();
  } else {
    ESP_LOGE(TAG, "WiFi connection failed");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi connection failed!");
    display.partialUpdate();
  }

  // Switch to grayscale mode for higher-quality image rendering.
  // WARNING: partial updates are not supported in this mode.
  display.setDisplayMode(GRAYSCALE);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(BLACK);

  if (gotUrl) {
    display.print("Downloading and displaying image (may take a while)...");
    display.display();
    display.clearDisplay();

    ESP_LOGI(TAG, "Image URL: %s", imageUrl);

    // Draw the image centered on the screen.
    // Image assumed to be 512x512; offset to center it (matches original).
    int x = display.width() / 2 - 512;
    int y = display.height() / 2 - 512;
    bool drawn = display.image.draw(imageUrl, x, y, true, false);

    if (!drawn) {
      ESP_LOGE(TAG, "Image decode error");
      display.setCursor(0, 0);
      display.print("Image decode error.");
    }
  } else if (wifiOk) {
    display.print("Failed to get image URL.");
  } else {
    display.print("WiFi connection failed. Will retry next cycle.");
  }
  display.display();

  // Schedule the next wake-up using the on-board RTC, then deep sleep.
  time_t now = 0;
  display.rtc.getTime(&now);
  display.rtc.setAlarmEpoch(now + SLEEP_DURATION_SECONDS);

  // Enable external wakeup on GPIO 39 (tied to the RTC alarm interrupt).
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  // Enter deep sleep mode to conserve power.
  esp_deep_sleep_start();
}
