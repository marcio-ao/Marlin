/***************
 * constants.h *
 ***************/

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

/****************************************************************************
 * This header defines registers and commands for the FTDI FT810 LCD Driver *
 * chip.                                                                    *
 ****************************************************************************/

/*******************************************************************************
 * FT810                                                                       *
 *                                                                             *
 * START    END ADDR   SIZE    NAME           DESCRIPTION                      *
 *                                                                             *
 * 0x000000 0x0FFFFF  1024 kB  RAM_G          Main Graphics RAM (0 to 1048572) *
 *                                                                             *
 * 0x0C0000 0x0C0003     4  B  ROM_CHIPID     [0:1] 0x800                      *
 *                                            [1:2] 0x0100  Chip Id, Vers ID   *
 *                                                                             *
 * 0x1E0000 0x2FFFFB  1152 kB  ROM_FONT       Font table and bitmap            *
 *                                                                             *
 * 0x2FFFFC 0x2FFFFF     4  B  ROM_FONT_ADDR  Font table pointer address       *
 *                                                                             *
 * 0x300000 0x301FFF     8 kB  RAM_DL         Display List RAM                 *
 *                                                                             *
 * 0x302000 0x302FFF     4 kB  REG_*          Registers                        *
 *                                                                             *
 * 0x308000 0x308FFF     4 kB  RAM_CMD        Command Buffer                   *
 *                                                                             *
 *******************************************************************************/

#pragma once

// Uncomment this line if using an Arduino board instead of a Printer Controller board.
// This allows the use of the Arduino SPI Library and SPI Peripheral instead of the bit-banged SPI
// of the Printer Controllers (RAMBo, Mini RAMBo, Einsy, Archim).
// This is used by the CLCD_spiTransfer(uint8_t) function and CLCD_spiInit() function.

// #define IS_ARDUINO

#if defined(USE_FTDI_FT800)
  namespace FTDI {
    //         MEMORY LOCATIONS     FT800
    constexpr uint32_t RAM_G          = 0x000000;   // Main Graphics RAM
    constexpr uint32_t ROM_FONT       = 0x0BB23C;   // Font ROM
    constexpr uint32_t ROM_FONT_ADDR  = 0x0FFFFC;   // Font Table Pointer
    constexpr uint32_t RAM_DL         = 0x100000;   // Display List RAM
    constexpr uint32_t RAM_PAL        = 0x102000;   // Palette RAM
    constexpr uint32_t RAM_REG        = 0x102400;   // Registers
    constexpr uint32_t RAM_CMD        = 0x108000;   // Command Buffer

    constexpr uint32_t RAM_G_SIZE     = 256l*1024l;   // 256k
  }
#endif

#if defined(USE_FTDI_FT810)
  namespace FTDI {
    //         MEMORY LOCATIONS     FT810
    constexpr uint32_t RAM_G          = 0x000000;   // Main Graphics RAM
    constexpr uint32_t ROM_CHIPID     = 0x0C0000;   // Chip ID ROM
    constexpr uint32_t ROM_FONT       = 0x1E0000;   // Font ROM
    constexpr uint32_t ROM_FONT_ADDR  = 0x2FFFFC;   // Font Table Pointer
    constexpr uint32_t RAM_DL         = 0x300000;   // Display List RAM
    constexpr uint32_t RAM_REG        = 0x302000;   // Registers
    constexpr uint32_t RAM_CMD        = 0x308000;   // Command Buffer

    constexpr uint32_t RAM_G_SIZE_800 =  256l*1024l;   // 256k
    constexpr uint32_t RAM_G_SIZE     = 1024l*1024l;  // 1024k
  }
#endif

