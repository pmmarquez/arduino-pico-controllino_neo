/*
 * Copyright (c) 2023 CONTROLLINO GmbH.
 *
 * SPDX-License-Identifier: MIT
 */
 
#include "neo_pin.h"
#include "string.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
 
cy8c95xx_t neo_cy8c9520;
mcp356x_t neo_mcp3564;
ad56x4_t neo_ad5664;
 
#define NEO_AI_CS      14
void mcp356x_cs_select(mcp356x_t* dac) { cy8c95xx_write_pin(&neo_cy8c9520, NEO_AI_CS, 0); }
void mcp356x_cs_deselect(mcp356x_t* dac) { cy8c95xx_write_pin(&neo_cy8c9520, NEO_AI_CS, 1); }
 
#define NEO_AO_CS       15
void ad56x4_cs_select(ad56x4_t* dac) { cy8c95xx_write_pin(&neo_cy8c9520, NEO_AO_CS, 0); }
void ad56x4_cs_deselect(ad56x4_t* dac) { cy8c95xx_write_pin(&neo_cy8c9520, NEO_AO_CS, 1); }

/* Init internal peripherals */
void initVariant()
{
  cy8c95xx_cfg_t cy8c95xx_cfg;
  cy8c95xx_set_default_cfg(&cy8c95xx_cfg);
  cy8c95xx_cfg.sda_pin = 4; /* RP2040 GPIO 4 (PIN_WIRE0_SDA)*/
  cy8c95xx_cfg.scl_pin = 5; /* RP2040 GPIO 5 (PIN_WIRE0_SCL)*/
  cy8c95xx_cfg.i2c_speed = 100000;
  cy8c95xx_cfg.i2c = i2c0; /* WIRE0 */
  cy8c95xx_cfg.int_pin = 15; /* RP2040 GPIO 15*/
  cy8c95xx_init(&neo_cy8c9520, &cy8c95xx_cfg);
  
  mcp356x_cfg_t mcp356x_cfg;
  mcp356x_set_default_cfg(&mcp356x_cfg);
  mcp356x_cfg.mosi_pin = 19; /* RP2040 GPIO 19 (PIN_SPI0_MOSI)*/
  mcp356x_cfg.miso_pin = 16; /* RP2040 GPIO 16 (PIN_SPI0_MISO)*/
  mcp356x_cfg.sck_pin = 18; /* RP2040 GPIO 18 (PIN_SPI0_SCK)*/
  mcp356x_cfg.spi_speed = 1000000;
  mcp356x_cfg.spi = spi0; /* SPI0 */
  mcp356x_init(&neo_mcp3564, &mcp356x_cfg);
  
  ad56x4_cfg_t ad56x4_cfg;
  ad56x4_set_default_cfg(&ad56x4_cfg);
  ad56x4_cfg.mosi_pin = 19; /* RP2040 GPIO 19 (PIN_SPI0_MOSI)*/
  ad56x4_cfg.miso_pin = 16; /* RP2040 GPIO 16 (PIN_SPI0_MISO)*/
  ad56x4_cfg.sck_pin = 18; /* RP2040 GPIO 18 (PIN_SPI0_SCK)*/
  ad56x4_cfg.spi_speed = 1000000;
  ad56x4_cfg.spi = spi0; /* SPI0 */
  ad56x4_init(&neo_ad5664, &ad56x4_cfg);
}
 
/* Arduino API functions compatible with CONTROLLINO NEO */
void pinMode(ControllinoNeoPin pin, PinMode mode)
{
  switch (pin.getType())
  {
  case ControllinoNeoPin::NATIVE_PIN:
    pinMode(pin.getPin(), mode);
    break;
  case ControllinoNeoPin::CY8C95XX_PIN:
    cy8c95xx_dir_mode_t dir;
    cy8c95xx_drv_mode_t drv;
    switch (mode)
    {
    case INPUT_PULLDOWN:
      dir = CY8C95XX_GPIO_IN;
      drv = CY8C95XX_DRV_PULL_DOWN;
      break;
    case OUTPUT:
    case OUTPUT_4MA:
    case OUTPUT_2MA:
    case OUTPUT_8MA:
    case OUTPUT_12MA:
      dir = CY8C95XX_GPIO_OUT;
      drv = CY8C95XX_DRV_STRONG;
      break;
    default:
      dir = CY8C95XX_GPIO_IN;
      drv = CY8C95XX_DRV_PULL_UP;
      break;
    }
    cy8c95xx_pin_mode(&neo_cy8c9520, (int)pin.getPin(), dir, drv);
    // Pending disable pwm by default
    break;
  default:
    // Other pin types has fixed modes
    break;
  }
  pin.setMode(mode);
}
 
