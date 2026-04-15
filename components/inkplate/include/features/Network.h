#ifndef NETWORK_H
#define NETWORK_H

#include "esp_event.h"
#include "esp_err.h"
#include <stdint.h>

class WiFi
{
public:
  WiFi() = default;
  esp_err_t   begin();
  bool        waitForConnect(uint32_t timeoutMs = 10000);
  void        setCurrentTime();
  uint8_t    *downloadFile(const char *url, int32_t *len);
  uint8_t    *downloadFileHTTPS(const char *url, int32_t *len);

  void        setCertificate(const char *cert) { m_certificate = cert; }

  bool        isConnected() { return m_connected; }
  bool        isTimeSet()   { return m_timeSet; }

private:
  static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
  static void ipEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

  static bool m_connected;
  bool        m_timeSet     = false;
  const char *m_certificate = nullptr;
};

#endif