#if defined(USE_FTDI_FT800)
  namespace FTDI {
    // REGISTERS AND ADDRESSES    FT800

    //             REGISTER              ADDRESS       SIZE    RESET VALUE     TYPE     DESCRIPTION

    constexpr uint32_t REG_ID                = 0x102400;  //    8    0x7C               r     Identification Register, Always 0x7C
    constexpr uint32_t REG_FRAMES            = 0x102404;  //   32    0x00000000         r     Frame Counter, Since Reset
    constexpr uint32_t REG_CLOCK             = 0x102408;  //   32    0x00000000         r     Clock cycles, Since Reset
    constexpr uint32_t REG_FREQUENCY         = 0x10240C;  //   28    0x03938700       r/w     Main Clock Frequency
    constexpr uint32_t REG_RENDERMODE        = 0x102410;  //    1    0x00             r/w     Rendering Mode: 0 = normal, 1 = single-line
    constexpr uint32_t REG_SNAPY             = 0x102414;  //   11    0x0000           r/w     Scan Line Select for RENDERMODE 1
    constexpr uint32_t REG_SNAPSHOT          = 0x102418;  //    1    -                  r     Trigger for RENDERMODE 1
    constexpr uint32_t REG_CPURESET          = 0x10241C;  //    3    0x02             r/w     RESET Bit2 Audio - Bit1 Touch - Bit0 Graphics
    constexpr uint32_t REG_TAP_CRC           = 0x102420;  //   32    -                  r     Live Video Tap
    constexpr uint32_t REG_TAP_MASK          = 0x102424;  //   32    0xFFFFFFFF       r/w     Live Video Tap Mask
    constexpr uint32_t REG_HCYCLE            = 0x102428;  //   12    0x224            r/w     Horizontal Total Cycle Count
    constexpr uint32_t REG_HOFFSET           = 0x10242C;  //   12    0x02B            r/w     Horizontal Display Start Offset
    constexpr uint32_t REG_HSIZE             = 0x102430;  //   12    0x1E0            r/w     Horizontal Display Pixel Count
    constexpr uint32_t REG_HSYNC0            = 0x102434;  //   12    0x000            r/w     Horizontal Sync Fall Offset
    constexpr uint32_t REG_HSYNC1            = 0x102438;  //   12    0x029            r/w     Horizontal Sync Rise Offset
    constexpr uint32_t REG_VCYCLE            = 0x10243C;  //   12    0x124            r/w     Vertical Total Cycle Count
    constexpr uint32_t REG_VOFFSET           = 0x102440;  //   12    0x00C            r/w     Vertical Display Start Offset
    constexpr uint32_t REG_VSIZE             = 0x102444;  //   12    0x110            r/w     Vertical Display Line Count
    constexpr uint32_t REG_VSYNC0            = 0x102448;  //   10    0x000            r/w     Vertical Sync Fall Offset
    constexpr uint32_t REG_VSYNC1            = 0x10244C;  //   10    0x00A            r/w     Vertical Sync Rise Offset
    constexpr uint32_t REG_DLSWAP            = 0x102450;  //    2    0x00             r/w     Display List Swap Control
    constexpr uint32_t REG_ROTATE            = 0x102454;  //    3    0x00             r/w     Screen 90,180, 270 degree rotate
    constexpr uint32_t REG_OUTBITS           = 0x102458;  //    9    0x1B6            r/w     Output Resolution, 3x3x3 Bits
    constexpr uint32_t REG_DITHER            = 0x10245C;  //    1    0x01             r/w     Output Dither Enable
    constexpr uint32_t REG_SWIZZLE           = 0x102460;  //    4    0x00             r/w     Output RGB Swizzle, Pin Change for PCB Routing
    constexpr uint32_t REG_CSPREAD           = 0x102464;  //    1    0x01             r/w     Output Clock Spreading Enable
    constexpr uint32_t REG_PCLK_POL          = 0x102468;  //    1    0x00             r/w     PCLK Polarity: 0 = Rising Edge, 1 = Falling Edge
    constexpr uint32_t REG_PCLK              = 0x10246C;  //    8    0x00             r/w     PCLK Frequency Divider, 0 = Disable Clock
    constexpr uint32_t REG_TAG_X             = 0x102470;  //   11    0x000            r/w     Tag Query X Coordinate
    constexpr uint32_t REG_TAG_Y             = 0x102474;  //   11    0x000            r/w     Tag Query Y Coordinate
    constexpr uint32_t REG_TAG               = 0x102478;  //    8    0x00               r     Tag Query Result
    constexpr uint32_t REG_VOL_PB            = 0x10247C;  //    8    0xFF             r/w     Audio Playback Volume
    constexpr uint32_t REG_VOL_SOUND         = 0x102480;  //    8    0xFF             r/w     Audio Synthesizer Volume
    constexpr uint32_t REG_SOUND             = 0x102484;  //   16    0x0000           r/w     Audio Sound Effect Select
    constexpr uint32_t REG_PLAY              = 0x102488;  //    1    0x00             r/w     Audio Start Effect Playback
    constexpr uint32_t REG_GPIO_DIR          = 0x10248C;  //    8    0x80             r/w     GPIO Pin Direction: 0 = Input , 1 = Output
    constexpr uint32_t REG_GPIO              = 0x102490;  //    8    0x00             r/w     GPIO Pin Values for 0, 1, 7 Drive Strength 2, 3, 4, 5, 6
    constexpr uint32_t REG_INT_FLAGS         = 0x102498;  //    8    0x00               r     Interrupt Flags, Clear by Reading
    constexpr uint32_t REG_INT_EN            = 0x10249C;  //    1    0x00             r/w     Global Interrupt Enable
    constexpr uint32_t REG_INT_MASK          = 0x1024A0;  //    8    0xFF             r/w     Interrupt Enable Mask
    constexpr uint32_t REG_PLAYBACK_START    = 0x1024A4;  //   20    0x00000          r/w     Audio Playback RAM Start Address
    constexpr uint32_t REG_PLAYBACK_LENGTH   = 0x1024A8;  //   20    0x00000          r/w     Audio Playback Sample Length (Bytes)
    constexpr uint32_t REG_PLAYBACK_READPTR  = 0x1024AC;  //   20    -                  r     Audio Playback Read Pointer
    constexpr uint32_t REG_PLAYBACK_FREQ     = 0x1024B0;  //   16    0x1F40           r/w     Audio Playback Frequency (Hz)
    constexpr uint32_t REG_PLAYBACK_FORMAT   = 0x1024B4;  //    2    0x00             r/w     Audio Playback Format
    constexpr uint32_t REG_PLAYBACK_LOOP     = 0x1024B8;  //    1    0x00             r/w     Audio Playback Loop Enable
    constexpr uint32_t REG_PLAYBACK_PLAY     = 0x1024BC;  //    1    0x00               r     Audio Start Playback
    constexpr uint32_t REG_PWM_HZ            = 0x1024C0;  //   14    0x00FA           r/w     Backlight PWM Frequency (Hz)
    constexpr uint32_t REG_PWM_DUTY          = 0x1024C4;  //    8    0x80             r/w     Backlight PWM Duty Cycle: 0 = 0%, 128 = 100%
    constexpr uint32_t REG_MACRO_0           = 0x1024C8;  //   32    0x00000000       r/w     Display List Macro Command 0
    constexpr uint32_t REG_MACRO_1           = 0x1024CC;  //   32    0x00000000       r/w     Display List Macro Command 1
    constexpr uint32_t REG_CMD_READ          = 0x1024E4;  //   12    0x000            r/w     Command Buffer Read Pointer
    constexpr uint32_t REG_CMD_WRITE         = 0x1024E8;  //   12    0x000            r/w     Command Buffer Write Pointer
    constexpr uint32_t REG_CMD_DL            = 0x1024EC;  //   13    0x0000           r/w     Command Display List Offset
    constexpr uint32_t REG_TOUCH_MODE        = 0x1024F0;  //    2    0x03             r/w     Touch-Screen Sampling Mode
    constexpr uint32_t REG_TOUCH_ADC_MODE    = 0x1024F4;  //    1    0x01             r/w     Select Single Ended or Differential Sampling
    constexpr uint32_t REG_TOUCH_CHARGE      = 0x1024F8;  //   16    0x1770           r/w     Touch Screen Charge Time, n x 6 Clocks
    constexpr uint32_t REG_TOUCH_SETTLE      = 0x1024FC;  //    4    0x03             r/w     Touch-Screen Settle Time, n x 6 Clocks
    constexpr uint32_t REG_TOUCH_OVERSAMPLE  = 0x102500;  //    4    0x07             r/w     Touch-Screen Oversample Factor
    constexpr uint32_t REG_TOUCH_RZTHRESH    = 0x102504;  //   16    0xFFFF           r/w     Touch-Screen Resistance Threshold
    constexpr uint32_t REG_TOUCH_RAW_XY      = 0x102508;  //   32    -                  r     Touch-Screen Raw (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_RZ          = 0x10250C;  //   16    -                  r     Touch-Screen Resistance
    constexpr uint32_t REG_TOUCH_SCREEN_XY   = 0x102510;  //   32    -                  r     Touch-Screen Screen (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_TAG_XY      = 0x102514;  //   32    -                  r     Touch-Screen Tag 0 Lookup (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_TAG         = 0x102518;  //    8    -                  r     Touch-Screen Tag 0 Result
    constexpr uint32_t REG_TOUCH_TRANSFORM_A = 0x10251C;  //   32    0x00010000       r/w     Touch-Screen Transform Coefficient A (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_B = 0x102520;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient B (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_C = 0x102524;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient C (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_D = 0x102528;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient D (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_E = 0x10252C;  //   32    0x00010000       r/w     Touch-Screen Transform Coefficient E (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_F = 0x102530;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient F (s15.16)
  //               Reserved Addresses      0x102434 - 0x102470
    constexpr uint32_t REG_TOUCH_DIRECT_XY   = 0x102574;  //   32    -                  r     Touch-Screen Direct Conversions XY (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_DIRECT_Z1Z2 = 0x102578;  //   32    -                  r     Touch-Screen Direct Conversions Z (z1-MSB16; z2-LSB16)
    constexpr uint32_t REG_TRACKER           = 0x109000;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)
  };