PinStatus digitalRead(ControllinoNeoPin pin)
{
  PinStatus pinStatus = CHANGE;
  switch (pin.getType())
  {
  case ControllinoNeoPin::NATIVE_PIN:
    pinStatus = digitalRead(pin.getPin());
    break;
  case ControllinoNeoPin::CY8C95XX_PIN:
    uint8_t pinState;
    switch (pin.getMode())
    {
    case OUTPUT:
    case OUTPUT_4MA:
    case OUTPUT_2MA:
    case OUTPUT_8MA:
    case OUTPUT_12MA:
      cy8c95xx_read_pin_out_lvl(&neo_cy8c9520, (int)pin.getPin(), &pinState);
      break;
    default:
      cy8c95xx_read_pin(&neo_cy8c9520, (int)pin.getPin(), &pinState);
      break;
    }
    pinStatus = pinState ? HIGH : LOW;
    break;
  default:
    // Other pin types are analog only
    break;
  }
  return pinStatus;
}
 
void digitalWrite(ControllinoNeoPin pin, PinStatus value)
{
  switch (pin.getType())
  {
  case ControllinoNeoPin::NATIVE_PIN:
    digitalWrite(pin.getPin(), value);
    break;
  case ControllinoNeoPin::CY8C95XX_PIN:
    cy8c95xx_write_pin(&neo_cy8c9520, (int)pin.getPin(), (uint8_t)value);
    break;
  default:
    // Other pin types are analog only
    break;
  }
}
 
int analogRead(ControllinoNeoPin pin)
{
  int adcValue = 0;
  switch (pin.getType())
  {
  case ControllinoNeoPin::NATIVE_PIN:
    adcValue = analogRead(pin.getPin());
    break;
  case ControllinoNeoPin::MCP356X_PIN:
    uint8_t txdata[7];
    uint8_t dummySgn;
    uint32_t dummyMaxResolution;
    memset(txdata, 0x00, sizeof(txdata));
    txdata[0] = MCP356X_MUX_VIN_NEG_VREF_EXT_MINUS;
    switch (pin.getPin())
    {
    case MCP356X_CH_CH1:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH1;
      break;
    case MCP356X_CH_CH2:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH2;
      break;
    case MCP356X_CH_CH3:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH4;
      break;
    case MCP356X_CH_CH5:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH5;
      break;
    case MCP356X_CH_CH6:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH6;
      break;
    case MCP356X_CH_CH7:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH7;
      break;
    default:
      txdata[0] |= MCP356X_MUX_VIN_POS_CH0;
      break;
    }
    mcp356x_iwrite(&neo_mcp3564, MCP356X_REG_MUX, txdata, sizeof(txdata));
    mcp356x_read_raw_adc(&neo_mcp3564, (uint32_t*)&adcValue, &dummySgn, &dummyMaxResolution);
    break;
  default:
    // Other pin types are digital or analog output only
    break;
  }
  return adcValue;
}
 
void analogWrite(ControllinoNeoPin pin, int value)
{
  switch (pin.getType())
  {
  case ControllinoNeoPin::NATIVE_PIN:
    analogWrite(pin.getPin(), value);
    break;
  case ControllinoNeoPin::CY8C95XX_PIN:
    // Missing 
    break;
  case ControllinoNeoPin::AD56X4_PIN:
    ad56x4_write_input_reg(&neo_ad5664, (uint8_t)pin.getPin(), ((uint16_t)value & 0xFFFF)); // 16 bits resolution
    ad56x4_update_dac_reg(&neo_ad5664, (uint8_t)pin.getPin());
    break;
  default:
    // Other pin types are analog only
    break;
  }
}

