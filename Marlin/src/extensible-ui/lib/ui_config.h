/***************
 * ui_config.h *
 ***************/

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

#ifndef _UI_CONFIG_H_
#define _UI_CONFIG_H_

// If using a pre-configured display for Marlin, select it below. Otherwise,
// select OTHER_DISPLAY to configure a custom display.

#define AO_COLOR_DISPLAY_REV_B
//#define AO_COLOR_DISPLAY_REV_C_EXP1
//#define AO_COLOR_DISPLAY_REV_C_EXP2
//#define OTHER_DISPLAY

#if defined(OTHER_DISPLAY)
    // Define whether an FT800 or FT810+ chip is being used
    #define USE_FTDI_FT800
    //#define USE_FTDI_FT810

    // When specifying pins:
    //   - If compiling Marlin, use Marlin pin numbers.
    //   - If compiling standalone sketch, use Arduino
    //     pin numbers or use USE_FAST_AVR_IO instead
    ///    (see below for documentation).

    // The pins for CS and MOD_RESET (PD) must be chosen.
    #define CLCD_MOD_RESET                      9
    #define CLCD_SPI_CS                        10

    // If using software SPI, specify pins for SCLK, MOSI, MISO
    //#define CLCD_USE_SOFT_SPI
    #if defined(CLCD_USE_SOFT_SPI)
        #define CLCD_SOFT_SPI_MOSI             11
        #define CLCD_SOFT_SPI_MISO             12
        #define CLCD_SOFT_SPI_SCLK             13
    #endif

    // Define the display resolution
    #define LCD_480x272
    //#define LCD_800x480

    // Defines how to orient the display. An inverted (i.e. upside-down) display
    // is supported on the FT800. The FT810 or better also support a portrait
    // and mirrored orientation.
    //#define USE_INVERTED_ORIENTATION
    //#define USE_PORTRAIT_ORIENTATION
    //#define USE_MIRRORED_ORIENTATION

    // If the following is defined, the pin definitions can be
    // given as a pairing of a port and bitmask, as opposed to
    // Arduino pin numbers, for faster sofware based I/O on
    // AVR chips, e.g:
    //
    //   #define CLCD_SPI_CS  G, 0b00001000 // PG3
    //
    //#define USE_FAST_AVR_IO
#endif

#endif // _UI_CONFIG_H_