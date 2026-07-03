/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       TRMNL BYOS client example for Soldered Inkplate 6 Flick.
 *
 * @details     Connects Inkplate 6 Flick to WiFi, registers with a TRMNL-compatible
 *              BYOS server via /api/setup, then polls /api/display,
 *              draws whatever image the server returns, and deep-sleeps
 *              until the next refresh.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      Stable WiFi connection, Terminus (TRMNL's official BYOS
 *               server) running via Docker
 *
 * ------------------------------------------------
 * Setting up the BYOS server (Terminus) via Docker
 * ------------------------------------------------
 * Terminus is TRMNL's official self-hosted "Bring Your Own Server" (BYOS)
 * implementation.
 *
 * 1) Install Docker:
 *      macOS:   brew install --cask docker
 *               open -a Docker
 *      Linux:   Use your distro's package manager, e.g.:
 *               sudo apt install docker.io docker-compose-plugin   (Debian/Ubuntu)
 *               sudo systemctl start docker
 *               sudo systemctl enable docker
 *      Windows: Install "Docker Desktop" from docker.com, then launch it
 *               (WSL2 backend required/recommended). Run the commands below
 *               from PowerShell, a WSL2 terminal, or Git Bash.
 *
 *      Then on any OS, confirm it's running:
 *      docker info          // confirms Docker is running
 *
 * 2) Quick start (fastest way to try it, NOT for permanent use):
 *      macOS/Linux (bash):
 *        curl https://raw.githubusercontent.com/usetrmnl/terminus/refs/heads/main/scripts/docker/quick.sh | bash
 *
 *    This script is NOT idempotent - do not run it more than once, since
 *    your database credentials will differ each time. Once it finishes,
 *    open http://localhost:2300 in a browser and click "Register" to
 *    create your login.
 *
 *    For permanent/production use instead, clone + set up manually:
 *      git clone https://github.com/usetrmnl/terminus
 *      cd terminus
 *      bin/setup            // idempotent, safe to re-run
 *
 * 3) Find your server's LAN IP (so the Inkplate can reach it):
 *      macOS:   ipconfig getifaddr en0
 *      Linux:   ip addr show   // look for inet under your active interface
 *      Windows: ipconfig       // look for "IPv4 Address"
 *
 *    Make sure this matches the API_URI value Terminus is using (check the
 *    .env file created during setup) - the device and the server must agree
 *    on the exact same host:port.
 *
 * 4) Register your device in the Terminus dashboard:
 *      Devices -> Add Device
 *      - Model: pick the closest match, or create a custom one under
 *        "Models" if your exact Inkplate isn't listed
 *      - MAC Address: your Inkplate's WiFi MAC (printed to the e-paper
 *        display after WiFi connects)
 *      - Refresh Rate: how often (seconds) the device should poll
 *
 * 5) Point this example at your server:
 *      Set BYOS_SERVER below to "http://<server-ip>:2300" (no trailing
 *      slash).
 *
 * 6) Build actual screen content:
 *      Designs   -> create a Liquid/HTML template for what to display
 *      Screens   -> confirm the rendered PNG shows up
 *      Playlists -> add that screen to a playlist
 *      Devices   -> assign the playlist to your device
 *
 * Once all of the above is done, this example's doSetup()/doDisplay() calls
 * fetch and draw whatever screen you've configured.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - Menuconfig -> WiFi Configuration -> Enter your credentials
 * - Set BYOS_SERVER below to your Terminus server's address
 *
 * How to use:
 * 1) Set up Terminus per the instructions above.
 * 2) Set BYOS_SERVER below.
 * 3) Build and flash to Inkplate 6 Flick.
 * 4) The board connects to WiFi, registers with the server, fetches and
 *    displays a screen, then deep-sleeps until the next refresh.
 *
 * Expected output:
 * - Connection status and device ID (WiFi MAC) shown while connecting.
 * - Whatever image the BYOS server assigns to this device, refreshed on the
 *   interval the server specifies.
 *
 * Notes:
 * - Uses BLACK_AND_WHITE display mode so WiFi-connect progress dots can be
 *   shown with fast partial updates.
 * - The whole /api/display JSON response is held in a single PSRAM buffer;
 *   image_url is expected to point directly at a BMP/JPEG/PNG file.
 * - update_firmware is logged only - OTA is not implemented in this example.
 * - JSON is parsed with a small built-in string-search extractor (no
 *   external JSON library dependency).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

#include "Inkplate.h"

// Set to your Terminus server, e.g. "http://192.168.1.50:2300" (no trailing slash)
#define BYOS_SERVER "http://YOUR_SERVER_IP:2300"

static const char *TAG = "TRMNL";

