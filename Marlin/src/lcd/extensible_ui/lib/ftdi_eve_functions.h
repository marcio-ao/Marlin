/**************************
 * ftdi_eve_functions.cpp *
 **************************/

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
  *                       FUNCTION MAP                                        *
  *                                                                           *
  * SPI and FT800/810 Commands                                                *
  *                                                                           *
  * CLCD::spi_select()                 Set CS line to 0                       *
  * CLCD::spi_deselect()               Set CS Line to 1                       *
  * CLCD::reset()                      Toggle FT800/810 Power Down Line 50 ms *
  * CLCD::spi_init()                   Configure I/O Lines for SPI            *
  * CLCD::spi_transfer()               Send/Receive 1 SPI Byte                *
  * CLCD::init()                       Set FT800/810 Registers                *
  * CLCD::enable()                     Turn On FT800/810 PCLK                 *
  * CLCD::disable()                    Turn Off FT8880/810 PCLK               *
  * CLCD::set_backlight()              Set LCD Backlight Level                *
  *                                                                           *
  * MEMORY READ FUNCTIONS                                                     *
  *                                                                           *
  * CLCD::mem_read_addr()              Send 32-Bit Address                    *
  * CLCD::mem_read_8()                 Read 1 Byte                            *
  * CLCD::mem_read_16()                Read 2 Bytes                           *
  * CLCD::mem_read_32()                Read 4 Bytes                           *
  *                                                                           *
  * MEMORY WRITE FUNCTIONS                                                    *
  *                                                                           *
  * CLCD::mem_write_addr()             Send 24-Bit Address                    *
  * CLCD::mem_write_8()                Write 1 Byte                           *
  * CLCD::mem_write_16()               Write 2 Bytes                          *
  * CLCD::mem_write_32()               Write 4 Bytes                          *
  *                                                                           *
  * HOST COMMAND FUNCTION                                                     *
  *                                                                           *
  * CLCD::host_cmd()                   Send 24-Bit Host Command               *
  *                                                                           *
  * COMMAND BUFFER FUNCTIONS                                                  *
  *                                                                           *
  * CLCD::cmd()                        Send 32-Bit Value(4 Bytes)CMD Buffer   *
  * CLCD::cmd()                        Send Data Structure with 32-Bit Cmd    *
  * CLCD::str()                        Send Text String in 32-Bit Multiples   *

  *                                                                           *
  * FT800/810 GRAPHIC COMMANDS                                                *
  *                                                                           *
  * class CLCD:CommandFifo {}          Class to control Cmd FIFO              *

  * CommandFifo::start()               Wait for CP finish - Set FIFO Ptr      *
  * CommandFifo::execute()             Set REG_CMD_WRITE and start CP         *
  * CommandFifo::reset()               Set Cmd Buffer Pointers to 0           *
  *
  * CommandFifo::fgcolor               Set Graphic Item Foreground Color      *
  * CommandFifo::bgcolor               Set Graphic Item Background Color      *
  * CommandFifo::begin()               Begin Drawing a Primative              *
  * CommandFifo::mem_copy()            Copy a Block of Memory                 *
  * CommandFifo::append()              Append Commands to Current DL          *
  * CommandFifo::gradient_color()      Set 3D Button Highlight Color          *
  * CommandFifo::button()              Draw Button with Bulk Write            *
  * CommandFifo::text()                Draw Text with Bulk Write              *
  *****************************************************************************/

 /**************************************************
  * RAM_G Graphics RAM Allocation                  *
  *                                                *
  * Address    Use                                 *
  *                                                *
  *    8000    Extruder Bitmap                     *
  *    8100    Bed Heat Bitmap                     *
  *    8200    Fan Bitmap                          *
  *    8300    Thumb Drive Symbol Bitmap           *
  *   35000    Static DL Space (FT800)             *
  *   F5000    Static DL Space (FT810)             *
  **************************************************/

#ifndef _FTDI_EVE_FUNCTIONS_H_
#define _FTDI_EVE_FUNCTIONS_H_

#include "ui_config.h"
#include "ftdi_eve_panels.h"
#include "ftdi_eve_dl.h"

typedef const __FlashStringHelper *progmem_str;

class CLCD {
  private:
    static void     spi_init (void);
    static void     spi_select (void);
    static void     spi_deselect (void);

    static uint8_t  _soft_spi_transfer (uint8_t spiOutByte);
    static void    _soft_spi_send (uint8_t spiOutByte);

