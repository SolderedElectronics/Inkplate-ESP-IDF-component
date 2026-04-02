#include "SDCard.h"
#include "driver/sdspi_host.h"
#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "esp_rom_sys.h"

/**
 * @brief  Store a reference to the IO expander used to control the SD power
 *         switch.
 *
 * @param  PCAL &expander
 *         Reference to the PCAL expander that has SD_PMOS_PIN.
 */
SDCard::SDCard(PCAL &expander) : m_expander(expander)
{
}

/**
 * @brief  Power on the SD card and mount the FAT filesystem via VFS.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an error code from the SPI/VFS driver.
 */
esp_err_t SDCard::sdCardInit()
{
  m_expander.setDirection(SD_PMOS_PIN, IO_MODE_OUTPUT);
  m_expander.setLevel(SD_PMOS_PIN, 0);
  esp_rom_delay_us(50000);

  spi_bus_config_t busCfg = {};
  busCfg.mosi_io_num      = SD_MOSI;
  busCfg.miso_io_num      = SD_MISO;
  busCfg.sclk_io_num      = SD_SCK;
  busCfg.quadwp_io_num    = -1;
  busCfg.quadhd_io_num    = -1;
  busCfg.max_transfer_sz  = 4096;
  esp_err_t ret = spi_bus_initialize(SPI2_HOST, &busCfg, SDSPI_DEFAULT_DMA);
  if (ret != ESP_OK) return ret;

  sdmmc_host_t host             = SDSPI_HOST_DEFAULT();
  host.slot                     = SPI2_HOST;

  sdspi_device_config_t slotCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
  slotCfg.gpio_cs               = GPIO_NUM_15;
  slotCfg.host_id               = SPI2_HOST;

  esp_vfs_fat_sdmmc_mount_config_t mountCfg = {};
  mountCfg.format_if_mount_failed           = false;
  mountCfg.max_files                        = 5;
  mountCfg.allocation_unit_size             = 16 * 1024;

  ret = esp_vfs_fat_sdspi_mount(SD_MOUNT_POINT, &host, &slotCfg, &mountCfg, &m_card);
  if (ret != ESP_OK) return ret;

  sdmmc_card_print_info(stdout, m_card);
  return ret;
}

/**
 * @brief  Unmount the filesystem, free the SPI bus, and cut power to the card.
 *         Sets all SPI lines and the power switch pin as inputs to minimise
 *         current draw.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an error code if unmounting failed.
 */
esp_err_t SDCard::sdCardSleep()
{
  esp_err_t ret = ESP_OK;
  if (m_card)
  {
    ret    = esp_vfs_fat_sdcard_unmount(SD_MOUNT_POINT, m_card);
    m_card = nullptr;
  }
  spi_bus_free(SPI2_HOST);

  gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT);
  m_expander.setDirection(SD_PMOS_PIN, IO_MODE_INPUT);

  return ret;
}

/**
 * @brief  Get the mount point string for constructing file paths.
 *
 * @return const char*
 *         Mount point, e.g. "/sdcard".
 */
const char *SDCard::getMountPoint()
{
  return SD_MOUNT_POINT;
}
