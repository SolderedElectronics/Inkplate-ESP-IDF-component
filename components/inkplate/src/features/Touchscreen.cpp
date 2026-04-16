#include "Touchscreen.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

static const char *TAG = "TouchCypress";

#define TS_GET_BOOTLOADERMODE(reg) (((reg)&0x10) >> 4)

static volatile bool tsFlag = false;

static void IRAM_ATTR tsInt(void *arg)
{
  tsFlag = true;
}

esp_err_t Touch::begin(I2C &i2c, PCAL &expander, uint8_t powerState)
{
  m_expander = &expander;

  i2c.addDevice(TOUCHSCREEN_I2C_ADDR, &m_devHandle);

  if (powerState)
  {
    power(powerState);

    reset();

    if (!ping(5))
    {
      ESP_LOGI(TAG, "asd");
      return ESP_FAIL;
    }

    ESP_ERROR_CHECK(sendCommand(0x01));

    ESP_ERROR_CHECK(loadBootloaderRegs(&m_blData));

    ESP_ERROR_CHECK(exitBootloaderMode());

    ESP_ERROR_CHECK(setSysInfoMode(&m_sysData));

    ESP_ERROR_CHECK(setSysInfoRegs(&m_sysData));
    
    // Switch it into operate mode (also can be in deep sleep mode as well as low power mode).
    ESP_ERROR_CHECK(sendCommand(CYPRESS_TOUCH_OPERATE_MODE));
    // Set dist value for detection?
    uint8_t distDefaultValue = 0xF8;
    ESP_ERROR_CHECK(writeI2CRegs(0x1E, &distDefaultValue, 1));

    gpio_set_direction(TOUCHSCREEN_INT, GPIO_MODE_INPUT);
    gpio_set_intr_type(TOUCHSCREEN_INT, GPIO_INTR_NEGEDGE);

    if (!m_tsInitDone)
    {
      ESP_ERROR_CHECK(gpio_install_isr_service(0));
      ESP_ERROR_CHECK(gpio_isr_handler_add(TOUCHSCREEN_INT, tsInt, NULL));
    }
    
    vTaskDelay(pdMS_TO_TICKS(50));

    tsFlag = false;
    m_tsInitDone = true;

    ESP_LOGI(TAG, "done init");
  }
  else
  {
    end();
  }

  return ESP_OK;
}

uint8_t Touch::getData(uint16_t *xPos, uint16_t *yPos, uint8_t *z)
{
  // Struct typedef for touch data report from the touchscreen controller IC.
  struct cypressTouchData touchReport;

  // Check for the null-pointer.
  if (xPos == NULL || yPos == NULL)
    return 0;

  // Fill the array with zeros.
  xPos[0] = 0;
  xPos[1] = 0;
  yPos[0] = 0;
  yPos[1] = 0;

  // Copy Z into array only if array address is passed as argument in function.
  if (z != NULL)
  {
    z[0] = 0;
    z[1] = 0;
  }

  // Check the flag for the new data.
  if (!tsFlag)
    return 0;

  // If there is new data, clear the interrupt flag.
  tsFlag = false;

  // Read the new data from the touchscreen controller IC.
  // Return zero detected fingers if reading has failed.
  if (!getTouchData(&touchReport))
    return 0;

  // Scale it to fit the screen.
  scale(&touchReport, E_INK_WIDTH - 1, E_INK_HEIGHT - 1, false, true, true);


  // Copy values into ararys.
  for (int i = 0; i < touchReport.fingers; i++)
  {
    // Save values into the arrays.
    xPos[i] = touchReport.x[i];
    yPos[i] = touchReport.y[i];

    // Copy Z into array only if array address is passed as argument in function.
    if (z != NULL)
    {
      z[i] = touchReport.z[i];
    }
  }

  // Return number of detected fingers on the touchscreen.
  return touchReport.fingers;
}