    static void     spi_send(uint8_t spiOutByte);
    static uint8_t  spi_recv();

    static void     mem_read_addr  (uint32_t reg_address);
    static void     mem_write_addr (uint32_t reg_address);
    static void     mem_read_bulk  (uint32_t reg_address, uint8_t *data, uint16_t len);
  public:
    static uint8_t  mem_read_8   (uint32_t reg_address);
    static uint16_t mem_read_16  (uint32_t reg_address);
    static uint32_t mem_read_32  (uint32_t reg_address);
    static void     mem_write_8  (uint32_t reg_address, uint8_t w_data);
    static void     mem_write_16 (uint32_t reg_address, uint16_t w_data);
    static void     mem_write_32 (uint32_t reg_address, uint32_t w_data);
    static void     mem_write_bulk (uint32_t reg_address, const void *data, uint16_t len, uint8_t padding = 0);
    static void     mem_write_bulk (uint32_t reg_address, progmem_str str, uint16_t len, uint8_t padding = 0);

    static inline uint32_t pack_rgb(uint8_t r, uint8_t g, uint8_t b) {
      return (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
    }

  public:
    class CommandFifo;

    static void init (void);
    static void turn_on_backlight (void);
    static void reset (void);
    static void test_pulse(void);
    static void enable (void);
    static void disable (void);
    static void set_backlight (uint16_t freq, uint8_t duty);
    static void set_brightness (uint8_t duty);
    static void host_cmd (unsigned char host_command, unsigned char byte2);

    static void get_font_metrics (uint8_t font, struct FontMetrics &fm);
    static void flash_write_rgb332_bitmap (uint32_t mem_address, const unsigned char* p_rgb332_array, uint16_t num_bytes);

    static uint8_t get_tag ()     {return mem_read_8(FTDI::REG_TOUCH_TAG);}
    static bool is_touching ()    {return (mem_read_32(FTDI::REG_TOUCH_DIRECT_XY) & 0x80000000) == 0;}

    static uint8_t get_tracker (uint16_t &value) {
      uint32_t tracker = mem_read_32(FTDI::REG_TRACKER);
      value            = tracker >> 16;
      return tracker & 0xFF;
    }
};

/********************************* FT800/810 Commands *********************************/

struct FontMetrics {
  uint8_t   char_widths[128];
  uint32_t  format;
  uint32_t  stride;
  uint32_t  width;
  uint32_t  height;
  uint32_t  ptr;
};

/******************* FT800/810 Graphic Commands *********************************/

class CLCD::CommandFifo {
  protected:
    #if defined(USE_FTDI_FT800)
      static uint32_t command_write_ptr;
      template <class T> void _write_unaligned(T data, uint16_t len);
    #else
      uint32_t getRegCmdBSpace();
    #endif
    void start(void);

  public:
    template <class T> void write(T data, uint16_t len);

  public:
    CommandFifo() {start();}

    static void reset (void);
    static bool is_processing();

    void execute(void);

    void cmd(uint32_t cmd32);
    void cmd(void* data, uint16_t len);

    #if defined(USE_FTDI_FT810)
    void set_rotate(uint8_t rotation);
    #endif

    void track     (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t tag);
    void clock     (int16_t x, int16_t y, int16_t r,            uint16_t options, int16_t h, int16_t m, int16_t s, int16_t ms);
    void gauge     (int16_t x, int16_t y, int16_t r,            uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range);
    void dial      (int16_t x, int16_t y, int16_t r,            uint16_t options, uint16_t val);
    void slider    (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range);
    void progress  (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range);
    void scrollbar (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range);
    void sketch    (int16_t x, int16_t y, uint16_t w, uint16_t h, uint32_t ptr, uint16_t format);
    void mediafifo (uint32_t ptr, uint32_t size);
    void playvideo (uint32_t options);
    void videostart();
    void videoframe(uint32_t dst, uint32_t ptr);

    // All the following must be followed by str()
    void text      (int16_t x, int16_t y,                       int16_t font, uint16_t options);
    void button    (int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t option);
    void toggle    (int16_t x, int16_t y, int16_t w,            int16_t font, uint16_t options, bool state);
    void keys      (int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options);

    // Sends the string portion of text, button, toggle and keys.
    void str (const char * const data);
    void str (progmem_str data);

    void memcpy (uint32_t dst, uint32_t src, uint32_t size);
    void append (uint32_t ptr, uint32_t size);
};

#endif // _FTDI_EVE_FUNCTIONS_H_