#endif

#if defined(USE_FTDI_FT810)
  namespace FTDI {
    // REGISTERS AND ADDRESSES    FT810

    //             REGISTER              ADDRESS       SIZE    RESET VALUE     TYPE     DESCRIPTION

    constexpr uint32_t REG_ID                = 0x302000;  //    8    0x7C               r     Identification Register, Always 0x7C
    constexpr uint32_t REG_FRAMES            = 0x302004;  //   32    0x00000000         r     Frame Counter, Since Reset
    constexpr uint32_t REG_CLOCK             = 0x302008;  //   32    0x00000000         r     Clock cycles, Since Reset
    constexpr uint32_t REG_FREQUENCY         = 0x30200C;  //   28    0x03938700       r/w     Main Clock Frequency
    constexpr uint32_t REG_RENDERMODE        = 0x302010;  //    1    0x00             r/w     Rendering Mode: 0 = normal, 1 = single-line
    constexpr uint32_t REG_SNAPY             = 0x302014;  //   11    0x0000           r/w     Scan Line Select for RENDERMODE 1
    constexpr uint32_t REG_SNAPSHOT          = 0x302018;  //    1    -                  r     Trigger for RENDERMODE 1
    constexpr uint32_t REG_SNAPFORMAT        = 0x30201C;  //    6    0x20             r/w     Pixel Format for Scanline Readout
    constexpr uint32_t REG_CPURESET          = 0x302020;  //    3    0x02             r/w     RESET Bit2 Audio - Bit1 Touch - Bit0 Graphics
    constexpr uint32_t REG_TAP_CRC           = 0x302024;  //   32    -                  r     Live Video Tap
    constexpr uint32_t REG_TAP_MASK          = 0x302028;  //   32    0xFFFFFFFF       r/w     Live Video Tap Mask
    constexpr uint32_t REG_HCYCLE            = 0x30202C;  //   12    0x224            r/w     Horizontal Total Cycle Count
    constexpr uint32_t REG_HOFFSET           = 0x302030;  //   12    0x02B            r/w     Horizontal Display Start Offset
    constexpr uint32_t REG_HSIZE             = 0x302034;  //   12    0x1E0            r/w     Horizontal Display Pixel Count
    constexpr uint32_t REG_HSYNC0            = 0x302038;  //   12    0x000            r/w     Horizontal Sync Fall Offset
    constexpr uint32_t REG_HSYNC1            = 0x30203C;  //   12    0x029            r/w     Horizontal Sync Rise Offset
    constexpr uint32_t REG_VCYCLE            = 0x302040;  //   12    0x124            r/w     Vertical Total Cycle Count
    constexpr uint32_t REG_VOFFSET           = 0x302044;  //   12    0x00C            r/w     Vertical Display Start Offset
    constexpr uint32_t REG_VSIZE             = 0x302048;  //   12    0x110            r/w     Vertical Display Line Count
    constexpr uint32_t REG_VSYNC0            = 0x30204C;  //   10    0x000            r/w     Vertical Sync Fall Offset
    constexpr uint32_t REG_VSYNC1            = 0x302050;  //   10    0x00A            r/w     Vertical Sync Rise Offset
    constexpr uint32_t REG_DLSWAP            = 0x302054;  //    2    0x00             r/w     Display List Swap Control
    constexpr uint32_t REG_ROTATE            = 0x302058;  //    3    0x00             r/w     Screen 90,180, 270 degree rotate
    constexpr uint32_t REG_OUTBITS           = 0x30205C;  //    9    0x1B6            r/w     Output Resolution, 3x3x3 Bits
    constexpr uint32_t REG_DITHER            = 0x302060;  //    1    0x01             r/w     Output Dither Enable
    constexpr uint32_t REG_SWIZZLE           = 0x302064;  //    4    0x00             r/w     Output RGB Swizzle, Pin Change for PCB Routing
    constexpr uint32_t REG_CSPREAD           = 0x302068;  //    1    0x01             r/w     Output Clock Spreading Enable
    constexpr uint32_t REG_PCLK_POL          = 0x30206C;  //    1    0x00             r/w     PCLK Polarity: 0 = Rising Edge, 1 = Falling Edge
    constexpr uint32_t REG_PCLK              = 0x302070;  //    8    0x00             r/w     PCLK Frequency Divider, 0 = Disable Clock
    constexpr uint32_t REG_TAG_X             = 0x302074;  //   11    0x000            r/w     Tag Query X Coordinate
    constexpr uint32_t REG_TAG_Y             = 0x302078;  //   11    0x000            r/w     Tag Query Y Coordinate
    constexpr uint32_t REG_TAG               = 0x30207C;  //    8    0x00               r     Tag Query Result
    constexpr uint32_t REG_VOL_PB            = 0x302080;  //    8    0xFF             r/w     Audio Playback Volume
    constexpr uint32_t REG_VOL_SOUND         = 0x302084;  //    8    0xFF             r/w     Audio Synthesizer Volume
    constexpr uint32_t REG_SOUND             = 0x302088;  //   16    0x0000           r/w     Audio Sound Effect Select
    constexpr uint32_t REG_PLAY              = 0x30208C;  //    1    0x00             r/w     Audio Start Effect Playback
    constexpr uint32_t REG_GPIO_DIR          = 0x302090;  //    8    0x80             r/w     GPIO Pin Direction: 0 = Input , 1 = Output
    constexpr uint32_t REG_GPIO              = 0x302094;  //    8    0x00             r/w     GPIO Pin Values for 0, 1, 7 Drive Strength 2, 3, 4, 5, 6
    constexpr uint32_t REG_GPIOX_DIR         = 0x302098;  //   16    0x8000           r/w     Extended GPIO Pin Direction
    constexpr uint32_t REG_GPIOX             = 0x30209C;  //   16    0x0080           r/w     Extended GPIO Pin Values
    //             Reserved Addr           0x3020A0
    //             Reserved Addr           0x3020A4
    constexpr uint32_t REG_INT_FLAGS         = 0x3020A8;  //    8    0x00               r     Interrupt Flags, Clear by Reading
    constexpr uint32_t REG_INT_EN            = 0x3020AC;  //    1    0x00             r/w     Global Interrupt Enable
    constexpr uint32_t REG_INT_MASK          = 0x3020B0;  //    8    0xFF             r/w     Interrupt Enable Mask
    constexpr uint32_t REG_PLAYBACK_START    = 0x3020B4;  //   20    0x00000          r/w     Audio Playback RAM Start Address
    constexpr uint32_t REG_PLAYBACK_LENGTH   = 0x3020B8;  //   20    0x00000          r/w     Audio Playback Sample Length (Bytes)
    constexpr uint32_t REG_PLAYBACK_READPTR  = 0x3020BC;  //   20    -                  r     Audio Playback Read Pointer
    constexpr uint32_t REG_PLAYBACK_FREQ     = 0x3020C0;  //   16    0x1F40           r/w     Audio Playback Frequency (Hz)
    constexpr uint32_t REG_PLAYBACK_FORMAT   = 0x3020C4;  //    2    0x00             r/w     Audio Playback Format
    constexpr uint32_t REG_PLAYBACK_LOOP     = 0x3020C8;  //    1    0x00             r/w     Audio Playback Loop Enable
    constexpr uint32_t REG_PLAYBACK_PLAY     = 0x3020CC;  //    1    0x00               r     Audio Start Playback
    constexpr uint32_t REG_PWM_HZ            = 0x3020D0;  //   14    0x00FA           r/w     Backlight PWM Frequency (Hz)
    constexpr uint32_t REG_PWM_DUTY          = 0x3020D4;  //    8    0x80             r/w     Backlight PWM Duty Cycle: 0 = 0%, 128 = 100%
    constexpr uint32_t REG_MACRO_0           = 0x3020D8;  //   32    0x00000000       r/w     Display List Macro Command 0
    constexpr uint32_t REG_MACRO_1           = 0x3020DC;  //   32    0x00000000       r/w     Display List Macro Command 1
    //             Reserved Addr           0x3020E0
    //             Reserved Addr           0x3020E4
    //             Reserved Addr           0x3020E8
    //             Reserved Addr           0x3020EC
    //             Reserved Addr           0x3020F0
    //             Reserved Addr           0x3020F4
    constexpr uint32_t REG_CMD_READ          = 0x3020F8;  //   12    0x000            r/w     Command Buffer Read Pointer
    constexpr uint32_t REG_CMD_WRITE         = 0x3020FC;  //   12    0x000            r/w     Command Buffer Write Pointer
    constexpr uint32_t REG_CMD_DL            = 0x302100;  //   13    0x0000           r/w     Command Display List Offset
    constexpr uint32_t REG_TOUCH_MODE        = 0x302104;  //    2    0x03             r/w     Touch-Screen Sampling Mode
    constexpr uint32_t REG_TOUCH_ADC_MODE    = 0x302108;  //    1    0x01             r/w     Select Single Ended or Differential Sampling
    constexpr uint32_t REG_TOUCH_CHARGE      = 0x30210C;  //   16    0x1770           r/w     Touch Screen Charge Time, n x 6 Clocks
    constexpr uint32_t REG_TOUCH_SETTLE      = 0x302110;  //    4    0x03             r/w     Touch-Screen Settle Time, n x 6 Clocks
    constexpr uint32_t REG_TOUCH_OVERSAMPLE  = 0x302114;  //    4    0x07             r/w     Touch-Screen Oversample Factor
    constexpr uint32_t REG_TOUCH_RZTHRESH    = 0x302118;  //   16    0xFFFF           r/w     Touch-Screen Resistance Threshold
    constexpr uint32_t REG_TOUCH_RAW_XY      = 0x30211C;  //   32    -                  r     Touch-Screen Raw (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_RZ          = 0x302120;  //   16    -                  r     Touch-Screen Resistance
    constexpr uint32_t REG_TOUCH_SCREEN_XY   = 0x302124;  //   32    -                  r     Touch-Screen Screen (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_TAG_XY      = 0x302128;  //   32    -                  r     Touch-Screen Tag 0 Lookup (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_TAG         = 0x30212C;  //    8    -                  r     Touch-Screen Tag 0 Result
    constexpr uint32_t REG_TOUCH_TAG1_XY     = 0x302130;  //   32    -                  r     Touch-Screen Tag 1 Lookup
    constexpr uint32_t REG_TOUCH_TAG1        = 0x302134;  //    8    -                  r     Touch-Screen Tag 1 Result
    constexpr uint32_t REG_TOUCH_TAG2_XY     = 0x302138;  //   32    -                  r     Touch-Screen Tag 2 Lookup
    constexpr uint32_t REG_TOUCH_TAG2        = 0x30213C;  //    8    -                  r     Touch-Screen Tag 2 Result
    constexpr uint32_t REG_TOUCH_TAG3_XY     = 0x302140;  //   32    -                  r     Touch-Screen Tag 3 Lookup
    constexpr uint32_t REG_TOUCH_TAG3        = 0x302144;  //    8    -                  r     Touch-Screen Tag 3 Result
    constexpr uint32_t REG_TOUCH_TAG4_XY     = 0x302148;  //   32    -                  r     Touch-Screen Tag 4 Lookup
    constexpr uint32_t REG_TOUCH_TAG4        = 0x30214C;  //    8    -                  r     Touch-Screen Tag 4 Result
    constexpr uint32_t REG_TOUCH_TRANSFORM_A = 0x302150;  //   32    0x00010000       r/w     Touch-Screen Transform Coefficient A (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_B = 0x302154;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient B (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_C = 0x302158;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient C (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_D = 0x30215C;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient D (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_E = 0x302160;  //   32    0x00010000       r/w     Touch-Screen Transform Coefficient E (s15.16)
    constexpr uint32_t REG_TOUCH_TRANSFORM_F = 0x302164;  //   32    0x00000000       r/w     Touch-Screen Transform Coefficient F (s15.16)
    constexpr uint32_t REG_TOUCH_CONFIG      = 0x302168;  //   16    0x8381           r/w     Touch Configuration
    constexpr uint32_t REG_CTOUCH_TOUCH4_X   = 0x30216C;  //   16    -                  r     Extended Mode Touch Screen
    //             Reserved Addresses      0x302170
    constexpr uint32_t REG_BIST_EN           = 0x302174;  //    1    0                r/w     BIST Memory Mapping Enable
    //             Reserved Addr           0x302178
    //             Reserved Addr           0x30217C
    constexpr uint32_t REG_TRIM              = 0x302180;  //    8    0                r/w     Internal Clock Trimming
    constexpr uint32_t REG_ANA_COMP          = 0x302184;  //    8    0                r/w     Analog Control Register
    constexpr uint32_t REG_SPI_WIDTH         = 0x302188;  //    3    0                r/w     QSPI Bus Width Setting
    constexpr uint32_t REG_TOUCH_DIRECT_XY   = 0x30218C;  //   32    -                  r     Touch-Screen Direct Conversions XY (x-MSB16; y-LSB16)
    constexpr uint32_t REG_TOUCH_DIRECT_Z1Z2 = 0x302190;  //   32    -                  r     Touch-Screen Direct Conversions Z (z1-MSB16; z2-LSB16)
    //             Reserved Addresses      0x302194 - 0x302560
    constexpr uint32_t REG_DATESTAMP         = 0x320564;  //  128    -                  r     Stamp Date Code
    constexpr uint32_t REG_CMDB_SPACE        = 0x302574;  //   12    0xFFC            r/w     Command DL Space Available
    constexpr uint32_t REG_CMDB_WRITE        = 0x302578;  //   32    0                  w     Command DL Write

    constexpr uint32_t REG_TRACKER           = 0x309000;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)
    constexpr uint32_t REG_TRACKER_1         = 0x309004;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)
    constexpr uint32_t REG_TRACKER_2         = 0x309008;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)
    constexpr uint32_t REG_TRACKER_3         = 0x30900C;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)
    constexpr uint32_t REG_TRACKER_4         = 0x309010;  //   32    0x00000000       r/w     Track Register (Track Value MSB16; Tag Value - LSB8)

    constexpr uint32_t REG_MEDIAFIFO_READ    = 0x309014;  //   32    0x00000000       r/w     Media FIFO read pointer
    constexpr uint32_t REG_MEDIAFIFO_WRITE   = 0x309018;  //   32    0x00000000       r/w     Media FIFO write pointer
  }