bool Touch::getTouchData(struct cypressTouchData *touchData)
{
  // Check for the null-pointer trap.
  if (touchData == NULL)
    return false;

  // Clear struct for touchscreen data.
  memset(touchData, 0, sizeof(cypressTouchData));

  // Buffer for the I2C registers.
  uint8_t regs[32];

  // Read registers for the touch data (32 bytes of data).
  // If read failed for some reason, return false.
  if (!readI2CRegs(CYPRESS_TOUCH_BASE_ADDR, regs, sizeof(regs)))
    return false;

  // Send a handshake.
  handshake();

  // Parse the data!
  // Data goes as follows:
  // [1 byte] Handshake bit - Must be written back with xor on last MSB bit for TSC knows that INT has been read.
  // [1 byte] Something? It changes with every new data. Data is always 0x00, 0x40, 0x80, 0xC0)
  // [1 byte] Number of fingers detected - Zero, one or two.
  // [2 bytes] X value position of the finger that has been detected first.
  // [2 bytes] Y value position of the finger that has been detected first.
  // [1 byte] Z value or the presusre os the touch on the first finger.
  // [1 byte] Type of detection - 0 or 255 finger released
  // [2 bytes] X value position of the finger that has been detected second.
  // [2 bytes] Y value position of the finger that has been detected second.
  // [1 byte] Z value or the presusre os the touch on the second finger.
  touchData->x[0] = regs[3] << 8 | regs[4];
  touchData->y[0] = regs[5] << 8 | regs[6];
  touchData->z[0] = regs[7];
  touchData->x[1] = regs[9] << 8 | regs[10];
  touchData->y[1] = regs[11] << 8 | regs[12];
  touchData->z[1] = regs[13];
  touchData->detectionType = regs[8];
  touchData->fingers = regs[2];

  // Everything went ok? Return true.
  return true;
}


void Touch::scale(struct cypressTouchData *touchData, uint16_t xSize, uint16_t ySize, bool flipX, bool flipY, bool swapXY)
{
  // Temp. variables for the mapped value.
  uint16_t mappedX = 0;
  uint16_t mappedY = 0;

  // Check for NULL pointer.
  if (touchData == NULL)
    return;

  // If the number of detected fingers is different than one or two, return.
  if (touchData->fingers != 1 && touchData->fingers != 2)
    return;

  // Map both touch channels.
  for (int i = 0; i < touchData->fingers; i++)
  {
    // Check for the flip.
    if (flipX)
      touchData->x[i] = CYPRESS_TOUCH_MAX_X - touchData->x[i];
    if (flipY)
      touchData->y[i] = CYPRESS_TOUCH_MAX_Y - touchData->y[i];

    // Check for X and Y swap.
    if (swapXY)
    {
      uint16_t temp = touchData->x[i];
      touchData->x[i] = touchData->y[i];
      touchData->y[i] = temp;
    }

    // Map X value.
    mappedX = (touchData->x[i] * xSize) / CYPRESS_TOUCH_MAX_X;

    // Map Y value.
    mappedY = (touchData->y[i] * ySize) / CYPRESS_TOUCH_MAX_Y;
  }
}

bool Touch::available()
{
  return tsFlag;
}

