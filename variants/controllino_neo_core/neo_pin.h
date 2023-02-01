/*
 * Copyright (c) 2023 CONTROLLINO GmbH.
 *
 * SPDX-License-Identifier: MIT
 */
 
#ifndef _NEO_PIN_
#define _NEO_PIN_
 
/**
 * \file neo_pin.h
 *
 * Arduino pins API for NEO boards
 * Based on https://github.com/arduino/ArduinoCore-mbed/tree/master/variants/NANO_RP2040_CONNECT
 */
 
#include "Arduino.h"
#include "cy8c95xx.h"
#include "mcp356x.h"
#include "ad56x4.h"

/**
 * \brief Class to diferenciate CONTROLLINO NEO pins
 * 
 */
class ControllinoNeoPin {
public:
	typedef enum {
		NATIVE_PIN,
		CY8C95XX_PIN,
		MCP356X_PIN,
		AD56X4_PIN
	} _pin_type_t;
	ControllinoNeoPin(int pin, _pin_type_t type): _pin(pin), _type(type) {};
	int getPin() { return _pin; };
	int getType() { return _type; };
	bool operator== (ControllinoNeoPin const& other) const { return _pin == other._pin; };
	__attribute__((error("Change to a #define"))) operator int();
private:
	int _pin;
	_pin_type_t _type;
};

/**
 *\brief Arduino API functions for compatibility with CONTROLLINO NEO
 * 
 */
void PinMode(ControllinoNeoPin pin, PinMode mode);
PinStatus digitalRead(ControllinoNeoPin pin);
void digitalWrite(ControllinoNeoPin pin, PinStatus value);
int analogRead(ControllinoNeoPin pin);
void analogWrite(ControllinoNeoPin pin, int value);

/**
 *\brief Public instances of internal peripherals drivers
 * 
 */
cy8c95xx_t neo_cy8c9520; 
mcp356x_t neo_mcp3564;
ad56x4_t neo_ad5664;

#endif /* _NEO_PIN_ */