#endif

// OPTIONS

namespace FTDI {
  constexpr uint16_t OPT_3D           = 0x0000;
  constexpr uint16_t OPT_RGB565       = 0x0000;
  constexpr uint16_t OPT_MONO         = 0x0001;
  constexpr uint16_t OPT_NODL         = 0x0002;
  constexpr uint16_t OPT_FLAT         = 0x0100;
  constexpr uint16_t OPT_SIGNED       = 0x0100;
  constexpr uint16_t OPT_CENTERX      = 0x0200;
  constexpr uint16_t OPT_CENTERY      = 0x0400;
  constexpr uint16_t OPT_CENTER       = (OPT_CENTERX | OPT_CENTERY);
  constexpr uint16_t OPT_RIGHTX       = 0x0800;
  constexpr uint16_t OPT_NOBACK       = 0x1000;
  constexpr uint16_t OPT_NOTICKS      = 0x2000;
  constexpr uint16_t OPT_NOHM         = 0x4000;
  constexpr uint16_t OPT_NOPOINTER    = 0x4000;
  constexpr uint16_t OPT_NOSECS       = 0x8000;
  constexpr uint16_t OPT_NOHANDS      = (OPT_NOPOINTER | OPT_NOSECS);

  #if defined(USE_FTDI_FT810)
    constexpr uint16_t OPT_NOTEAR      = 0x0004;
    constexpr uint16_t OPT_FULLSCREEN  = 0x0008;
    constexpr uint16_t OPT_MEDIAFIFO   = 0x0010;
    constexpr uint16_t OPT_SOUND       = 0x0020;
  #endif
}

