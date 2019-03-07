/****************************************************************************
 *   Written By Marcio Teixeira 2018 - Aleph Objects, Inc.                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#pragma once

#include "../config.h"

#if defined(__MARLIN_FIRMWARE__)
  // Marlin will define the I/O functions for us

  #if ENABLED(EXTENSIBLE_UI)
    #define FTDI_BASIC
    #define FTDI_EXTENDED
  #endif

#else
  #include "Arduino.h"

  #if !defined(CLCD_USE_SOFT_SPI)
    #include "SPI.h"
  #endif

  #if defined(USE_FAST_AVR_IO)
    // Pin definitions are given as a pairing of port and bitmask:
    //
    //    #define CLCD_SPI_CS   G, 0b00001000 // PG3 P1 Pin-3

    #define _PIN_HIGH( port, bit)       PORT##port = (PORT##port |   bit);
    #define _PIN_LOW(  port, bit)       PORT##port = (PORT##port & (~bit));

    #define _SET_INPUT(   port, bit)    DDR##port  = (DDR##port  & (~bit));
    #define _SET_OUTPUT(  port, bit)    DDR##port  = (DDR##port  |   bit);
    #define _READ(  port, bit)          (PIN##port & bit)
    #define _WRITE(  port, bit, value)  {if(value) {_PIN_HIGH(port, bit)} else {_PIN_LOW(port, bit)}}

    // This level of indirection is needed to unpack the "pin" into two arguments
    #define SET_INPUT(pin)               _SET_INPUT(pin)
    #define SET_INPUT_PULLUP(pin)        _SET_INPUT(pin); _PIN_HIGH(pin);
    #define SET_OUTPUT(pin)              _SET_OUTPUT(pin)
    #define READ(pin)                    _READ(pin)
    #define WRITE(pin, value)            _WRITE(pin, value)
  #else
    // Use standard Arduino Wire library
    #include <Wire.h>

    #define SET_OUTPUT(p)             pinMode(p, OUTPUT);
    #define SET_INPUT_PULLUP(p)       pinMode(p, INPUT_PULLUP);
    #define SET_INPUT(p)              pinMode(p, INPUT);
    #define WRITE(p,v)                digitalWrite(p, v ? HIGH : LOW);
    #define READ(p)                   digitalRead(p)
  #endif

  #ifndef pgm_read_word_far
  #define pgm_read_word_far pgm_read_word
  #endif

  #ifndef pgm_read_dword_far
  #define pgm_read_dword_far pgm_read_dword
  #endif

  #ifndef pgm_read_ptr_far
  #define pgm_read_ptr_far pgm_read_ptr
  #endif

  #define SERIAL_ECHO_START()
  #define SERIAL_ECHOLNPGM(str)        Serial.println(F(str))
  #define SERIAL_ECHOPGM(str)          Serial.print(F(str))
  #define SERIAL_ECHOLNPAIR(str, val) {Serial.print(F(str)); Serial.println(val);}
  #define SERIAL_ECHOPAIR(str, val)   {Serial.print(F(str)); Serial.print(val);}

  #define safe_delay delay
#endif // !defined(__MARLIN_FIRMWARE__)