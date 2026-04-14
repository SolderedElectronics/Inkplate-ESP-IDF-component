#include "soc/i2s_struct.h"
#include "soc/gpio_sig_map.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "string.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"

#include "Inkplate6Color.h"
#include "TPS.h"

// Peripherals defined in BoardCommon.cpp
extern TPS    tps;
extern I2C    i2c;

static const char *TAG = "INKPLATE6COLOR";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Inkplate6 constructor.
 *
 * @note   Allocates framebuffers in PSRAM, pre-computes grayscale waveform LUTs,
 *         initialises GPIO and the PMIC.
 */
Inkplate6Color::Inkplate6Color() : BoardCommon(E_INK_WIDTH, E_INK_HEIGHT, 21, 12)
{
  ESP_ERROR_CHECK(initBuffers());

  clearDisplay();

  gpio_set_direction(EPAPER_RST_PIN,  GPIO_MODE_OUTPUT);
  gpio_set_direction(EPAPER_DC_PIN,   GPIO_MODE_OUTPUT);
  gpio_set_direction(EPAPER_CS_PIN,   GPIO_MODE_OUTPUT);
  gpio_set_direction(EPAPER_CLK,      GPIO_MODE_OUTPUT);
  gpio_set_direction(EPAPER_DIN,      GPIO_MODE_OUTPUT);

  gpio_set_level(EPAPER_RST_PIN, 0);
  gpio_set_level(EPAPER_DC_PIN,  0);
  gpio_set_level(EPAPER_CS_PIN,  0);
  gpio_set_level(EPAPER_CLK,     0);
  gpio_set_level(EPAPER_DIN,     0);

  gpio_set_direction(EPAPER_BUSY_PIN, GPIO_MODE_INPUT);
  gpio_pullup_en(EPAPER_BUSY_PIN);

  //vTaskDelay(pdMS_TO_TICKS(5000));

  spi_bus_config_t bus_cfg = {};
  bus_cfg.mosi_io_num     = EPAPER_DIN;
  bus_cfg.miso_io_num     = -1;
  bus_cfg.sclk_io_num     = EPAPER_CLK;
  bus_cfg.quadwp_io_num   = -1;
  bus_cfg.quadhd_io_num   = -1;
  ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

  spi_device_interface_config_t dev_cfg = {};
  dev_cfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
  dev_cfg.mode           = 0;
  dev_cfg.spics_io_num   = EPAPER_CS_PIN;
  dev_cfg.queue_size     = 3;
  ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev_cfg, &m_spiDev));

  if (!setPanelDeepSleep(false))
    ESP_LOGE(TAG, "Panel init failed");

  setPanelDeepSleep(true);
  rtc.begin(i2c.getBusHandle());

  ESP_LOGI(TAG, "Initialization finished!");
}

/**
 * @brief  Power on the e-ink panel and assert all required control signals.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_TIMEOUT if the PMIC does not reach
 *         power-good within 250 ms.
 */
esp_err_t Inkplate6Color::einkOn()
{
  return ESP_OK;
}

/**
 * @brief  Power off the e-ink panel and tri-state all data lines.
 *
 * @return esp_err_t
 *         ESP_OK on success, or a TPS driver error code.
 */
esp_err_t Inkplate6Color::einkOff()
{
  return ESP_OK;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Allocate all framebuffers, DMA buffers, and LUT arrays.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_NO_MEM if any allocation fails
 */
esp_err_t Inkplate6Color::initBuffers()
{
  m_framebufferColor = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM  | MALLOC_CAP_8BIT);
  if (!m_framebufferColor) return ESP_ERR_NO_MEM;
  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);

  return ESP_OK;
}

/**
 * @brief  Push the 3-bit grayscale framebuffer to the display.
 *
 * @param  bool leaveOn
 *         If true, leaves the e-ink panel powered on after the update
 *
 * @return esp_err_t
 *         ESP_OK on success, or an error code if einkOn() failed
 */
esp_err_t Inkplate6Color::display3b(bool leaveOn)
{
  setPanelDeepSleep(false);

  // set resolution setting
  uint8_t data[4] = {0x02, 0x58, 0x01, 0xc0};
  sendCommand(0x61);
  sendData(data, 4);

  // push pixdel data to epaper ram
  sendCommand(0x10);

  sendData(m_framebufferColor, m_einkWidth * m_einkHeight / 2);

  sendCommand(POWER_OFF_REGISTER);
  waitForEpd(60000);
  sendCommand(DISPLAY_REF_REGISTER);
  waitForEpd(60000);
  sendCommand(0x02);
  waitForEpd(60000);

  setPanelDeepSleep(true);

  return ESP_OK;
}

bool Inkplate6Color::waitForEpd(uint32_t timeout)
{
  uint32_t elapsed = 0;
  const uint32_t STEP = 10;

  while (gpio_get_level(EPAPER_BUSY_PIN) == 0)
  {
    if (elapsed >= timeout)
    {
      ESP_LOGE(TAG, "EPD busy timeout");
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(STEP));
    elapsed += STEP;
  }
  vTaskDelay(pdMS_TO_TICKS(200));
  return true;
}