// GPIO Bits

namespace FTDI {
  constexpr uint8_t GPIO_GP0         = 1 << 0;
  constexpr uint8_t GPIO_GP1         = 1 << 1;
  constexpr uint8_t GPIO_DISP        = 1 << 7;

  #if defined(USE_FTDI_FT810)
    constexpr uint16_t GPIOX_GP0       = 1 << 0;
    constexpr uint16_t GPIOX_GP1       = 1 << 1;
    constexpr uint16_t GPIOX_DISP      = 1 << 15;
  #endif

}

// HOST COMMANDS

namespace FTDI {
  constexpr uint8_t ACTIVE  = 0x00;
  constexpr uint8_t STANDBY = 0x41;
  constexpr uint8_t SLEEP   = 0x42;
  constexpr uint8_t PWRDOWN = 0x50;
  constexpr uint8_t CLKEXT  = 0x44;
  constexpr uint8_t CLKINT  = 0x48;
  constexpr uint8_t CORESET = 0x68;

  #if defined(USE_FTDI_FT800)
    constexpr uint8_t CLK48M  = 0x62;
    constexpr uint8_t CLK36M  = 0x61;
  #else
    constexpr uint8_t CLKSEL  = 0x61;
  #endif
}


// DISPLAY LIST COMMANDS

