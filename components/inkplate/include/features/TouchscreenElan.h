#pragma once


#include <stdint.h>
#include <string.h>
#include "driver/i2c_master.h"
#include "I2C.h"
#include "PCAL.h"

#define TOUCHSCREEN_EN          IO_NUM_A0
#define TOUCHSCREEN_RST         IO_NUM_A1
#define TOUCHSCREEN_INT         GPIO_NUM_36
#define TOUCHSCREEN_I2C_ADDR    0x15

#define E_INK_WIDTH             600
#define E_INK_HEIGHT            600

#define BOUND(lo, val, hi)      ((val) >= (lo) && (val) <= (hi))

class TouchElan
{
  public:
    esp_err_t begin(I2C &i2c, PCAL &expander, uint8_t powerState);
    void      shutdown();

    bool     available();
    bool     touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h);
    uint8_t  getData(uint16_t *xPos, uint16_t *yPos);
    void     getRawData(uint8_t *b);
    void     setPowerState(uint8_t s);
    uint8_t  getPowerState();
    void     setRotation(uint8_t rotation) { m_rotation = rotation; }

  private:
    const uint8_t hello_packet[4] = {0x55, 0x55, 0x55, 0x55};

    uint8_t  tsWriteRegs(uint8_t addr, const uint8_t *buff, uint8_t size);
    void     tsReadRegs(uint8_t addr, uint8_t *buff, uint8_t size);
    void     tsHardwareReset();
    bool     tsSoftwareReset();
    void     tsGetXY(uint8_t *d, uint16_t *x, uint16_t *y);
    void     tsGetResolution(uint16_t *xRes, uint16_t *yRes);
    void     power(bool enable);
    void     end();

    i2c_master_dev_handle_t m_devHandle     = nullptr;
    PCAL                   *m_expander      = nullptr;

    uint16_t                m_tsXResolution = 0;
    uint16_t                m_tsYResolution = 0;
    uint8_t                 m_rotation      = 0;

    uint8_t                 touchN          = 0;
    uint16_t                touchX[2]       = {0, 0};
    uint16_t                touchY[2]       = {0, 0};
    uint32_t                touchT          = 0;
    bool                    m_tsInitDone    = false;
};