// ---------------------------------------------------------------------------
// Created by Francisco Malpartida on 20/08/11.
// Copyright (C) - 2018
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License v3.0
//    along with this program.
//    If not, see <https://www.gnu.org/licenses/gpl-3.0.en.html>.
//
// ---------------------------------------------------------------------------
//
// Thread Safe: No
// Extendable: Yes
//
// @file LCD1602.c
// This file implements a basic liquid crystal library that comes as standard
// in the Arduino SDK but using an I2C IO extension board.
//
// @brief
// This is a basic implementation of the LiquidCrystal library of the
// Arduino SDK. The original library has been reworked in such a way that
// this class implements the all methods to command an LCD based
// on the Hitachi HD44780 and compatible chipsets using I2C extension
// backpacks such as the I2CLCDextraIO with the PCF8574* I2C IO Expander ASIC.
//
// The functionality provided by this class and its base class is identical
// to the original functionality of the Arduino LiquidCrystal library.
//
//
//
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#include <Arduino.h>

#include <inttypes.h>
#include <SoftwareWire.h>
#include "LCD1602.h"


// CONSTRUCTORS
// ---------------------------------------------------------------------------
LCD1602::LCD1602(int pin_sda, int pin_scl, uint8_t lcd_Addr) {
    _Addr = lcd_Addr;
    _i2cio = new SoftwareWire(pin_sda, pin_scl);
}

LCD1602::~LCD1602() {
    delete _i2cio;
};

void LCD1602::sendIIC(uint8_t data) {
    _i2cio->beginTransmission(_Addr);
    _i2cio->write(data);
    _i2cio->endTransmission();
}

// PUBLIC METHODS
// ---------------------------------------------------------------------------

//
// begin
void LCD1602::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
    init();     // Initialise the I2C expander interface
    LCD::begin(cols, lines, dotsize);
}

//
// setBacklight
void LCD1602::setBacklight(uint8_t value) {
    // Check for polarity to configure mask accordingly
    // ----------------------------------------------------------
    _backlightStsMask = _backlightPinMask & (value ? LCD_BACKLIGHT : LCD_NOBACKLIGHT);
    sendIIC(_backlightStsMask);
}


// PRIVATE METHODS
// ---------------------------------------------------------------------------

//
// init
int LCD1602::init() {
    int status = 0;

    // initialize the backpack IO expander
    // and display functions.
    // ------------------------------------------------------------------------
    _i2cio->begin();
    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    status = 1;

    sendIIC(0);

    return (status);
}


// low level data pushing commands
//----------------------------------------------------------------------------

//
// send - write either command or data
void LCD1602::send(uint8_t value, uint8_t mode) {
    // No need to use the delay routines since the time taken to write takes
    // longer that what is needed both for toggling and enable pin an to execute
    // the command.

    if (mode == FOUR_BITS) {
        write4bits((value & 0x0F), COMMAND);
    } else {
        write4bits((value >> 4), mode);
        write4bits((value & 0x0F), mode);
    }
}

//
// write4bits
void LCD1602::write4bits(uint8_t value, uint8_t mode) {
    uint8_t pinMapValue = 0;

    // Map the value to LCD pin mapping
    // --------------------------------
    for (uint8_t i = 0; i < 4; i++) {
        if ((value & 0x1) == 1) {
            pinMapValue |= _data_pins[i];
        }
        value = (value >> 1);
    }

    // Is it a command or data
    // -----------------------
    if (mode == LCD_DATA) {
        mode = _Rs;
    }

    pinMapValue |= mode | _backlightStsMask;
    pulseEnable(pinMapValue);
}

//
// pulseEnable
void LCD1602::pulseEnable(uint8_t data) {
    sendIIC(data | _En);   // En HIGH
    sendIIC(data & ~_En);  // En LOW
}