namespace FTDI {
  constexpr uint8_t ARGB1555                           = 0;
  constexpr uint8_t L1                                 = 1;
  constexpr uint8_t L4                                 = 2;
  constexpr uint8_t L8                                 = 3;
  constexpr uint8_t RGB332                             = 4;
  constexpr uint8_t ARGB2                              = 5;
  constexpr uint8_t ARGB4                              = 6;
  constexpr uint8_t RGB565                             = 7;
  constexpr uint8_t PALETTED                           = 8;
  constexpr uint8_t TEXT8X8                            = 9;
  constexpr uint8_t TEXTVGA                            = 10;
  constexpr uint8_t BARGRAPH                           = 11;

  constexpr uint8_t ALPHA_FUNC_NEVER                   = 0;
  constexpr uint8_t ALPHA_FUNC_LESS                    = 1;
  constexpr uint8_t ALPHA_FUNC_LEQUAL                  = 2;
  constexpr uint8_t ALPHA_FUNC_GREATER                 = 3;
  constexpr uint8_t ALPHA_FUNC_GEQUAL                  = 4;
  constexpr uint8_t ALPHA_FUNC_EQUAL                   = 5;
  constexpr uint8_t ALPHA_FUNC_NOTEQUAL                = 6;
  constexpr uint8_t ALPHA_FUNC_ALWAYS                  = 7;

  constexpr uint8_t NEAREST                            = 0;
  constexpr uint8_t BILINEAR                           = 1;
  constexpr uint8_t BORDER                             = 0;
  constexpr uint8_t REPEAT                             = 1;

  constexpr uint8_t BLEND_FUNC_ZERO                    = 0;
  constexpr uint8_t BLEND_FUNC_ONE                     = 1;
  constexpr uint8_t BLEND_FUNC_SRC_ALPHA               = 2;
  constexpr uint8_t BLEND_FUNC_DST_ALPHA               = 3;
  constexpr uint8_t BLEND_FUNC_ONE_MINUS_SRC_ALPHA     = 4;
  constexpr uint8_t BLEND_FUNC_ONE_MINUS_DST_ALPHA     = 5;

  constexpr uint32_t COLOR_MASK_RED                    = 8;
  constexpr uint32_t COLOR_MASK_GRN                    = 4;
  constexpr uint32_t COLOR_MASK_BLU                    = 2;
  constexpr uint32_t COLOR_MASK_ALPHA                  = 1;

  constexpr uint8_t STENCIL_FUNC_NEVER                 = 0;
  constexpr uint8_t STENCIL_FUNC_LESS                  = 1;
  constexpr uint8_t STENCIL_FUNC_LEQUAL                = 2;
  constexpr uint8_t STENCIL_FUNC_GREATER               = 3;
  constexpr uint8_t STENCIL_FUNC_GEQUAL                = 4;
  constexpr uint8_t STENCIL_FUNC_EQUAL                 = 5;
  constexpr uint8_t STENCIL_FUNC_NOTEQUAL              = 6;
  constexpr uint8_t STENCIL_FUNC_ALWAYS                = 7;

  constexpr uint8_t STENCIL_OP_ZERO                    = 0;
  constexpr uint8_t STENCIL_OP_KEEP                    = 1;
  constexpr uint8_t STENCIL_OP_REPLACE                 = 2;
  constexpr uint8_t STENCIL_OP_INCR                    = 3;
  constexpr uint8_t STENCIL_OP_DECR                    = 4;
  constexpr uint8_t STENCIL_OP_INVERT                  = 5;

  typedef enum: uint32_t {
   BITMAPS                                             = 1,
   POINTS                                              = 2,
   LINES                                               = 3,
   LINE_STRIP                                          = 4,
   EDGE_STRIP_R                                        = 5,
   EDGE_STRIP_L                                        = 6,
   EDGE_STRIP_A                                        = 7,
   EDGE_STRIP_B                                        = 8,
   RECTS                                               = 9
  } begin_t;

  namespace DL {
    constexpr uint32_t ALPHA_FUNC                         = 0x09000000;
    constexpr uint32_t BEGIN                              = 0x1F000000;
    constexpr uint32_t BITMAP_HANDLE                      = 0x05000000;
    constexpr uint32_t BITMAP_LAYOUT                      = 0x07000000;
    constexpr uint32_t BITMAP_SIZE                        = 0x08000000;
    constexpr uint32_t BITMAP_SOURCE                      = 0x01000000;
    constexpr uint32_t BITMAP_TRANSFORM_A                 = 0x15000000;
    constexpr uint32_t BITMAP_TRANSFORM_B                 = 0x16000000;
    constexpr uint32_t BITMAP_TRANSFORM_C                 = 0x17000000;
    constexpr uint32_t BITMAP_TRANSFORM_D                 = 0x18000000;
    constexpr uint32_t BITMAP_TRANSFORM_E                 = 0x19000000;
    constexpr uint32_t BITMAP_TRANSFORM_F                 = 0x1A000000;
    constexpr uint32_t BLEND_FUNC                         = 0x0B000000;
    constexpr uint32_t CALL                               = 0x1D000000;
    constexpr uint32_t CELL                               = 0x06000000;
    constexpr uint32_t CLEAR                              = 0x26000000;
    constexpr uint32_t CLEAR_COLOR_BUFFER                 = 0x00000004;
    constexpr uint32_t CLEAR_STENCIL_BUFFER               = 0x00000002;
    constexpr uint32_t CLEAR_TAG_BUFFER                   = 0x00000001;
    constexpr uint32_t CLEAR_COLOR_A                      = 0x0F000000;
    constexpr uint32_t CLEAR_COLOR_RGB                    = 0x02000000;
    constexpr uint32_t CLEAR_STENCIL                      = 0x11000000;
    constexpr uint32_t CLEAR_TAG                          = 0x12000000;
    constexpr uint32_t COLOR_A                            = 0x10000000;
    constexpr uint32_t COLOR_MASK                         = 0x20000000;
    constexpr uint32_t COLOR_RGB                          = 0x04000000;
    constexpr uint32_t DL_DISPLAY                         = 0x00000000;
    constexpr uint32_t END                                = 0x21000000;
    constexpr uint32_t JUMP                               = 0x1E000000;
    constexpr uint32_t LINE_WIDTH                         = 0x0E000000;
    constexpr uint32_t MACRO                              = 0x25000000;
    constexpr uint32_t POINT_SIZE                         = 0x0D000000;
    constexpr uint32_t RESTORE_CONTEXT                    = 0x23000000;
    constexpr uint32_t RETURN                             = 0x24000000;
    constexpr uint32_t SAVE_CONTEXT                       = 0x22000000;
    constexpr uint32_t SCISSOR_SIZE                       = 0x1C000000;
    constexpr uint32_t SCISSOR_XY                         = 0x1B000000;
    constexpr uint32_t STENCIL_FUNC                       = 0x0A000000;
    constexpr uint32_t STENCIL_MASK                       = 0x13000000;
    constexpr uint32_t STENCIL_OP                         = 0x0C000000;
    constexpr uint32_t TAG                                = 0x03000000;
    constexpr uint32_t TAG_MASK                           = 0x14000000;
    constexpr uint32_t VERTEX2F                           = 0x40000000;
    constexpr uint32_t VERTEX2II                          = 0x80000000;
    #if defined(USE_FTDI_FT810)
    constexpr uint32_t BITMAP_LAYOUT_H                    = 0x28000000;
    constexpr uint32_t BITMAP_SIZE_H                      = 0x29000000;
    constexpr uint32_t VERTEX_FORMAT                      = 0x27000000;
    constexpr uint32_t VERTEX_TRANSLATE_X                 = 0x2B000000;
    constexpr uint32_t VERTEX_TRANSLATE_Y                 = 0x2C000000;
    #endif
  }
}