void Touch::power(bool power)
{
  if (power)
  {
    m_expander->setLevel(TOUCHSCREEN_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    m_expander->setLevel(TOUCHSCREEN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
  }
  else
  {
    m_expander->setLevel(TOUCHSCREEN_EN, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    m_expander->setLevel(TOUCHSCREEN_RST, 0);
  }
}

void Touch::reset()
{
  m_expander->setLevel(TOUCHSCREEN_RST, 1);
  vTaskDelay(pdMS_TO_TICKS(10));
  m_expander->setLevel(TOUCHSCREEN_RST, 0);
  vTaskDelay(pdMS_TO_TICKS(2));
  m_expander->setLevel(TOUCHSCREEN_RST, 1);
  vTaskDelay(pdMS_TO_TICKS(10));
}

bool Touch::ping(int retries)
{
  for (int i = 0; i < retries; i++)
  {
    uint8_t reg = 0x00;

    esp_err_t ret = i2c_master_transmit(m_devHandle, &reg, 1, 50);

    if (ret == ESP_OK)
      return true;

    vTaskDelay(pdMS_TO_TICKS(20));
  }

  return false;
}

esp_err_t Touch::sendCommand(uint8_t cmd)
{
  uint8_t buf[2] = { CYPRESS_TOUCH_BASE_ADDR, cmd };
  esp_err_t ret = i2c_master_transmit(m_devHandle, buf, sizeof(buf), -1);
  vTaskDelay(pdMS_TO_TICKS(20));
  return ret;
}

esp_err_t Touch::loadBootloaderRegs(struct cyttspBootloaderData *blDataPtr)
{
  uint8_t bootloaderData[16];

  ESP_ERROR_CHECK(readI2CRegs(CYPRESS_TOUCH_BASE_ADDR, bootloaderData, 16));

  memcpy(blDataPtr, bootloaderData, 16);

  return ESP_OK;
}

esp_err_t Touch::readI2CRegs(uint8_t cmd, uint8_t *buffer, int len)
{
    //ESP_LOGI(TAG, "read");
  return (i2c_master_transmit_receive(m_devHandle, &cmd, 1, buffer, len, -1));
}

esp_err_t Touch::writeI2CRegs(uint8_t cmd, uint8_t *buffer, int len)
{
    //ESP_LOGI(TAG, "write");
  uint8_t *writeBuf = (uint8_t *)malloc(len + 1);
  if (writeBuf == NULL)
    return ESP_FAIL;

  writeBuf[0] = cmd;
  memcpy(writeBuf + 1, buffer, len);

  esp_err_t err = i2c_master_transmit(m_devHandle, writeBuf, len + 1, - 1);

  free(writeBuf);
  return err;
}

esp_err_t Touch::exitBootloaderMode()
{
  uint8_t blCommandArray[] = { 0x00, 0xFF, 0xA5, 0, 1, 2, 3, 4, 5, 6, 7};

  ESP_ERROR_CHECK(writeI2CRegs(CYPRESS_TOUCH_BASE_ADDR, blCommandArray, sizeof(blCommandArray)));

  vTaskDelay(pdMS_TO_TICKS(500));

  struct cyttspBootloaderData bootloaderData;

  loadBootloaderRegs(&bootloaderData);

  if (TS_GET_BOOTLOADERMODE(bootloaderData.bl_status))
    return ESP_FAIL;

  return ESP_OK;
}

esp_err_t Touch::setSysInfoMode(struct cyttspSysinfoData *sysDataPtr)
{
  sendCommand(CYPRESS_TOUCH_SYSINFO_MODE);

  vTaskDelay(pdMS_TO_TICKS(20));

  uint8_t sysInfoArray[32];

  ESP_ERROR_CHECK(readI2CRegs(CYPRESS_TOUCH_BASE_ADDR, sysInfoArray, sizeof(sysInfoArray)));

  memcpy(sysDataPtr, sysInfoArray, sizeof(sysInfoArray));

  handshake();

  if (!sysDataPtr->tts_verh && !sysDataPtr->tts_verl)
  {
    ESP_LOGE(TAG, "versions");
    return ESP_FAIL;
  }

  return ESP_OK;
}

esp_err_t Touch::setSysInfoRegs(struct cyttspSysinfoData *sysDataPtr)
{
  // Modify registers to the default values.
  sysDataPtr->act_intrvl = CYPRESS_TOUCH_ACT_INTRVL_DFLT;
  sysDataPtr->tch_tmout = CYPRESS_TOUCH_TCH_TMOUT_DFLT;
  sysDataPtr->lp_intrvl = CYPRESS_TOUCH_LP_INTRVL_DFLT;

  // Pack them into array.
  uint8_t regs[] = {sysDataPtr->act_intrvl, sysDataPtr->tch_tmout, sysDataPtr->lp_intrvl};
  // Send the registers to the I2C. Check if failed. If failed, return false.
  esp_err_t err = writeI2CRegs(0x1D, regs, 3);

  // Wait a little bit.
  vTaskDelay(pdMS_TO_TICKS(20));

  // Everything went ok? Return true for success.
  return err;
}

void Touch::handshake()
{
  uint8_t hstModeReg = 0;
  ESP_ERROR_CHECK(readI2CRegs(CYPRESS_TOUCH_BASE_ADDR, &hstModeReg, 1));
  hstModeReg ^= 0x80;
  ESP_ERROR_CHECK(writeI2CRegs(CYPRESS_TOUCH_BASE_ADDR, &hstModeReg, 1));
}

void Touch::end()
{
  if (m_tsInitDone)
  {
    gpio_isr_handler_remove(TOUCHSCREEN_INT);
    gpio_set_intr_type(TOUCHSCREEN_INT, GPIO_INTR_DISABLE);
  }

  tsFlag = false;
  power(false);
  m_tsInitDone = false;
}