static bool httpGetWithHeader(const char *url, const char *deviceId,
                              char **outBody, int *outLen) {
  esp_http_client_config_t config = {};
  config.url = url;
  config.timeout_ms = 10000;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client)
    return false;

  esp_http_client_set_header(client, "ID", deviceId);

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return false;
  }

  int contentLen = esp_http_client_fetch_headers(client);
  if (contentLen <= 0)
    contentLen = 4096; // fallback if server doesn't send Content-Length

  char *buf = (char *)heap_caps_malloc(contentLen + 1,
                                       MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf) {
    ESP_LOGE(TAG, "Out of memory (%d bytes)", contentLen);
    esp_http_client_cleanup(client);
    return false;
  }

  int totalRead = 0;
  int read;
  while (totalRead < contentLen) {
    read = esp_http_client_read(client, buf + totalRead, contentLen - totalRead);
    if (read <= 0)
      break;
    totalRead += read;
  }
  buf[totalRead] = '\0';

  esp_http_client_cleanup(client);

  *outBody = buf;
  *outLen = totalRead;
  return true;
}

static void goToSleep(long seconds) {
  ESP_LOGI(TAG, "Sleeping for %ld seconds", seconds);
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}

static void doSetup(const char *deviceId) {
  char url[160];
  snprintf(url, sizeof(url), "%s/api/setup", BYOS_SERVER);

  char *body = nullptr;
  int len = 0;
  if (httpGetWithHeader(url, deviceId, &body, &len)) {
    ESP_LOGI(TAG, "Setup response: %s", body);
    free(body);
  } else {
    ESP_LOGE(TAG, "Setup request failed");
  }
}

// Tiny string-search JSON value extractors - the TRMNL /api/display
// response is a small, fixed-shape object, so a full JSON parser is more
// dependency than this needs.
static bool jsonExtractString(const char *json, const char *key, char *out,
                              size_t outSize) {
  char pattern[64];
  snprintf(pattern, sizeof(pattern), "\"%s\"", key);
  const char *p = strstr(json, pattern);
  if (!p)
    return false;
  p = strchr(p + strlen(pattern), ':');
  if (!p)
    return false;
  p++;
  while (*p == ' ' || *p == '\t')
    p++;
  if (*p != '"')
    return false;
  p++;
  size_t i = 0;
  while (*p && *p != '"' && i < outSize - 1)
    out[i++] = *p++;
  out[i] = '\0';
  return true;
}

static bool jsonExtractNumber(const char *json, const char *key, long *out) {
  char pattern[64];
  snprintf(pattern, sizeof(pattern), "\"%s\"", key);
  const char *p = strstr(json, pattern);
  if (!p)
    return false;
  p = strchr(p + strlen(pattern), ':');
  if (!p)
    return false;
  *out = strtol(p + 1, nullptr, 10);
  return true;
}

static bool jsonExtractBool(const char *json, const char *key) {
  char pattern[64];
  snprintf(pattern, sizeof(pattern), "\"%s\"", key);
  const char *p = strstr(json, pattern);
  if (!p)
    return false;
  p = strchr(p + strlen(pattern), ':');
  if (!p)
    return false;
  p++;
  while (*p == ' ' || *p == '\t')
    p++;
  return strncmp(p, "true", 4) == 0;
}

static void doDisplay(Inkplate &display, const char *deviceId) {
  char url[160];
  snprintf(url, sizeof(url), "%s/api/display", BYOS_SERVER);

  char *body = nullptr;
  int len = 0;
  if (!httpGetWithHeader(url, deviceId, &body, &len)) {
    ESP_LOGE(TAG, "Display request failed");
    goToSleep(900);
    return;
  }

  ESP_LOGI(TAG, "Display response: %s", body);

  char imageUrl[256];
  bool hasImageUrl =
      jsonExtractString(body, "image_url", imageUrl, sizeof(imageUrl));
  long refreshRate = 0;
  jsonExtractNumber(body, "refresh_rate", &refreshRate);
  bool updateFirmware = jsonExtractBool(body, "update_firmware");

  free(body);

  if (refreshRate <= 0)
    refreshRate = 900;

  if (hasImageUrl) {
    display.clearDisplay();
    bool ok = display.image.draw(imageUrl, 0, 0, false, false);
    if (!ok) {
      display.setCursor(0, 0);
      display.print("Failed to draw image from URL");
    }
    display.display();
  } else {
    ESP_LOGE(TAG, "No image_url in response");
  }

  if (updateFirmware) {
    ESP_LOGW(TAG, "Firmware update flagged - not implemented, skipping.");
  }

  goToSleep(refreshRate);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.setTextColor(BLACK, WHITE);
  display.print("Connecting to WiFi...");
  display.display();

  display.wifi.begin();

  int wifiAttempts = 0;
  while (!display.wifi.isConnected() && wifiAttempts < 20) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    display.print(".");
    display.partialUpdate();
    wifiAttempts++;
  }

  if (!display.wifi.isConnected()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("WiFi connection failed!");
    display.display();
    return;
  }

  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac);
  char deviceId[18];
  snprintf(deviceId, sizeof(deviceId), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Connected. ID: ");
  display.print(deviceId);
  display.display();

  doSetup(deviceId);
  doDisplay(display, deviceId);
}