void Inkplate6Color::resetPanel()
{
  gpio_set_level(EPAPER_RST_PIN, 0);
  vTaskDelay(pdMS_TO_TICKS(1));
  gpio_set_level(EPAPER_RST_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(200));
}

void Inkplate6Color::sendCommand(uint8_t command)
{
  //gpio_set_level(EPAPER_CS_PIN, 0);
  gpio_set_level(EPAPER_DC_PIN, 0);
  esp_rom_delay_us(10);

  spi_transaction_t t = {};
  t.length    = 8;
  t.tx_buffer = &command;
  spi_device_polling_transmit(m_spiDev, &t);

  //gpio_set_level(EPAPER_CS_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(1));
}
void Inkplate6Color::sendData(uint8_t *data, int n)
{
  if (n == 0) return;

  //gpio_set_level(EPAPER_CS_PIN, 0);
  gpio_set_level(EPAPER_DC_PIN, 1);
  esp_rom_delay_us(10);

  const size_t chunkSize = 4092;

  for (int i = 0; i < n; i += chunkSize)
  {
    int len = (n - i > chunkSize) ? chunkSize : (n - i);

    spi_transaction_t trans;
    memset(&trans, 0, sizeof(trans));

    trans.tx_buffer = data + i;
    trans.length = len * 8;

    ESP_ERROR_CHECK(spi_device_transmit(m_spiDev, &trans));
  }

  //gpio_set_level(EPAPER_CS_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(1));
}

void Inkplate6Color::sendData(uint8_t data)
{
  sendData(&data, 1);
}

bool Inkplate6Color::setPanelDeepSleep(bool sleep)
{
  if (!sleep)
  {
    if (!m_spiDev)
    {
      // Re-initialize SPI bus on wake
      spi_bus_config_t bus_cfg = {};
      bus_cfg.mosi_io_num     = EPAPER_DIN;
      bus_cfg.miso_io_num     = -1;
      bus_cfg.sclk_io_num     = EPAPER_CLK;
      bus_cfg.quadwp_io_num   = -1;
      bus_cfg.quadhd_io_num   = -1;
      ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &bus_cfg, SPI_DMA_CH_AUTO));
      
      spi_device_interface_config_t dev_cfg = {};
      dev_cfg.clock_speed_hz = SPI_MASTER_FREQ_20M;
      dev_cfg.mode           = 0;
      dev_cfg.spics_io_num   = EPAPER_CS_PIN;
      dev_cfg.queue_size     = 3;
      ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev_cfg, &m_spiDev));
    }
      
    // Wake
    gpio_set_direction(EPAPER_BUSY_PIN, GPIO_MODE_INPUT);
    gpio_pullup_en(EPAPER_BUSY_PIN);
    resetPanel();

    waitForEpd(60000);

    uint8_t panel_set_data[] = {0xEF, 0x08};
    sendCommand(PANEL_SET_REGISTER);
    sendData(panel_set_data, 2);

    uint8_t power_set_data[] = {0x37, 0x00, 0x05, 0x05};
    sendCommand(POWER_SET_REGISTER);
    sendData(power_set_data, 4);

    sendCommand(POWER_OFF_SEQ_SET_REGISTER);
    sendData(0x00);

    uint8_t booster_softstart_data[] = {0xC7, 0xC7, 0x1D};
    sendCommand(BOOSTER_SOFTSTART_REGISTER);
    sendData(booster_softstart_data, 3);

    sendCommand(TEMP_SENSOR_EN_REGISTER);
    sendData(0x00);

    sendCommand(VCOM_DATA_INTERVAL_REGISTER);
    sendData(0x37);

    sendCommand(0x60);
    sendData(0x20);

    uint8_t res_set_data[] = {0x02, 0x58, 0x01, 0xC0};
    sendCommand(RESOLUTION_SET_REGISTER);
    sendData(res_set_data, 4);

    sendCommand(0xE3);
    sendData(0xAA);

    vTaskDelay(pdMS_TO_TICKS(100));
    sendCommand(VCOM_DATA_INTERVAL_REGISTER);
    sendData(0x37);

    return true;
  }
  else
  {
    // Sleep
    vTaskDelay(pdMS_TO_TICKS(10));
    sendCommand(DEEP_SLEEP_REGISTER);
    sendData(0xA5);

    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(EPAPER_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(EPAPER_DC_PIN, 0);
    gpio_set_level(EPAPER_CS_PIN, 0);

    // Free SPI bus to release DMA channel
    if (m_spiDev)
    {
      spi_bus_remove_device(m_spiDev);
      m_spiDev = nullptr;
    }
    spi_bus_free(SPI3_HOST);

    return true;
  }
}