// CO-PROCESSOR ENGINE COMMANDS
namespace FTDI {
  constexpr uint32_t CMD_DLSTART                        = 0xFFFFFF00;
  constexpr uint32_t CMD_SWAP                           = 0xFFFFFF01;
  constexpr uint32_t CMD_COLDSTART                      = 0xFFFFFF32;
  constexpr uint32_t CMD_INTERRUPT                      = 0xFFFFFF02;
  constexpr uint32_t CMD_APPEND                         = 0xFFFFFF1E;
  constexpr uint32_t CMD_REGREAD                        = 0xFFFFFF19;
  constexpr uint32_t CMD_MEMWRITE                       = 0xFFFFFF1A;
  constexpr uint32_t CMD_INFLATE                        = 0xFFFFFF22;
  constexpr uint32_t CMD_LOADIMAGE                      = 0xFFFFFF24;
  constexpr uint32_t CMD_MEMCRC                         = 0xFFFFFF18;
  constexpr uint32_t CMD_MEMZERO                        = 0xFFFFFF1C;
  constexpr uint32_t CMD_MEMSET                         = 0xFFFFFF1B;
  constexpr uint32_t CMD_MEMCPY                         = 0xFFFFFF1D;
  constexpr uint32_t CMD_BUTTON                         = 0xFFFFFF0D;
  constexpr uint32_t CMD_CLOCK                          = 0xFFFFFF14;
  constexpr uint32_t CMD_FGCOLOR                        = 0xFFFFFF0A;
  constexpr uint32_t CMD_BGCOLOR                        = 0xFFFFFF09;
  constexpr uint32_t CMD_GRADCOLOR                      = 0xFFFFFF34;
  constexpr uint32_t CMD_GAUGE                          = 0xFFFFFF13;
  constexpr uint32_t CMD_GRADIENT                       = 0xFFFFFF0B;
  constexpr uint32_t CMD_KEYS                           = 0xFFFFFF0E;
  constexpr uint32_t CMD_PROGRESS                       = 0xFFFFFF0F;
  constexpr uint32_t CMD_SCROLLBAR                      = 0xFFFFFF11;
  constexpr uint32_t CMD_SLIDER                         = 0xFFFFFF10;
  constexpr uint32_t CMD_DIAL                           = 0xFFFFFF2D;
  constexpr uint32_t CMD_TOGGLE                         = 0xFFFFFF12;
  constexpr uint32_t CMD_TEXT                           = 0xFFFFFF0C;
  constexpr uint32_t CMD_NUMBER                         = 0xFFFFFF2E;
  constexpr uint32_t CMD_LOADIDENTITY                   = 0xFFFFFF26;
  constexpr uint32_t CMD_SETMATRIX                      = 0xFFFFFF2A;
  constexpr uint32_t CMD_GETMATRIX                      = 0xFFFFFF33;
  constexpr uint32_t CMD_GETPTR                         = 0xFFFFFF23;
  constexpr uint32_t CMD_GETPROPS                       = 0xFFFFFF25;
  constexpr uint32_t CMD_SCALE                          = 0xFFFFFF28;
  constexpr uint32_t CMD_ROTATE                         = 0xFFFFFF29;
  constexpr uint32_t CMD_TRANSLATE                      = 0xFFFFFF27;
  constexpr uint32_t CMD_CALIBRATE                      = 0xFFFFFF15;
  constexpr uint32_t CMD_SPINNER                        = 0xFFFFFF16;
  constexpr uint32_t CMD_SCREENSAVER                    = 0xFFFFFF2F;
  constexpr uint32_t CMD_SKETCH                         = 0xFFFFFF30;
  constexpr uint32_t CMD_STOP                           = 0xFFFFFF17;
  constexpr uint32_t CMD_SETFONT                        = 0xFFFFFF2B;
  constexpr uint32_t CMD_TRACK                          = 0xFFFFFF2C;
  constexpr uint32_t CMD_SNAPSHOT                       = 0xFFFFFF1F;
  constexpr uint32_t CMD_LOGO                           = 0xFFFFFF31;
}

