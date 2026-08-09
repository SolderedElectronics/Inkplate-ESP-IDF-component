/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       WiFi image uploader for Soldered Inkplate 2.
 *
 * @details     Turns Inkplate 2 into a small standalone web app for
 *              uploading an image from a phone or PC. The device connects
 *              to a WiFi network and starts an HTTP server (ESP-IDF's
 *              esp_http_server component) that serves an upload page
 *              (main/html.h). The page lets the user take a photo or pick
 *              an image from their gallery, scales/crops it to the panel
 *              resolution in the browser, and POSTs it to the device. The
 *              device then decodes the uploaded JPEG and renders it on the
 *              e-paper display.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 2
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 2, USB cable
 * - Extra:      WiFi network + phone/PC with a web browser
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate2
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 *
 * How to use:
 * 1) Build and flash to Inkplate 2.
 * 2) Open the Serial Monitor. Once WiFi connects, the device's IP address
 *    is printed to the log (and shown on the display).
 * 3) On a phone/PC connected to the same network, open
 *    http://<printed-ip>/ in a web browser.
 * 4) Take a photo or choose an image from the gallery, then press Upload.
 * 5) The device decodes the uploaded image and renders it on the e-paper
 *    display.
 *
 * Expected output:
 * - Serial Monitor: WiFi connection status and the device's IP address.
 * - Display: connection instructions, then the uploaded image after a
 *   successful upload.
 * - Browser: the upload page served from main/html.h at "/".
 *
 * Notes:
 * - Arduino's WebServer class has no equivalent in this component. This
 *   example uses ESP-IDF's native esp_http_server component instead, with
 *   handlers registered via httpd_register_uri_handler().
 * - The browser posts the picked/captured image as multipart/form-data, so
 *   the raw POST body contains form-field headers and boundaries around the
 *   actual JPEG bytes. Rather than implementing a full multipart parser,
 *   this example scans the received body for the JPEG start-of-image marker
 *   (0xFF 0xD8) to locate the embedded image before handing it to
 *   display.image.draw().
 * - The uploaded image is buffered fully in RAM, sized from the request's
 *   Content-Length header; very large uploads may fail to allocate.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate2 in the boards menu."
#endif

#include "Inkplate.h"
#include "html.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdlib.h>

static const char *TAG = "IMAGE_UPLOADER";

// Display instance shared with the HTTP handlers below. Handlers registered
// with esp_http_server are free functions (no lambda captures), so the
// display is kept as a file-scope static instead of a local in app_main().
static Inkplate display;

/* -------------------------------------------------------------------------- */
/*                              HTTP request handlers                         */
/* -------------------------------------------------------------------------- */

// GET "/" - serves the upload page.
static esp_err_t handleRoot(httpd_req_t *req) {
  httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// Scans a buffer for the JPEG start-of-image marker (0xFF 0xD8).
//
// The browser posts the image as multipart/form-data, so the raw POST body
// contains form-field headers/boundaries wrapped around the JPEG bytes.
// This locates where the actual JPEG data begins without implementing a
// full multipart parser.
static int findJpegStart(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i + 1 < len; i++) {
    if (buf[i] == 0xFF && buf[i + 1] == 0xD8)
      return (int)i;
  }
  return -1;
}

// POST "/upload" - receives the uploaded image, decodes it, and renders it
// on the e-paper display.
static esp_err_t handleUpload(httpd_req_t *req) {
  size_t remaining = req->content_len;
  if (remaining == 0) {
    ESP_LOGE(TAG, "Upload request has no body");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  uint8_t *buf = (uint8_t *)malloc(remaining);
  if (!buf) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for upload",
             (unsigned)remaining);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  // Read the full request body into the buffer. httpd_req_recv() may return
  // fewer bytes than requested per call, so keep reading until the whole
  // body has been received.
  size_t received = 0;
  while (remaining > 0) {
    int r = httpd_req_recv(req, (char *)(buf + received), remaining);
    if (r == HTTPD_SOCK_ERR_TIMEOUT) {
      continue; // retry on timeout
    }
    if (r <= 0) {
      ESP_LOGE(TAG, "Upload receive failed (%d)", r);
      free(buf);
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    received += (size_t)r;
    remaining -= (size_t)r;
  }
  ESP_LOGI(TAG, "Upload complete, %u bytes received", (unsigned)received);

  int jpegStart = findJpegStart(buf, received);
  if (jpegStart < 0) {
    ESP_LOGE(TAG, "No JPEG data found in uploaded body");
    free(buf);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  display.clearDisplay();
  display.image.draw(buf + jpegStart, (int32_t)(received - jpegStart), 0, 0,
                      true, false);
  display.display();

  free(buf);

  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*                                   Helpers                                  */
/* -------------------------------------------------------------------------- */

// Prints connection instructions on the e-paper display.
static void showConnectionInfo(const esp_ip4_addr_t &ip) {
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), IPSTR, IP2STR(&ip));

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(INKPLATE2_BLACK, INKPLATE2_WHITE);

  display.setCursor(0, 0);
  display.print("Inkplate Image Uploader");

  display.setCursor(0, 20);
  display.print("Open in your browser:");

  display.setCursor(0, 30);
  display.print("http://");
  display.print(ipStr);
  display.print("/");

  display.display();
}

/* -------------------------------------------------------------------------- */
/*                                    Main                                    */
/* -------------------------------------------------------------------------- */

extern "C" void app_main(void) {
  // Connect to WiFi using the credentials configured via menuconfig.
  display.wifi.begin();
  if (!display.wifi.waitForConnect()) {
    ESP_LOGE(TAG, "Failed to connect to WiFi, check menuconfig credentials");
  }

  // Fetch the IP address assigned to the station interface and log it.
  esp_ip4_addr_t ip = {};
  esp_netif_t *staNetif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (staNetif != nullptr) {
    esp_netif_ip_info_t ipInfo;
    if (esp_netif_get_ip_info(staNetif, &ipInfo) == ESP_OK) {
      ip = ipInfo.ip;
      ESP_LOGI(TAG, "Connected! Open http://" IPSTR "/ in your browser",
               IP2STR(&ip));
    }
  }

  showConnectionInfo(ip);

  // Start the HTTP server and register the route handlers.
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = nullptr;
  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
  } else {
    httpd_uri_t rootUri = {};
    rootUri.uri = "/";
    rootUri.method = HTTP_GET;
    rootUri.handler = handleRoot;
    httpd_register_uri_handler(server, &rootUri);

    httpd_uri_t uploadUri = {};
    uploadUri.uri = "/upload";
    uploadUri.method = HTTP_POST;
    uploadUri.handler = handleUpload;
    httpd_register_uri_handler(server, &uploadUri);

    ESP_LOGI(TAG, "HTTP server started");
  }

  // Nothing left to do in app_main() - esp_http_server dispatches requests
  // from its own task, so just keep this task alive.
  while (true) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
