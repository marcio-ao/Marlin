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

#include "../ui_api.h"

// Select which display you are using.
//#define AO_COLOR_DISPLAY_REV_B
//#define AO_COLOR_DISPLAY_REV_C_EXP1
//#define AO_COLOR_DISPLAY_REV_C_EXP2
#define CR10_TFT

#if defined(AO_COLOR_DISPLAY_REV_B)
    // The AlephObjects Rev B Color Display must connect
    // to EXP1 and can only use software SPI
    #define CLCD_USE_SOFT_SPI
    #define CLCD_MOD_RESET                   LCD_PINS_D4
    #define CLCD_SPI_CS                      LCD_PINS_D5
    #define CLCD_SOFT_SPI_SCLK               LCD_PINS_D7
    #define CLCD_SOFT_SPI_MOSI               LCD_PINS_D6
    #define CLCD_SOFT_SPI_MISO               LCD_PINS_RS
    #define CLCD_AUX_0                       LCD_PINS_ENABLE
    #define CLCD_AUX_1                       BTN_ENC
    #define CLCD_AUX_2                       BEEPER_PIN
#endif

// The AlephObjects Rev C Color Display can connect
// to EXP1 for software SPI, or EXP2 for hardware SPI

#if defined(AO_COLOR_DISPLAY_REV_C_EXP1)
    #define CLCD_USE_SOFT_SPI
    #define CLCD_MOD_RESET                 LCD_PINS_ENABLE
    #define CLCD_SPI_CS                    LCD_PINS_D4
    #define CLCD_SOFT_SPI_SCLK             BTN_ENC
    #define CLCD_SOFT_SPI_MOSI             LCD_PINS_D5
    #define CLCD_SOFT_SPI_MISO             BEEPER_PIN
#endif

#if defined(AO_COLOR_DISPLAY_REV_C_EXP2)
    #define CLCD_SPI_CS                    BTN_EN1
    #define CLCD_MOD_RESET                 BTN_EN2
#endif

#if defined(CR10_TFT)
    #define CLCD_USE_SOFT_SPI
    #define CLCD_SOFT_SPI_SCLK             LCD_PINS_D4      // PORTA1               Pin 6
    #define CLCD_SOFT_SPI_MOSI             LCD_PINS_ENABLE  // PORTC1               Pin 8
    #define CLCD_SPI_CS               LCD_PINS_RS      // PORTA3               Pin 7
    #define CLCD_SOFT_SPI_MISO             16               // PORTC0   BTN_ENC     Pin 2
    #define CLCD_MOD_RESET                 11               // PORTD3   BTN_EN1     Pin 3
    #define CLCD_AUX_0                     10               // PORTD2   BTN_EN2     Pin 5
    #define CLCD_AUX_1                     BEEPER_PIN       // PORTA4               Pin 1
//    #define CLCD_AUX_2                     BEEPER_PIN
#endif

// Define whether an FT800 or FT810+ chip is being used
//#define USE_FTDI_FT800
#define USE_FTDI_FT810

// Defines how to orient the display. An inverted (i.e. upside-down) display
// is supported on the FT800. The FT810 or better also support a portrait
// and mirrored orientation.
// #define USE_INVERTED_ORIENTATION
// #define USE_PORTRAIT_ORIENTATION
//#define USE_MIRRORED_ORIENTATION

// Define the display resolution
#define LCD_480x272
//#define LCD_800x480

#endif // _UI_CONFIG_H_