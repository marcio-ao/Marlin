/*********************
 * ui_conditionals.h *
 *********************/

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

#ifndef _UI_CONDITIONALS_H_
#define _UI_CONDITIONALS_H_

#if defined(_MARLIN_CONFIG_H_)
    // If _MARLIN_CONFIG_H_ exists, then we are being
    // compiled inside Marlin.
    #define USE_MARLIN_IO
#else
    #include "Arduino.h"

    // Load up compatibility routines
    #define EXTENSIBLE_UI
    #define _CAT(a, ...) a ## __VA_ARGS__
    #define SWITCH_ENABLED_      1
    #define ENABLED(b) _CAT(SWITCH_ENABLED_, b)
    #define DISABLED(b) !ENABLED(b)

    // Messages that are declared in Marlin
    #define WELCOME_MSG     "Printer Ready"
    #define MSG_SD_INSERTED "Media Inserted"
    #define MSG_SD_REMOVED  "Media Removed"
#endif

// The AlephObjects Rev B Color Display is display for Marlin
// that connects to EXP1 and uses software SPI

#if defined(AO_COLOR_DISPLAY_REV_B)
    #ifndef USE_MARLIN_IO
        #error This display configuration cannot be used outside of Marlin.
    #endif

    #define USE_FTDI_FT810
    //#define LCD_800x480
    #define LCD_480x272
    //#define USE_INVERTED_ORIENTATION
    //#define USE_PORTRAIT_ORIENTATION

    #define CLCD_MOD_RESET                 LCD_PINS_D4
    #define CLCD_SPI_CS                    LCD_PINS_D5

    #define CLCD_AUX_0                     LCD_PINS_ENABLE
    #define CLCD_AUX_1                     BTN_ENC
    #define CLCD_AUX_2                     BEEPER_PIN

    #define CLCD_USE_SOFT_SPI
    #define CLCD_SOFT_SPI_SCLK             LCD_PINS_D7
    #define CLCD_SOFT_SPI_MOSI             LCD_PINS_D6
    #define CLCD_SOFT_SPI_MISO             LCD_PINS_RS
#endif

// The AlephObjects Rev C Color Display is a display for Marlin
// that can connect to EXP1 for software SPI, or EXP2 for
// hardware SPI

#if defined(AO_COLOR_DISPLAY_REV_C_EXP1)
    #ifndef USE_MARLIN_IO
        #error This display configuration cannot be used outside of Marlin.
    #endif

    #define USE_FTDI_FT810
    #define LCD_800x480
    #define USE_INVERTED_ORIENTATION
    #define USE_PORTRAIT_ORIENTATION

    #define CLCD_MOD_RESET                 LCD_PINS_ENABLE
    #define CLCD_SPI_CS                    LCD_PINS_D4

    #define CLCD_USE_SOFT_SPI
    #define CLCD_SOFT_SPI_SCLK             BTN_ENC
    #define CLCD_SOFT_SPI_MOSI             LCD_PINS_D5
    #define CLCD_SOFT_SPI_MISO             BEEPER_PIN
#endif

#if defined(AO_COLOR_DISPLAY_REV_C_EXP2)
    #ifndef USE_MARLIN_IO
        #error This display configuration cannot be used outside of Marlin.
    #endif

    #define USE_FTDI_FT810
    #define LCD_800x480
    #define USE_INVERTED_ORIENTATION
    #define USE_PORTRAIT_ORIENTATION

    #define CLCD_SPI_CS                    BTN_EN1
    #define CLCD_MOD_RESET                 BTN_EN2
#endif

#endif // _UI_CONDITIONALS_H_