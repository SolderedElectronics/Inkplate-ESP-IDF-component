#ifndef _PCAL_H_
#define _PCAL_H_

#include "I2C.h"

// PCAL6416 registers
#define PCAL6416A_INPORT0        0x00
#define PCAL6416A_INPORT1        0x01
#define PCAL6416A_OUTPORT0       0x02
#define PCAL6416A_OUTPORT1       0x03
#define PCAL6416A_POLINVPORT0    0x04
#define PCAL6416A_POLINVPORT1    0x05
#define PCAL6416A_CFGPORT0       0x06
#define PCAL6416A_CFGPORT1       0x07
#define PCAL6416A_OUTDRVST_REG00 0x40
#define PCAL6416A_OUTDRVST_REG01 0x41
#define PCAL6416A_OUTDRVST_REG10 0x42
#define PCAL6416A_OUTDRVST_REG11 0x43
#define PCAL6416A_INLAT_REG0     0x44
#define PCAL6416A_INLAT_REG1     0x45
#define PCAL6416A_PUPDEN_REG0    0x46
#define PCAL6416A_PUPDEN_REG1    0x47
#define PCAL6416A_PUPDSEL_REG0   0x48
#define PCAL6416A_PUPDSEL_REG1   0x49
#define PCAL6416A_INTMSK_REG0    0x4A
#define PCAL6416A_INTMSK_REG1    0x4B
#define PCAL6416A_INTSTAT_REG0   0x4C
#define PCAL6416A_INTSTAT_REG1   0x4D
#define PCAL6416A_OUTPORT_CONF   0x4F

// user pins on IO Expander for Inkplate 6COLOR
typedef enum
{
  IO_NUM_A0 = 0,
  IO_NUM_A1,
  IO_NUM_A2,
  IO_NUM_A3,
  IO_NUM_A4,
  IO_NUM_A5,
  IO_NUM_A6,
  IO_NUM_A7,
  IO_NUM_B0,
  IO_NUM_B1,
  IO_NUM_B2,
  IO_NUM_B3,
  IO_NUM_B4,
  IO_NUM_B5,
  IO_NUM_B6,
  IO_NUM_B7
} IOPin_t;

typedef enum
{
  IO_MODE_INPUT = 0,
  IO_MODE_OUTPUT,
} IOMode_t;

typedef enum
{
  IO_PULLUP = 0,
  IO_PULLDOWN,
} IOPullMode_t;

typedef enum
{
  IO_PORT_0 = 0,
  IO_PORT_1,
} IOPort_t;

typedef enum
{
  IO_PUSH_PULL  = 0,
  IO_OPEN_DRAIN,
} IOOutputMode_t;

typedef enum
{
  IO_DRIVE_25  = 0,
  IO_DRIVE_50,
  IO_DRIVE_75,
  IO_DRIVE_100,
} IODriveStrength_t;

class PCAL
{
public:
  PCAL(uint8_t addr, I2C &i2c);

  esp_err_t setLevel(IOPin_t pin, uint8_t level);
  int       getLevel(IOPin_t pin);

  esp_err_t setPort(IOPort_t port, uint8_t value);
  int       getPort(IOPort_t port);

  esp_err_t setPortDirection(IOPort_t port, uint8_t mask);
  esp_err_t setDirection(IOPin_t pin, IOMode_t mode);
  esp_err_t setPullMode(IOPin_t pin, IOPullMode_t pullMode);
  esp_err_t setPolarityInversion(IOPin_t pin, bool invert);
  esp_err_t setInputLatch(IOPin_t pin, bool latch);
  esp_err_t setOutputMode(IOPort_t port, IOOutputMode_t mode);
  esp_err_t setDriveStrength(IOPin_t pin, IODriveStrength_t strength);

  esp_err_t interruptEnable(IOPin_t pin);
  esp_err_t interruptDisable(IOPin_t pin);
  bool      getInterrupt(IOPin_t pin);

  esp_err_t blockPin(IOPin_t pin);
  esp_err_t unblockPin(IOPin_t pin);

private:
  void      pinToRegBit(IOPin_t pin, uint8_t baseReg, uint8_t &reg, uint8_t &bit);
  bool      checkBlockedPins(IOPin_t pin);

  esp_err_t writePin(uint8_t reg, uint8_t val);
  uint8_t   readPin(uint8_t reg);

  uint16_t                m_blockedPins;
  i2c_master_dev_handle_t m_devHandle;
};

#endif
