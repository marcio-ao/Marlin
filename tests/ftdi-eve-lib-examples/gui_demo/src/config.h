/************
 * config.h *
 ************/

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

// Define the display board used (see "ftdi_eve_boards.h" for definitions)

//#define LCD_FTDI_VM800B35A        // FTDI 3.5" 320x240 with FT800
//#define LCD_4DSYSTEMS_4DLCD_FT843 // 4D Systems 4.3" 480x272
//#define LCD_HAOYU_FT800CB         // Haoyu with 4.3" or 5" 480x272
//#define LCD_HAOYU_FT810CB         // Haoyu with 5" 800x480
#define LCD_ALEPHOBJECTS_CLCD_UI  // Aleph Objects Color LCD User Interface

// Leave the following commented out to use a board's default resolution.
// If you have changed the LCD panel, you may override the resolution
// below (see "ftdi_eve_resolutions.h" for definitions):

//#define LCD_320x240
#define LCD_480x272
//#define LCD_800x480

// Select interfacing pins

// The pins for CS and MOD_RESET (PD) must be chosen.
    #define CLCD_MOD_RESET      D, 0b00001000 // LCD_PINS_ENABLE, Marlin Logical Pin 18, D3
    #define CLCD_SPI_CS         D, 0b00000100 // LCD_PINS_D4, Marlin Logical Pin 19, D2

// If using software SPI, specify pins for SCLK, MOSI, MISO
#define CLCD_USE_SOFT_SPI
#if defined(CLCD_USE_SOFT_SPI)
    #define CLCD_SOFT_SPI_SCLK  H, 0b01000000 // BTN_ENC, Marlin Logical Pin 9, H6
    #define CLCD_SOFT_SPI_MOSI  G, 0b00010000 // LCD_PINS_D5, Marlin Logical Pin 70, G4
    #define CLCD_SOFT_SPI_MISO  H, 0b00000100 // BEEPER_PIN, Marlin Logical Pin 84, H2
#endif

// If the following is defined, the pin definitions can be
// given as a pairing of a port and bitmask, as opposed to
// Arduino pin numbers, for faster sofware based I/O on
// AVR chips, e.g:
//
//   #define CLCD_SPI_CS  G, 0b00001000 // PG3
//
#define USE_FAST_AVR_IO

// Defines how to orient the display. An inverted (i.e. upside-down) display
// is supported on the FT800. The FT810 or better also support a portrait
// and mirrored orientation.
//#define USE_INVERTED_ORIENTATION
#define USE_PORTRAIT_ORIENTATION
//#define USE_MIRRORED_ORIENTATION

// Enable this to debug the event framework
#define UI_FRAMEWORK_DEBUG