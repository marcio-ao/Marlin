/*****************
 * resolutions.h *
 *****************/

/****************************************************************************
 *   Written By Mark Pelletier  2019 - Aleph Objects, Inc.                  *
 *   Written By Marcio Teixeira 2019 - Aleph Objects, Inc.                  *
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

#if defined(LCD_320x240)
  namespace FTDI {
    const uint8_t Pclk                 =     8;
    const uint8_t Pclkpol              =     0;
    const uint16_t Hsize               =   320;
    const uint16_t Vsize               =   240;
    const uint16_t Vsync0              =     0;
    const uint16_t Vsync1              =     2;
    const uint16_t Voffset             =    13;
    const uint16_t Vcycle              =   263;
    const uint16_t Hsync0              =     0;
    const uint16_t Hsync1              =    10;
    const uint16_t Hoffset             =    70;
    const uint16_t Hcycle              =   408;

    const uint32_t default_transform_a =  0x000054ad;
    const uint32_t default_transform_b =  0xffffff52;
    const uint32_t default_transform_c =  0xfff7f6e4;
    const uint32_t default_transform_d =  0x00000065;
    const uint32_t default_transform_e =  0xffffbe3b;
    const uint32_t default_transform_f =  0x00f68e75;
  }

#elif defined(LCD_480x272)
  namespace FTDI {
    const uint8_t  Pclk                 =    5;
    const uint8_t  Pclkpol              =    1;
    const uint16_t Hsize                =   480;
    const uint16_t Vsize                =   272;
    const uint16_t Vsync0               =     0;
    const uint16_t Vsync1               =    10;
    const uint16_t Voffset              =    12;
    const uint16_t Vcycle               =   292;
    const uint16_t Hsync0               =     0;
    const uint16_t Hsync1               =    41;
    const uint16_t Hoffset              =    43;
    const uint16_t Hcycle               =   548;

    const uint32_t default_transform_a  =  0x00008100;
    const uint32_t default_transform_b  =  0x00000000;
    const uint32_t default_transform_c  =  0xFFF18000;
    const uint32_t default_transform_d  =  0x00000000;
    const uint32_t default_transform_e  =  0xFFFFB100;
    const uint32_t default_transform_f  =  0x0120D000;
  }

#elif defined(LCD_800x480)
  namespace FTDI {
    const uint8_t Pclk                  =     3;
    const uint8_t Pclkpol               =     1;
    const uint16_t Hsize                =   800;
    const uint16_t Vsize                =   480;
    const uint16_t Vsync0               =     0;
    const uint16_t Vsync1               =     2;
    const uint16_t Voffset              =    13;
    const uint16_t Vcycle               =   525;
    const uint16_t Hsync0               =     0;
    const uint16_t Hsync1               =    20;
    const uint16_t Hoffset              =    64;
    const uint16_t Hcycle               =   952;

    const uint32_t default_transform_a  =  0x0000D8B9;
    const uint32_t default_transform_b  =  0x00000124;
    const uint32_t default_transform_c  =  0xFFE23926;
    const uint32_t default_transform_d  =  0xFFFFFF51;
    const uint32_t default_transform_e  =  0xFFFF7E4F;
    const uint32_t default_transform_f  =  0x01F0AF70;
  }

#else
  #error Unknown or no display resolution specified. To add a display resolution, modify "ftdi_eve_resolutions.h"
#endif