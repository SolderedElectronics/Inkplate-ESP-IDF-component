#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include <string.h>
#include "esp_attr.h"
#include "PCAL.h"

#include "TouchCypressTypedefs.h"

#define TOUCHSCREEN_I2C_ADDR          0x24

#define CYPRESS_TOUCH_BASE_ADDR       0x00
#define CYPRESS_TOUCH_SOFT_RST_MODE   0x01
#define CYPRESS_TOUCH_SYSINFO_MODE    0x10
#define CYPRESS_TOUCH_OPERATE_MODE    0x00
#define CYPRESS_TOUCH_LOW_POWER_MODE  0x04
#define CYPRESS_TOUCH_DEEP_SLEEP_MODE 0x02
#define CYPRESS_TOUCH_REG_ACT_INTRVL  0x1D

#define TOUCHSCREEN_EN                IO_NUM_B4
#define TOUCHSCREEN_RST               IO_NUM_B2
#define TOUCHSCREEN_INT               GPIO_NUM_36
#define TOUCHSCREEN_IO_EXPANDER       IO_INT_ADDR
#define TOUCHSCREEN_IO_REGS           ioRegsInt

#define CYPRESS_TOUCH_ACT_INTRVL_DFLT 0x00
#define CYPRESS_TOUCH_LP_INTRVL_DFLT  0x0A
#define CYPRESS_TOUCH_TCH_TMOUT_DFLT  0xFF

#define CYPRESS_TOUCH_MAX_X           682
#define CYPRESS_TOUCH_MAX_Y           1023

#define E_INK_WIDTH  1024
#define E_INK_HEIGHT 758

class Touch
{
public:
  Touch() = default;

  esp_err_t begin(I2C &i2c, PCAL &expander, uint8_t powerState);
  bool      touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h);
  void      shutdown();
  bool      available();
  void      setPowerState(uint8_t state);
  uint8_t   getPowerState();
  uint8_t   getData(uint16_t *xPos, uint16_t *yPos, uint8_t *z = NULL);
  bool      ping(int retries = 5);
  
  void      getRawData(uint8_t *b);
  void      handshake();

private:
  void      power(bool power);
  bool      getTouchData(struct cypressTouchData *touchData);
  void      scale(struct cypressTouchData *touchData, uint16_t xSize, uint16_t ySize, bool flipX, bool flipY, bool swapXY);
  void      end();
  void      reset();
  void      swReset();
  esp_err_t loadBootloaderRegs(struct cyttspBootloaderData *blDataPtr);
  esp_err_t exitBootloaderMode();
  esp_err_t setSysInfoMode(struct cyttspSysinfoData *sysDataPtr);
  esp_err_t setSysInfoRegs(struct cyttspSysinfoData *sysDataPtr);
  esp_err_t sendCommand(uint8_t cmd);
  esp_err_t readI2CRegs(uint8_t cmd, uint8_t *buffer, int len);
  esp_err_t writeI2CRegs(uint8_t cmd, uint8_t *buffer, int len);

  PCAL                       *m_expander  = nullptr;
  i2c_master_dev_handle_t     m_devHandle = NULL;


  struct cyttspBootloaderData m_blData;
  struct cyttspSysinfoData    m_sysData;

  uint8_t                     touchN;
  uint16_t                    touchX[2], touchY[2];
  uint32_t                    touchT = 0;
  bool                        m_tsInitDone = false;
};

#endif
