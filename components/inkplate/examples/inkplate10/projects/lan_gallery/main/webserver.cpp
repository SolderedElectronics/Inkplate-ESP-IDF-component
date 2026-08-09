/**
 * @file        webserver.cpp
 * @author      Fran Fodor for Soldered
 * @brief       HTTP routes for the LAN gallery web server (Inkplate 10).
 *
 * @details     Implements the two routes used by the LAN image gallery:
 *              GET "/" serves the upload page defined in main/html.h, and
 *              POST "/upload" receives an uploaded picture and hands it to
 *              the SD-card writing helpers implemented in main.cpp.
 *
 *              This mirrors the split used by the original Arduino sketch
 *              (Inkplate10_Lan_Gallery.ino + webserver.cpp): the web server
 *              only knows about HTTP, while main.cpp owns the SD card, the
 *              gallery file list, and the e-paper display.
 *
 * Notes:
 * - Arduino's ESPAsyncWebServer has no equivalent in this component. This
 *   file is built directly on ESP-IDF's native esp_http_server component
 *   instead, with routes registered via httpd_register_uri_handler().
 *   ESPAsyncWebServer's upload callback used to hand the sketch a clean
 *   filename and raw file bytes because it parses multipart/form-data
 *   internally; esp_http_server does not, so this example does not attempt
 *   a full multipart parser either. The upload page (main/html.h) always
 *   re-encodes the picked image to JPEG client-side before uploading, so
 *   the handler below just scans the raw POST body for the JPEG start-of-
 *   image (0xFF 0xD8) and end-of-image (0xFF 0xD9) markers to locate the
 *   picture bytes, and a fresh file name is generated on the device.
 * - The uploaded image is buffered fully in RAM (from SPIRAM when
 *   available) for the duration of the request, sized from the request's
 *   Content-Length header.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "webserver.h"
#include "html.h"

#include "esp_heap_caps.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_random.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "LAN_GALLERY_HTTP";

// Functions implemented in main.cpp, used to write the uploaded picture to
// the SD card while holding the shared SD-card mutex.
void startFileUpload(const char *filename);
void writeFileData(const uint8_t *data, size_t len);
void finishFileUpload();

/* -------------------------------------------------------------------------- */
/*                                  Helpers                                   */
/* -------------------------------------------------------------------------- */

// Scans a buffer for the JPEG start-of-image marker (0xFF 0xD8).
static int findJpegStart(const uint8_t *buf, size_t len) {
  for (size_t i = 0; i + 1 < len; i++) {
    if (buf[i] == 0xFF && buf[i + 1] == 0xD8)
      return (int)i;
  }
  return -1;
}

// Scans a buffer (from "from" onward) for the last JPEG end-of-image marker
// (0xFF 0xD9). This trims off the multipart/form-data trailer (boundary and
// headers) that follows the picture bytes in the raw POST body.
static int findJpegEnd(const uint8_t *buf, size_t len, size_t from) {
  int last = -1;
  for (size_t i = from; i + 1 < len; i++) {
    if (buf[i] == 0xFF && buf[i + 1] == 0xD9)
      last = (int)i + 1;
  }
  return last;
}

/* -------------------------------------------------------------------------- */
/*                              HTTP request handlers                        */
/* -------------------------------------------------------------------------- */

// GET "/" - serves the upload page.
static esp_err_t handleRoot(httpd_req_t *req) {
  httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

// POST "/upload" - receives the uploaded picture and writes it to the SD
// card as a new gallery image.
static esp_err_t handleUpload(httpd_req_t *req) {
  size_t remaining = req->content_len;
  if (remaining == 0) {
    ESP_LOGE(TAG, "Upload request has no body");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  // Prefer SPIRAM for the upload buffer since pictures can be a few hundred
  // KB once re-encoded; fall back to plain heap if SPIRAM isn't available.
  uint8_t *buf =
      (uint8_t *)heap_caps_malloc(remaining, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!buf)
    buf = (uint8_t *)malloc(remaining);
  if (!buf) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for upload", (unsigned)remaining);
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
  int jpegEnd = (jpegStart >= 0) ? findJpegEnd(buf, received, jpegStart) : -1;
  if (jpegStart < 0 || jpegEnd <= jpegStart) {
    ESP_LOGE(TAG, "No JPEG data found in uploaded body");
    free(buf);
    httpd_resp_send(req, "No image data found", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
  }
  size_t jpegLen = (size_t)(jpegEnd - jpegStart);

  // Generate a fresh, unique file name for the new gallery picture (the
  // original filename isn't recovered without a full multipart parser, see
  // the note at the top of this file).
  char filename[32];
  snprintf(filename, sizeof(filename), "img_%08x.jpg",
           (unsigned)esp_random());

  ESP_LOGI(TAG, "Saving upload as %s (%u bytes)", filename, (unsigned)jpegLen);
  startFileUpload(filename);
  writeFileData(buf + jpegStart, jpegLen);
  finishFileUpload();

  free(buf);

  httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
  return ESP_OK;
}

/* -------------------------------------------------------------------------- */
/*                                    Setup                                   */
/* -------------------------------------------------------------------------- */

void setupWebServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();

  httpd_handle_t server = nullptr;
  if (httpd_start(&server, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server");
    return;
  }

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
