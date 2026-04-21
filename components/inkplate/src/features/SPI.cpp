#include "SPI.h"
#include "esp_log.h"
#include "string.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "SPI";

SPI::SPI(gpio_num_t mosi, gpio_num_t clk, spi_host_device_t host)
{
  m_mosi = mosi;
  m_clk = clk;
  m_host = host;
}

bool SPI::init()
{
  if (m_spiDev) return true;

  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num   = m_mosi;
  bus_cfg.miso_io_num   = -1;
  bus_cfg.sclk_io_num   = m_clk;
  bus_cfg.quadwp_io_num = -1;
  bus_cfg.quadhd_io_num = -1;
  ESP_ERROR_CHECK(spi_bus_initialize(m_host, &bus_cfg, SPI_DMA_CH_AUTO));

  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = SPI_MASTER_FREQ_10M;
  dev_cfg.mode           = 0;
  dev_cfg.spics_io_num   = -1;
  dev_cfg.queue_size     = 3;
  ESP_ERROR_CHECK(spi_bus_add_device(m_host, &dev_cfg, &m_spiDev));

  return true;
}

void SPI::deinit()
{
  if (m_spiDev)
  {
    spi_bus_remove_device(m_spiDev);
    m_spiDev = nullptr;
  }
  spi_bus_free(m_host);
}

void SPI::sendCommand(uint8_t command, gpio_num_t dcPin)
{
  gpio_set_level(dcPin, 0);
  esp_rom_delay_us(10);

  spi_transaction_t t = {};
  t.length    = 8;
  t.tx_buffer = &command;
  spi_device_polling_transmit(m_spiDev, &t);

  esp_rom_delay_us(1000);
}

void SPI::sendData(uint8_t *data, int n, gpio_num_t dcPin)
{
  if (n == 0) return;

  gpio_set_level(dcPin, 1);
  esp_rom_delay_us(10);

  const size_t chunkSize = 4092;

  for (int i = 0; i < n; i += chunkSize)
  {
    int len = (n - i > (int)chunkSize) ? (int)chunkSize : (n - i);

    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));
    trans.tx_buffer = data + i;
    trans.length    = len * 8;

    ESP_ERROR_CHECK(spi_device_transmit(m_spiDev, &trans));
  }

  esp_rom_delay_us(1000);
}

void SPI::sendData(uint8_t data, gpio_num_t dcPin)
{
  sendData(&data, 1, dcPin);
}
