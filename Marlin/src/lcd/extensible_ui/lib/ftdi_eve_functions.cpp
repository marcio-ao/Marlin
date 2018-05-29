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

#include "ui.h"

#if defined(EXTENSIBLE_UI)

#include "ftdi_eve_panels.h"
#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"

#define MULTIPLE_OF_4(val) ((((val)+3)>>2)<<2)

using namespace FTDI;

void CLCD::enable (void) {
  mem_write_8(REG_PCLK, Pclk);
}

void CLCD::disable (void) {
  mem_write_8(REG_PCLK, 0x00);
}

void CLCD::set_backlight (uint16_t Freq, uint8_t Duty) {
  mem_write_16(REG_PWM_HZ, Freq);
  mem_write_8(REG_PWM_DUTY, Duty);
}

void CLCD::set_brightness (uint8_t brightness) {
  mem_write_8(REG_PWM_DUTY, brightness>>1);
}

void CLCD::turn_on_backlight (void) {
  set_backlight(0x00FA, 128);
}

void CLCD::get_font_metrics(uint8_t font, struct FontMetrics &fm) {
  uint32_t rom_fontroot = mem_read_32(ROM_FONT_ADDR);
  mem_read_bulk(rom_fontroot + 148 * (font - 16), (uint8_t*) &fm, 148);
}

// HOST COMMAND FUNCTION

void CLCD::host_cmd (unsigned char host_command, unsigned char byte2) {  // Sends 24-Bit Host Command to LCD
  if(host_command != ACTIVE) {
    host_command |= 0x40;
  }
  spi_select();
  spi_send(host_command);
  spi_send(byte2);
  spi_send(0x00);
  spi_deselect();
}

void CLCD::flash_write_rgb332_bitmap(uint32_t mem_address, const unsigned char* p_rgb332_array, uint16_t num_bytes)
{
  for(unsigned int i = 0; i < num_bytes; i++) {
    unsigned char flash_byte = pgm_read_byte_near(p_rgb332_array + i);
    mem_write_8((mem_address + i), flash_byte);
  }
}

/******************* FT800/810 Co-processor Commands *********************************/

#if defined(USE_FTDI_FT800)
uint32_t CLCD::CommandFifo::command_write_ptr = 0xFFFFFFFFul;
#endif

void CLCD::CommandFifo::cmd(uint32_t cmd32) {
  write((void*)&cmd32, sizeof(uint32_t));
}

void CLCD::CommandFifo::cmd(void* data, uint16_t len) {
  write(data, len);
}

// This sends the a text command to the command preprocessor, must be followed by str()
void CLCD::CommandFifo::button(int16_t x, int16_t y, int16_t w, int16_t h, int16_t font,  uint16_t option) {
  struct {
    int32_t type = CMD_BUTTON;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int16_t font;
    uint16_t option;
  } cmd_data;

  cmd_data.x      = x;
  cmd_data.y      = y;
  cmd_data.w      = w;
  cmd_data.h      = h;
  cmd_data.font   = font;
  cmd_data.option = option;

  cmd( &cmd_data, sizeof(cmd_data) );
}

// This sends the a text command to the command preprocessor, must be followed by str()
void CLCD::CommandFifo::text(int16_t x, int16_t y, int16_t font,  uint16_t options) {
  struct {
    int32_t type = CMD_TEXT;
    int16_t x;
    int16_t y;
    int16_t font;
    uint16_t options;
    } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.font    = font;
  cmd_data.options = options;

  cmd( &cmd_data, sizeof(cmd_data) );
}

