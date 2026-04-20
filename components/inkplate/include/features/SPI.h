#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"

class SPI {
public:
  SPI(gpio_num_t mosi, gpio_num_t clk, gpio_num_t cs, spi_host_device_t host = SPI3_HOST);

  bool    init();
  void    deinit();
  bool    isInitialized() const { return m_spiDev != nullptr; }

  void    sendCommand(uint8_t command, gpio_num_t dcPin);
  void    sendData(uint8_t *data, int n, gpio_num_t dcPin);
  void    sendData(uint8_t data, gpio_num_t dcPin);

  // Dual-CS write: caller controls CS manually, SPIBus just does the transaction
  void    beginTransaction();
  void    endTransaction();
  void    write(uint8_t byte);
  void    writeBytes(const uint8_t *data, size_t n);

private:
    gpio_num_t          m_mosi;
    gpio_num_t          m_clk;
    gpio_num_t          m_cs;
    spi_host_device_t   m_host;
    spi_device_handle_t m_spiDev = nullptr;
};