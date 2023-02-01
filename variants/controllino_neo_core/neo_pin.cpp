/*
 * Copyright (c) 2023 CONTROLLINO GmbH.
 *
 * SPDX-License-Identifier: MIT
 */

#include "neo_pin.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"

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
  cy8c95xx_cfg.i2c = (hw_i2c_t*)i2c0; /* WIRE0 */
  cy8c95xx_init(&neo_cy8c9520, &cy8c95xx_cfg);
  
  mcp356x_cfg_t mcp356x_cfg;
  mcp356x_set_default_cfg(&mcp356x_cfg);
  mcp356x_cfg.mosi_pin = 19; /* RP2040 GPIO 19 (PIN_SPI0_MOSI)*/
  mcp356x_cfg.miso_pin = 16; /* RP2040 GPIO 16 (PIN_SPI0_MISO)*/
  mcp356x_cfg.sck_pin = 18; /* RP2040 GPIO 18 (PIN_SPI0_SCK)*/
  mcp356x_cfg.spi_speed = 1000000;
  mcp356x_cfg.spi = (hw_i2c_t*)spi0; /* SPI0 */
  mcp356x_init(&neo_mcp3564, &mcp356x_cfg);
  
  ad56x4_cfg_t ad56x4_cfg;
  ad56x4_set_default_cfg(&ad56x4_cfg);
  ad56x4_cfg.mosi_pin = 19; /* RP2040 GPIO 19 (PIN_SPI0_MOSI)*/
  ad56x4_cfg.miso_pin = 16; /* RP2040 GPIO 16 (PIN_SPI0_MISO)*/
  ad56x4_cfg.sck_pin = 18; /* RP2040 GPIO 18 (PIN_SPI0_SCK)*/
  ad56x4_cfg.spi_speed = 1000000;
  ad56x4_cfg.spi = (hw_i2c_t*)spi0; /* SPI0 */
  ad56x4_init(&neo_ad5664, &ad56x4_cfg);
}

/* Arduino API functions compatible with CONTROLLINO NEO */
void pinMode(ControllinoNeoPin pin, PinMode mode)
{
  WiFiDrv::pinMode(VAL(pin), static_cast<uint8_t>(mode));
}

PinStatus digitalRead(ControllinoNeoPin pin)
{
  return WiFiDrv::digitalRead(VAL(pin));
}

void digitalWrite(ControllinoNeoPin pin, PinStatus value)
{
  if (value == LOW)
    WiFiDrv::digitalWrite(VAL(pin), 1);
  else
    WiFiDrv::digitalWrite(VAL(pin), 0);
}

int analogRead(ControllinoNeoPin pin)
{
  uint8_t const adc_channel = toAnalogPin(pin);

  if (adc_channel == 0xFF)
    return 0;
  else
#ifdef NINA_PINS_AS_CLASS
    return WiFiDrv::analogRead(adc_channel) >> (12 - pin.analogReadResolution());
#else
    return WiFiDrv::analogRead(adc_channel);
#endif
}

void analogWrite(ControllinoNeoPin pin, int value)
{
  WiFiDrv::analogWrite(VAL(pin), static_cast<uint8_t>(value));
}

