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
// @file LCD1602.h
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
// @author F. Malpartida - fmalpartida@gmail.com
// ---------------------------------------------------------------------------
#ifndef LCD1602_h
#define LCD1602_h

#include <inttypes.h>
#include <Print.h>

#include <SoftwareWire.h>
#include "LCD.h"


// CONSTANT  definitions
// ---------------------------------------------------------------------------

// flags for backlight control
/*!
  @defined
  @abstract   LCD_NOBACKLIGHT
  @discussion NO BACKLIGHT MASK
*/
#define LCD_NOBACKLIGHT 0x00

/*!
  @defined
  @abstract   LCD_BACKLIGHT
  @discussion BACKLIGHT MASK used when backlight is on
*/
#define LCD_BACKLIGHT   0xFF


// Default library configuration parameters used by class constructor with
// only the I2C address field.
// ---------------------------------------------------------------------------
/*!
  @defined
  @abstract   Enable bit of the LCD
  @discussion Defines the IO of the expander connected to the LCD Enable
*/
#define BIT_EN 2  // Enable bit

/*!
  @defined
  @abstract   Read/Write bit of the LCD
  @discussion Defines the IO of the expander connected to the LCD Rw pin
*/
#define BIT_RW 1  // Read/Write bit

/*!
  @defined
  @abstract   Register bit of the LCD
  @discussion Defines the IO of the expander connected to the LCD Register select pin
*/
#define BIT_RS 0  // Register select bit

/*!
  @defined
  @abstract   LCD dataline allocation this library only supports 4 bit LCD control
  mode.
  @discussion D4, D5, D6, D7 LCD data lines pin mapping of the extender module
*/
#define BIT_D4 4
#define BIT_D5 5
#define BIT_D6 6
#define BIT_D7 7

#define BIT_BL 3

#define LCD_ADDR 0x27

class LCD1602 : public LCD {
public:

    /*!
     @method
     @abstract   Class constructor.
     @discussion Initializes class variables and defines the I2C address of the
     LCD. The constructor does not initialize the LCD.

     @param      lcd_Addr[in] I2C address of the IO expansion module. For I2CLCDextraIO,
     the address can be configured using the on board jumpers.
     */
    LCD1602(int pin_sda, int pin_scl, uint8_t lcd_Addr=LCD_ADDR);
    ~LCD1602();

    /*!
     @function
     @abstract   LCD initialization and associated HW.
     @discussion Initializes the LCD to a given size (col, row). This methods
     initializes the LCD, therefore, it MUST be called prior to using any other
     method from this class or parent class.

     The begin method can be overloaded if necessary to initialize any HW that
     is implemented by a library and can't be done during construction, here
     we use the Wire class.

     @param      cols[in] the number of columns that the display has
     @param      rows[in] the number of rows that the display has
     @param      charsize[in] size of the characters of the LCD: LCD_5x8DOTS or
     LCD_5x10DOTS.
     */
    virtual void begin(uint8_t cols=16, uint8_t rows=2, uint8_t charsize = LCD_5x8DOTS);

    /*!
     @function
     @abstract   Send a particular value to the LCD.
     @discussion Sends a particular value to the LCD for writing to the LCD or
     as an LCD command.

     Users should never call this method.

     @param      value[in] Value to send to the LCD.
     @param      mode[in] DATA - write to the LCD CGRAM, COMMAND - write a
     command to the LCD.
     */
    virtual void send(uint8_t value, uint8_t mode);

    /*!
     @function
     @abstract   Switch-on/off the LCD backlight.
     @discussion Switch-on/off the LCD backlight.
     The setBacklightPin has to be called before setting the backlight for
     this method to work. @see setBacklightPin.

     @param      value: backlight mode (HIGH|LOW)
     */
    void setBacklight(uint8_t value);

private:

    /*!
     @method
     @abstract   Initializes the LCD class
     @discussion Initializes the LCD class and IO expansion module.
     */
    int init();

    /*!
     @method
     @abstract   Writes an 4 bit value to the LCD.
     @discussion Writes 4 bits (the least significant) to the LCD control data lines.
     @param      value[in] Value to write to the LCD
     @param      more[in]  Value to distinguish between command and data.
     COMMAND == command, DATA == data.
     */
    void write4bits(uint8_t value, uint8_t mode);


    /*!
     @method
     @abstract   Writes data to the LCD through i2c.
     @discussion Writes data to the LCD through i2c.
     @param      data[in] Value to write to the LCD
     */
    void sendIIC(uint8_t data);

    /*!
     @method
     @abstract   Pulse the LCD enable line (En).
     @discussion Sends a pulse of 1 uS to the Enable pin to execute an command
     or write operation.
     */
    void pulseEnable(uint8_t);


    uint8_t _Addr = LCD_ADDR;             // I2C Address of the IO expander
    SoftwareWire *_i2cio = NULL;            // I2CIO PCF8574* expansion module driver I2CLCDextraIO

    const uint8_t _backlightPinMask = (1 << BIT_BL);
    uint8_t _backlightStsMask = LCD_NOBACKLIGHT;

    const uint8_t _polarity = POSITIVE;

    const uint8_t _En = (1 << BIT_EN);
    const uint8_t _Rw = (1 << BIT_RW);
    const uint8_t _Rs = (1 << BIT_RS);

    // Initialise pin mapping
    const uint8_t _data_pins[4] = {
            1 << BIT_D4,
            1 << BIT_D5,
            1 << BIT_D6,
            1 << BIT_D7
    };
};

#endif
