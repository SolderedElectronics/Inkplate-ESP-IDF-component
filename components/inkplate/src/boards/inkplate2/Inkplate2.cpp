#include "Inkplate2.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "esp_heap_caps.h"
#include <string.h>

static const char *TAG = "Inkplate2";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

Inkplate2::Inkplate2()
{
    m_framebufferColor = (uint8_t *)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 4, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (!m_framebufferColor)
    {
        ESP_LOGE(TAG, "Failed to allocate framebuffer");
    }

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
    dev_cfg.clock_speed_hz = 1000000;
    dev_cfg.mode           = 0;
    dev_cfg.spics_io_num   = EPAPER_CS_PIN;
    dev_cfg.queue_size     = 3;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &dev_cfg, &m_spiDev));

    if (!setPanelDeepSleep(false))
    {
        ESP_LOGE(TAG, "Panel init failed");
    }
    setPanelDeepSleep(true);
}

void Inkplate2::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || y < 0 || x >= E_INK_WIDTH || y >= E_INK_HEIGHT)
        return;
    if (color > 2)
        return;

    int _x    = x / 8;
    int _xSub = x % 8;
    int _position = (E_INK_WIDTH / 8) * y + _x;

    const uint8_t pixelMaskLUT[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

    // Set both planes to 1 first (clear)
    *(m_framebufferColor + _position) |= pixelMaskLUT[7 - _xSub];
    *(m_framebufferColor + (E_INK_WIDTH * E_INK_HEIGHT / 8) + _position) |= pixelMaskLUT[7 - _xSub];

    if (color < 2)
    {
        *(m_framebufferColor + _position) &= ~(color << (7 - _xSub));
    }
    else
    {
        *(m_framebufferColor + (E_INK_WIDTH * E_INK_HEIGHT / 8) + _position) &= ~pixelMaskLUT[7 - _xSub];
    }
}

esp_err_t Inkplate2::display(bool leaveOn)
{
    const size_t plane_bytes = E_INK_WIDTH * E_INK_HEIGHT / 8;

    setPanelDeepSleep(false);
    vTaskDelay(pdMS_TO_TICKS(20));

    sendCommand(0x10);
    sendData(m_framebufferColor, plane_bytes);

    sendCommand(0x13);
    sendData(m_framebufferColor + plane_bytes, plane_bytes);

    sendCommand(0x11);
    sendData((uint8_t)0x00);
    sendCommand(0x12);
    esp_rom_delay_us(500);
    waitForEpd(60000);

    if (!leaveOn)
        setPanelDeepSleep(true);

    return ESP_OK;
}

void Inkplate2::setDisplayMode(uint8_t displayMode)
{
    m_displayMode = displayMode;
}

void Inkplate2::clearDisplay()
{
    if (m_framebufferColor)
        memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 4);
}

void Inkplate2::fillDisplay()
{
    if (m_framebufferColor)
        memset(m_framebufferColor, 0x00, E_INK_WIDTH * E_INK_HEIGHT / 4);
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

bool Inkplate2::waitForEpd(uint32_t timeout)
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

void Inkplate2::resetPanel()
{
    gpio_set_level(EPAPER_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(EPAPER_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void Inkplate2::sendCommand(uint8_t command)
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

void Inkplate2::sendData(uint8_t *data, int n)
{
    if (n == 0) return;

    //gpio_set_level(EPAPER_CS_PIN, 0);
    gpio_set_level(EPAPER_DC_PIN, 1);
    esp_rom_delay_us(10);

    spi_transaction_t trans;
	memset(&trans, 0, sizeof(spi_transaction_t));
	
	trans.tx_buffer = data;
	trans.length = n*8;
	
	if (spi_device_transmit(m_spiDev, &trans) != ESP_OK)
	{
		printf("writing error\n");	
	}
    //gpio_set_level(EPAPER_CS_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1));
}

void Inkplate2::sendData(uint8_t data)
{
    sendData(&data, 1);
}

bool Inkplate2::setPanelDeepSleep(bool sleep)
{
    if (!sleep)
    {
        // Wake
        gpio_set_direction(EPAPER_BUSY_PIN, GPIO_MODE_INPUT);
        gpio_pullup_en(EPAPER_BUSY_PIN);
        resetPanel();

        sendCommand(0x04); // Power on
        if (!waitForEpd(BUSY_TIMEOUT_MS))
            return false;

        sendCommand(0x00);      // Panel setting
        sendData((uint8_t)0x0f); // LUT from OTP
        sendData((uint8_t)0x89); // Temp sensor, boost, timing

        sendCommand(0x61);                          // Resolution setting
        sendData((uint8_t)E_INK_WIDTH);
        sendData((uint8_t)(E_INK_HEIGHT >> 8));
        sendData((uint8_t)(E_INK_HEIGHT & 0xff));

        sendCommand(0x50);       // VCOM and data interval
        sendData((uint8_t)0x77);

        return true;
    }
    else
    {
        // Sleep
        sendCommand(0x50);
        sendData((uint8_t)0xf7);

        sendCommand(0x02); // Power off
        waitForEpd(BUSY_TIMEOUT_MS);

        sendCommand(0x07); // Deep sleep
        sendData((uint8_t)0xA5);

        vTaskDelay(pdMS_TO_TICKS(1));

        gpio_set_direction(EPAPER_BUSY_PIN, GPIO_MODE_INPUT);

        return true;
    }
}