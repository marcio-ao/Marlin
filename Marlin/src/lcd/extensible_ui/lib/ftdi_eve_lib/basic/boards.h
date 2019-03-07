/************
 * boards.h *
 ************/

/****************************************************************************
 *   Written By Mark Pelletier  2017 - Aleph Objects, Inc.                  *
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

#define HAS_RESOLUTION defined(LCD_320x240) || defined(LCD_480x272) || defined(LCD_800x480)

#if defined(LCD_FTDI_VM800B35A)
  #if !(HAS_RESOLUTION)
    #define LCD_320x240
  #endif
  #define USE_FTDI_FT800
  namespace FTDI {
    const bool Use_Crystal             =  true;  // 0 = use internal oscillator, 1 = module has a crystal populated
    const bool GPIO_0_Audio_Enable     =  false; /* 1 = does use GPIO00 for amplifier control, 0 = not in use for Audio */
    const bool GPIO_1_Audio_Shutdown   =  true;  /* 1 = does use GPIO01 for amplifier control, 0 = not in use for Audio */
    const uint8_t Swizzle              =     2;
    const uint8_t CSpread              =     1;

    const uint16_t touch_threshold     =  1200; /* touch-sensitivity */

  }

/*
 * Settings for the Haoyu Electronics, 4.3" Graphical LCD Touchscreen,       480x272, SPI, FT800 (FT800CB-HY43B)
 *                  Haoyu Electronics,   5" Graphical LCD Touchscreen,       480x272, SPI, FT800 (FT800CB-HY50B)
 *
 *    http://www.hotmcu.com/43-graphical-lcd-touchscreen-480x272-spi-ft800-p-111.html?cPath=6_16
 *    http://www.hotmcu.com/5-graphical-lcd-touchscreen-480x272-spi-ft800-p-124.html?cPath=6_16
 *
 * Datasheet:
 *
 *    http://www.hantronix.com/files/data/1278363262430-3.pdf
 *    http://www.haoyuelectronics.com/Attachment/HY43-LCD/LCD%20DataSheet.pdf
 *    http://www.haoyuelectronics.com/Attachment/HY5-LCD-HD/KD50G21-40NT-A1.pdf
 *
 */

#elif defined(LCD_HAOYU_FT800CB)
  #if !(HAS_RESOLUTION)
    #define LCD_480x272
  #endif
  #define USE_FTDI_FT800
  namespace FTDI {
    const bool Use_Crystal              =  true; // 0 = use internal oscillator, 1 = module has a crystal populated
    const bool GPIO_0_Audio_Enable      = false;
    const bool GPIO_1_Audio_Shutdown    = false;
    const uint8_t Swizzle               =     0;
    const uint8_t CSpread               =     1;
    const uint16_t touch_threshold      =  2000; /* touch-sensitivity */
  }

/*
 * Settings for the Haoyu Electronics, 5" Graphical LCD Touchscreen, 800x480, SPI, FT810
 *
 *    http://www.hotmcu.com/5-graphical-lcd-touchscreen-800x480-spi-ft810-p-286.html
 *
 * Datasheet:
 *
 *    http://www.haoyuelectronics.com/Attachment/HY5-LCD-HD/KD50G21-40NT-A1.pdf
 *
 */

#elif defined(LCD_HAOYU_FT810CB)
  #if !(HAS_RESOLUTION)
    #define LCD_800x480
  #endif
  #define USE_FTDI_FT810
  namespace FTDI {
    const bool Use_Crystal              =  true; // 0 = use internal oscillator, 1 = module has a crystal populated
    const bool GPIO_0_Audio_Enable      = false;
    const bool GPIO_1_Audio_Shutdown    = false;
    const uint8_t Swizzle               =     0;
    const uint8_t CSpread               =     1;
    const uint16_t touch_threshold      =  2000; /* touch-sensitivity */
  }

/*
 * Settings for the 4D Systems,        4.3" Embedded SPI Display             480x272, SPI, FT800 (4DLCD-FT843)
 *
 *    http://www.4dsystems.com.au/product/4DLCD_FT843/
 *
 * Datasheet:
 *
 *    http://www.4dsystems.com.au/productpages/4DLCD-FT843/downloads/FT843-4.3-Display_datasheet_R_1_2.pdf
 *
 */

#elif defined(LCD_4DSYSTEMS_4DLCD_FT843)
  #define USE_FTDI_FT800
  #define LCD_480x272

  namespace FTDI {
    const bool Use_Crystal              =  true; // 0 = use internal oscillator, 1 = module has a crystal populated
    const bool GPIO_0_Audio_Enable      = false;
    const bool GPIO_1_Audio_Shutdown    =  true;
    const uint8_t Swizzle               =     0;
    const uint8_t CSpread               =     1;
    const uint16_t touch_threshold      =  1200; /* touch-sensitivity */
  }

/*
 * Settings for the Aleph Objects Color LCD User Interface
 *
 *    https://code.alephobjects.com/source/aotctl/
 *
 * Datasheet:
 *
 *    http://www.hantronix.com/files/data/s1501799605s500-gh7.pdf
 *
 */
#elif defined(LCD_ALEPHOBJECTS_CLCD_UI)
  #if !(HAS_RESOLUTION)
    #define LCD_800x480
  #endif
  #define USE_FTDI_FT810
  namespace FTDI {
    const bool Use_Crystal              = false; // 0 = use internal oscillator, 1 = module has a crystal populated
    const bool GPIO_0_Audio_Enable      = true;  // The AO CLCD uses GPIO0 to enable audio
    const bool GPIO_1_Audio_Shutdown    = false;
    const uint8_t Swizzle               =     0;
    const uint8_t CSpread               =     0;
    const uint16_t touch_threshold      =  2000; /* touch-sensitivity */
  }

#else

  #error Unknown or no board specified. To add a new board, modify "ftdi_eve_boards.h"
#endif