#if defined(USE_FTDI_FT810)
  namespace FTDI {
    constexpr uint32_t CMD_SETROTATE                    = 0xFFFFFF36;
    constexpr uint32_t CMD_SNAPSHOT2                    = 0xFFFFFF37;
    constexpr uint32_t CMD_SETBASE                      = 0xFFFFFF38;
    constexpr uint32_t CMD_MEDIAFIFO                    = 0xFFFFFF39;
    constexpr uint32_t CMD_PLAYVIDEO                    = 0xFFFFFF3A;
    constexpr uint32_t CMD_VIDEOSTART                   = 0xFFFFFF40;
    constexpr uint32_t CMD_VIDEOFRAME                   = 0xFFFFFF41;
    constexpr uint32_t CMD_SETBITMAP                    = 0xFFFFFF43;
  }
#endif

namespace FTDI {
  enum effect_t {
    SILENCE                                         = 0x00,
    SQUARE_WAVE                                     = 0x01,
    SINE_WAVE                                       = 0x02,
    SAWTOOTH_WAVE                                   = 0x03,
    TRIANGLE_WAVE                                   = 0x04,
    BEEPING                                         = 0x05,
    ALARM                                           = 0x06,
    WARBLE                                          = 0x07,
    CAROUSEL                                        = 0x08,
    SHORT_PIPS_1                                    = 0x10,
    SHORT_PIPS_2                                    = 0x11,
    SHORT_PIPS_3                                    = 0x12,
    SHORT_PIPS_4                                    = 0x13,
    SHORT_PIPS_5                                    = 0x14,
    SHORT_PIPS_6                                    = 0x15,
    SHORT_PIPS_7                                    = 0x16,
    SHORT_PIPS_8                                    = 0x17,
    SHORT_PIPS_9                                    = 0x18,
    SHORT_PIPS_10                                   = 0x19,
    SHORT_PIPS_11                                   = 0x1A,
    SHORT_PIPS_12                                   = 0x1B,
    SHORT_PIPS_13                                   = 0x1C,
    SHORT_PIPS_14                                   = 0x1D,
    SHORT_PIPS_15                                   = 0x1E,
    SHORT_PIPS_16                                   = 0x1F,
    DTMF_POUND                                      = 0x23,
    DTMF_STAR                                       = 0x2C,
    DTMF_0                                          = 0x30,
    DTMF_1                                          = 0x31,
    DTMF_2                                          = 0x32,
    DTMF_3                                          = 0x33,
    DTMF_4                                          = 0x34,
    DTMF_5                                          = 0x35,
    DTMF_6                                          = 0x36,
    DTMF_7                                          = 0x37,
    DTMF_8                                          = 0x38,
    DTMF_9                                          = 0x39,
    HARP                                            = 0x40,
    XYLOPHONE                                       = 0x41,
    TUBA                                            = 0x42,
    GLOCKENSPIEL                                    = 0x43,
    ORGAN                                           = 0x44,
    TRUMPET                                         = 0x45,
    PIANO                                           = 0x46,
    CHIMES                                          = 0x47,
    MUSIC_BOX                                       = 0x48,
    BELL                                            = 0x49,
    CLICK                                           = 0x50,
    SWITCH                                          = 0x51,
    COWBELL                                         = 0x52,
    NOTCH                                           = 0x53,
    HIHAT                                           = 0x54,
    KICKDRUM                                        = 0x55,
    POP                                             = 0x56,
    CLACK                                           = 0x57,
    CHACK                                           = 0x58,
    MUTE                                            = 0x60,
    UNMUTE                                          = 0x61
  };

  enum note_t {
    END_SONG                                        = 0xFF,
    REST                                            = 0x00,

    NOTE_C1                                         = 0x18, // 24
    NOTE_C1S                                        = 0x19,
    NOTE_D1                                         = 0x1A,
    NOTE_D1S                                        = 0x1B,
    NOTE_E1                                         = 0x1C,
    NOTE_F1                                         = 0x1D,
    NOTE_F1S                                        = 0x1E,
    NOTE_G1                                         = 0x1F,
    NOTE_G1S                                        = 0x20,
    NOTE_A1                                         = 0x21,
    NOTE_A1S                                        = 0x22,
    NOTE_B1                                         = 0x23,

    NOTE_C2                                         = 0x24,  //36
    NOTE_C2S                                        = 0x25,
    NOTE_D2                                         = 0x26,
    NOTE_D2S                                        = 0x27,
    NOTE_E2                                         = 0x28,
    NOTE_F2                                         = 0x29,
    NOTE_F2S                                        = 0x2A,
    NOTE_G2                                         = 0x2B,
    NOTE_G2S                                        = 0x2C,
    NOTE_A2                                         = 0x2D,
    NOTE_A2S                                        = 0x2E,
    NOTE_B2                                         = 0x2F,

    NOTE_C3                                         = 0x30,
    NOTE_C3S                                        = 0x31,
    NOTE_D3                                         = 0x32,
    NOTE_D3S                                        = 0x33,
    NOTE_E3                                         = 0x34,
    NOTE_F3                                         = 0x35,
    NOTE_F3S                                        = 0x36,
    NOTE_G3                                         = 0x37,
    NOTE_G3S                                        = 0x38,
    NOTE_A3                                         = 0x39,
    NOTE_A3S                                        = 0x3A,
    NOTE_B3                                         = 0x3B,

    NOTE_C4                                         = 0x3C,
    NOTE_C4S                                        = 0x3D,
    NOTE_D4                                         = 0x3E,
    NOTE_D4S                                        = 0x3F,
    NOTE_E4                                         = 0x40,
    NOTE_F4                                         = 0x41,
    NOTE_F4S                                        = 0x42,
    NOTE_G4                                         = 0x43,
    NOTE_G4S                                        = 0x44,
    NOTE_A4                                         = 0x45,
    NOTE_A4S                                        = 0x46,
    NOTE_B4                                         = 0x47,

    NOTE_C5                                         = 0x48,
    NOTE_C5S                                        = 0x49,
    NOTE_D5                                         = 0x4A,
    NOTE_D5S                                        = 0x4B,
    NOTE_E5                                         = 0x4C,
    NOTE_F5                                         = 0x4D,
    NOTE_F5S                                        = 0x4E,
    NOTE_G5                                         = 0x4F,
    NOTE_G5S                                        = 0x50,
    NOTE_A5                                         = 0x51,
    NOTE_A5S                                        = 0x52,
    NOTE_B5                                         = 0x53,
  };
}