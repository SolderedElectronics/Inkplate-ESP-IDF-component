#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"

class SPI {
public:
  SPI(gpio_num_t mosi, gpio_num_t clk, spi_host_device_t host = SPI3_HOST);

  bool    init();
  void    deinit();
  bool    isInitialized() const { return m_spiDev != nullptr; }

  void    sendCommand(uint8_t command, gpio_num_t dcPin);
  void    sendData(uint8_t *data, int n, gpio_num_t dcPin);
  void    sendData(uint8_t data, gpio_num_t dcPin);

private:
  gpio_num_t          m_mosi;
  gpio_num_t          m_clk;
  spi_host_device_t   m_host;
  spi_device_handle_t m_spiDev = nullptr;
};