// This sends the a toggle command to the command preprocessor, must be followed by str()
void CLCD::CommandFifo::toggle (int16_t x, int16_t y, int16_t w, int16_t font, uint16_t options, bool state) {
  struct {
    int32_t type = CMD_TOGGLE;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t font;
    uint16_t options;
    uint16_t state;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.font    = font;
  cmd_data.options = options;
  cmd_data.state   = state ? 65535 : 0;

  cmd( &cmd_data, sizeof(cmd_data) );
}

// This sends the a keys command to the command preprocessor, must be followed by str()
void CLCD::CommandFifo::keys (int16_t x, int16_t y, int16_t w, int16_t h, int16_t font, uint16_t options) {
  struct {
    int32_t type = CMD_KEYS;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    int16_t font;
    uint16_t options;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.h       = h;
  cmd_data.font    = font;
  cmd_data.options = options;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::clock (int16_t x, int16_t y, int16_t r, uint16_t options, int16_t h, int16_t m, int16_t s, int16_t ms)
{
  struct {
    int32_t type = CMD_CLOCK;
    int16_t x;
    int16_t y;
    int16_t r;
    uint16_t options;
    int16_t h;
    int16_t m;
    int16_t s;
    int16_t ms;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.r       = r;
  cmd_data.options = options;
  cmd_data.h       = h;
  cmd_data.m       = m;
  cmd_data.s       = s;
  cmd_data.ms      = ms;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::gauge (int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t major, uint16_t minor, uint16_t val, uint16_t range)
{
  struct {
    int32_t  type = CMD_GAUGE;
    int16_t  x;
    int16_t  y;
    int16_t  r;
    uint16_t options;
    uint16_t major;
    uint16_t minor;
    uint16_t val;
    uint16_t range;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.r       = r;
  cmd_data.options = options;
  cmd_data.major   = major;
  cmd_data.minor   = minor;
  cmd_data.val     = val;
  cmd_data.range   = range;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::dial (int16_t x, int16_t y, int16_t r, uint16_t options, uint16_t val)
{
  struct {
    int32_t  type = CMD_DIAL;
    int16_t  x;
    int16_t  y;
    int16_t  r;
    uint16_t options;
    uint16_t val;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.r       = r;
  cmd_data.options = options;
  cmd_data.val     = val;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::scrollbar (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t size, uint16_t range) {
  struct {
    int32_t  type = CMD_SCROLLBAR;
    int16_t  x;
    int16_t  y;
    int16_t  w;
    uint16_t h;
    uint16_t options;
    uint16_t val;
    uint16_t size;
    uint16_t range;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.h       = h;
  cmd_data.options = options;
  cmd_data.val     = val;
  cmd_data.size    = size;
  cmd_data.range   = range;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::progress (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options,  uint16_t val, uint16_t range) {
  struct {
    int32_t  type = CMD_PROGRESS;
    int16_t  x;
    int16_t  y;
    int16_t  w;
    int16_t  h;
    uint16_t options;
    uint16_t val;
    uint16_t range;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.h       = h;
  cmd_data.options = options;
  cmd_data.val     = val;
  cmd_data.range   = range;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::slider (int16_t x, int16_t y, int16_t w, int16_t h, uint16_t options, uint16_t val, uint16_t range) {
  struct {
    int32_t type = CMD_SLIDER;
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    uint16_t options;
    uint16_t val;
    uint16_t range;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.h       = h;
  cmd_data.options = options;
  cmd_data.val     = val;
  cmd_data.range   = range;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::memcpy (uint32_t dst, uint32_t src, uint32_t size) {
  struct {
    uint32_t  type = CMD_MEMCPY;
    uint32_t  dst;
    uint32_t  src;
    uint32_t  size;
  } cmd_data;

  cmd_data.dst    = dst;
  cmd_data.src    = src;
  cmd_data.size   = size;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::append (uint32_t ptr, uint32_t size) {
  struct {
    uint32_t  type = CMD_APPEND;
    uint32_t  ptr;
    uint32_t  size;
  } cmd_data;

  cmd_data.ptr    = ptr;
  cmd_data.size   = size;

  cmd( &cmd_data, sizeof(cmd_data) );
}

void CLCD::CommandFifo::track(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t tag) {
  struct {
    uint32_t type = CMD_TRACK;
    int16_t  x;
    int16_t  y;
    int16_t  w;
    int16_t  h;
    int16_t  tag;
  } cmd_data;

  cmd_data.x       = x;
  cmd_data.y       = y;
  cmd_data.w       = w;
  cmd_data.h       = h;
  cmd_data.tag     = tag;

  cmd( &cmd_data, sizeof(cmd_data) );
}

#if defined(USE_FTDI_FT810)
void CLCD::CommandFifo::set_rotate (uint8_t rotation) {
  struct {
    uint32_t  type = CMD_SETROTATE;
    uint32_t  rotation;
  } cmd_data;

  cmd_data.rotation = rotation;

  cmd( &cmd_data, sizeof(cmd_data) );
}
#endif

/**************************** FT800/810 Co-Processor Command FIFO ****************************/

uint32_t CLCD::CommandFifo::get_reg_cmd_write() {
  return mem_read_32(REG_CMD_WRITE) & 0x0FFF;
}

uint32_t CLCD::CommandFifo::get_reg_cmd_read() {
  return mem_read_32(REG_CMD_READ) & 0x0FFF;
}

bool CLCD::CommandFifo::is_idle() {
  return get_reg_cmd_read() == get_reg_cmd_write();
}

void CLCD::CommandFifo::wait_until_idle() {
  #if defined(UI_FRAMEWORK_DEBUG)
    const uint32_t startTime = millis();
  #endif
  do {
    #if defined(UI_FRAMEWORK_DEBUG)
      if(millis() - startTime > 3) {
        #if defined (SERIAL_PROTOCOLLNPGM)
          SERIAL_PROTOCOLLNPGM("Timeout on CommandFifo::Wait_Until_Idle()");
        #else
          Serial.println(F("Timeout on CommandFifo::Wait_Until_Idle()"));
        #endif
        break;
      }
    #endif
  } while(!is_idle());
}

#if defined(USE_FTDI_FT800)
void CLCD::CommandFifo::start() {
  if(command_write_ptr == 0xFFFFFFFFul) {
    command_write_ptr = get_reg_cmd_write();
  }
}

void CLCD::CommandFifo::execute() {
  if(command_write_ptr != 0xFFFFFFFFul) {
    mem_write_32(REG_CMD_WRITE, command_write_ptr);
  }
}

void CLCD::CommandFifo::reset() {
  mem_write_32(REG_CMD_WRITE, 0x00000000);
  mem_write_32(REG_CMD_READ,  0x00000000);
  command_write_ptr = 0xFFFFFFFFul;
};

template <class T> void CLCD::CommandFifo::_write_unaligned(T data, uint16_t len) {
  const char *ptr = (const char*)data;
  uint32_t bytes_tail, bytes_head;
  uint32_t command_read_ptr;

  #if defined(UI_FRAMEWORK_DEBUG)
  if(command_write_ptr == 0xFFFFFFFFul) {
    #if defined (SERIAL_PROTOCOLLNPGM)
      SERIAL_PROTOCOLLNPGM("Attempt to write to FIFO before CommandFifo::Cmd_Start().");
    #else
      Serial.println(F("Attempt to write to FIFO before CommandFifo::Cmd_Start()."));
    #endif
  }
  #endif

  /* Wait until there is enough space in the circular buffer for the transfer */
  do {
    command_read_ptr = get_reg_cmd_read();
    if (command_read_ptr <= command_write_ptr) {
      bytes_tail = 4096U - command_write_ptr;
      bytes_head = command_read_ptr;
    } else {
      bytes_tail = command_read_ptr - command_write_ptr;
      bytes_head = 0;
    }
  } while((bytes_tail + bytes_head) < len);

  /* Write as many bytes as possible following REG_CMD_WRITE */
  uint16_t bytes_to_write = min(len, bytes_tail);
  mem_write_bulk (RAM_CMD + command_write_ptr, T(ptr), bytes_to_write);
  command_write_ptr += bytes_to_write;
  ptr  += bytes_to_write;
  len  -= bytes_to_write;

  if(len > 0) {
    /* Write remaining bytes at start of circular buffer */
    mem_write_bulk (RAM_CMD, T(ptr), len);
    command_write_ptr = len;
  }

  if(command_write_ptr == 4096U) {
    command_write_ptr = 0;
  }
}

// Writes len bytes into the FIFO, if len is not
// divisible by four, zero bytes will be written
// to align to the boundary.

template <class T> void CLCD::CommandFifo::write(T data, uint16_t len) {
  const uint8_t padding = MULTIPLE_OF_4(len) - len;

  uint8_t pad_bytes[] = {0, 0, 0, 0};
  _write_unaligned(data,      len);
  _write_unaligned(pad_bytes, padding);
}
#else
uint32_t CLCD::CommandFifo::getRegCmdBSpace() {
  return mem_read_32(REG_CMDB_SPACE)  & 0x0FFF;
}

void CLCD::CommandFifo::start() {
}

void CLCD::CommandFifo::execute() {
}

void CLCD::CommandFifo::reset() {
  mem_write_32(REG_CMD_WRITE, 0x00000000);
  mem_write_32(REG_CMD_READ,  0x00000000);
};

// Writes len bytes into the FIFO, if len is not
// divisible by four, zero bytes will be written
// to align to the boundary.

template <class T> void CLCD::CommandFifo::write(T data, uint16_t len) {
  const uint8_t padding = MULTIPLE_OF_4(len) - len;

  // The FT810 provides a special register that can be used
  // for writing data without us having to do our own FIFO
  // management.
  uint32_t Command_Space = getRegCmdBSpace();
  while(Command_Space < len + padding) {
    Command_Space = getRegCmdBSpace();
  }
  mem_write_bulk(REG_CMDB_WRITE, data, len, padding);
}
#endif

template void CLCD::CommandFifo::write(void*, uint16_t);
template void CLCD::CommandFifo::write(progmem_str, uint16_t);

// CO_PROCESSOR COMMANDS

void CLCD::CommandFifo::str (const char * const data) {
  write(data, strlen(data)+1);
}

void CLCD::CommandFifo::str (progmem_str data) {
  write(data, strlen_P((const char*)data)+1);
}

/******************* LCD INITIALIZATION ************************/

void CLCD::init (void) {
  spi_init();                                  // Set Up I/O Lines for SPI and FT800/810 Control
  reset();                                    // Power down/up the FT8xx with the apropriate delays

  if(Use_Crystal == 1) {
    host_cmd(CLKEXT, 0);
  }
  else {
    host_cmd(CLKINT, 0);
  }

  host_cmd(ACTIVE, 0);                        // Activate the System Clock

  /* read the device-id until it returns 0x7c or times out, should take less than 150ms */
  uint8_t counter;
  for(counter=0;counter<250;counter++) {
   uint8_t device_id = mem_read_8(REG_ID);            // Read Device ID, Should Be 0x7C;
   if(device_id == 0x7c) {
     break;
   }
   else {
     delay(1);
   }
   if(counter == 250) {
     #if defined(UI_FRAMEWORK_DEBUG)
       #if defined (SERIAL_ECHOLNPAIR)
         SERIAL_ECHOLNPAIR("Timeout waiting for device ID, should be 124, got ", device_id);
       #else
         Serial.print(F("Timeout waiting for device ID, should be 7C, got "));
         Serial.println(device_id, HEX);
       #endif
     #endif
   }
  }

  mem_write_8(REG_PWM_DUTY, 0);   // turn off Backlight, Frequency already is set to 250Hz default

  /* Configure the FT8xx Registers */
  mem_write_16(REG_HCYCLE,  Hcycle);
  mem_write_16(REG_HOFFSET, Hoffset);
  mem_write_16(REG_HSYNC0,  Hsync0);
  mem_write_16(REG_HSYNC1,  Hsync1);
  mem_write_16(REG_VCYCLE,  Vcycle);
  mem_write_16(REG_VOFFSET, Voffset);
  mem_write_16(REG_VSYNC0,  Vsync0);
  mem_write_16(REG_VSYNC1,  Vsync1);
  mem_write_16(REG_HSIZE,   Hsize);
  mem_write_16(REG_VSIZE,   Vsize);
  mem_write_8(REG_SWIZZLE,  Swizzle);
  mem_write_8(REG_PCLK_POL, Pclkpol);
  mem_write_8(REG_CSPREAD,  CSpread);

  /* write a basic display-list to get things started */
	mem_write_32(RAM_DL,      DL::CLEAR_COLOR_RGB);
	mem_write_32(RAM_DL + 4, (DL::CLEAR | 0x07)); /* clear color, stencil and tag buffer */
	mem_write_32(RAM_DL + 8,  DL::DL_DISPLAY);	/* end of display list */

  mem_write_8(REG_DLSWAP, 0x02); // activate display list, Bad Magic Cookie 2 = switch to new list after current frame is scanned out

  //mem_write_8(REG_TOUCH_MODE, 0x03);      // Configure the Touch Screen, Bad Magic Cookie, 3 = CONTINUOUS = Reset Default
  //mem_write_8(REG_TOUCH_ADC_MODE, 0x01);  // Bad Magic Cookie, 1 = single touch = Reset Default
  //mem_write_8(REG_TOUCH_OVERSAMPLE, 0x0F); // Reset Default = 7 - why 15?
  mem_write_16(REG_TOUCH_RZTHRESH, touch_threshold); /* setup touch sensitivity */
  mem_write_8(REG_VOL_SOUND, 0x00);       // Turn Synthesizer Volume Off

  /* turn on the display by setting DISP high */
  /* turn on the Audio Amplifier by setting GPIO_1 high for the select few modules supporting this */
  /* no need to use GPIOX here since DISP/GPIO_0 and GPIO_1 are on REG_GPIO for FT81x as well */
  if(GPIO_1_Audio_Shutdown) {
    mem_write_8(REG_GPIO_DIR,   GPIO_DISP  | GPIO_GP1);
    mem_write_8(REG_GPIO,       GPIO_DISP  | GPIO_GP1);
  }
  else {
    mem_write_8(REG_GPIO, GPIO_DISP); /* REG_GPIO_DIR is set to output for GPIO_DISP by default */
  }

  mem_write_8(REG_PCLK, Pclk); // Turns on Clock by setting PCLK Register to the value necessary for the module

  // Initialize the command FIFO
  CommandFifo::reset();

  // Set Initial Values for Touch Transform Registers
  mem_write_32(REG_TOUCH_TRANSFORM_A, default_transform_a);
  mem_write_32(REG_TOUCH_TRANSFORM_B, default_transform_b);
  mem_write_32(REG_TOUCH_TRANSFORM_C, default_transform_c);
  mem_write_32(REG_TOUCH_TRANSFORM_D, default_transform_d);
  mem_write_32(REG_TOUCH_TRANSFORM_E, default_transform_e);
  mem_write_32(REG_TOUCH_TRANSFORM_F, default_transform_f);

  #if defined(USE_FTDI_FT810)
    // Set the initial display orientation. On the FT810, we use the command
    // processor to do this since it will also update the transform matrices.

    CommandFifo cmd;
    #if   defined(USE_PORTRAIT_ORIENTATION)  &&  defined(USE_INVERTED_ORIENTATION) &&  defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(7);
    #elif defined(USE_PORTRAIT_ORIENTATION)  && !defined(USE_INVERTED_ORIENTATION) &&  defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(6);
    #elif !defined(USE_PORTRAIT_ORIENTATION) &&  defined(USE_INVERTED_ORIENTATION) &&  defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(5);
    #elif !defined(USE_PORTRAIT_ORIENTATION) && !defined(USE_INVERTED_ORIENTATION) &&  defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(4);
    #elif  defined(USE_PORTRAIT_ORIENTATION) &&  defined(USE_INVERTED_ORIENTATION) && !defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(3);
    #elif defined(USE_PORTRAIT_ORIENTATION)  && !defined(USE_INVERTED_ORIENTATION) && !defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(2);
    #elif !defined(USE_PORTRAIT_ORIENTATION) &&  defined(USE_INVERTED_ORIENTATION) && !defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(1);
    #else // !defined(USE_PORTRAIT_ORIENTATION) && !defined(USE_INVERTED_ORIENTATION) && !defined(USE_MIRRORED_ORIENTATION)
    cmd.set_rotate(0);
    #endif
    cmd.execute();
  #endif

  #if defined(USE_FTDI_FT800) &&  defined(USE_INVERTED_ORIENTATION)
    mem_write_32(REG_ROTATE, 1);
  #endif
}

#endif // EXTENSIBLE_UI