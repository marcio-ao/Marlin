/********************
 * ftdi_eve_spi.cpp *
 ********************/

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

#if ENABLED(EXTENSIBLE_UI)

#include "ftdi_eve_panels.h"
#include "ftdi_eve_pins.h"
#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"

/*************************** I/O COMPATIBILITY ******************************/

// The following functions allow this code to be used outside of Marlin (i.e.
// in Arduino sketches)

#if defined(USE_FAST_AVR_IO)
  // If the following is defined, the pin definitions can be
  // given as a pairing of port and bitmask, as opposed to
  // Arduino pin numbers, for faster I/O on AVR chips.
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

  #define CLCD_USE_SOFT_SPI // Hardware SPI not implemented yet

#elif !defined(SET_OUTPUT)
  // Use standard Arduino Wire library

  #include <Wire.h>
  #if !defined(CLCD_USE_SOFT_SPI)
      #include "SPI.h"
  #endif

  #define SET_OUTPUT(p)             pinMode(p, OUTPUT);
  #define SET_INPUT_PULLUP(p)       pinMode(p, INPUT_PULLUP);
  #define SET_INPUT(p)              pinMode(p, INPUT);
  #define WRITE(p,v)                digitalWrite(p, v ? HIGH : LOW);
  #define READ(p)                   digitalRead(p)
#endif

/********************************* SPI Functions *********************************/

void CLCD::spi_init (void) {
  SET_OUTPUT(CLCD_MOD_RESET); // Module Reset (a.k.a. PD, not SPI)
  WRITE(CLCD_MOD_RESET, 0); // start with module in power-down

  SET_OUTPUT(CLCD_SPI_CS);
  WRITE(CLCD_SPI_CS, 1);

#if defined(CLCD_USE_SOFT_SPI)
  SET_OUTPUT(CLCD_SOFT_SPI_MOSI);
  WRITE(CLCD_SOFT_SPI_MOSI, 1);

  SET_OUTPUT(CLCD_SOFT_SPI_SCLK);
  WRITE(CLCD_SOFT_SPI_SCLK, 0);

  SET_INPUT_PULLUP(CLCD_SOFT_SPI_MISO);
#elif defined(USE_MARLIN_IO)
  spiBegin();
  spiInit(SPI_SPEED);
#else
  SPI.begin();
  SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
#endif

}

// CLCD SPI - Chip Select
void CLCD::spi_select (void) {
  WRITE(CLCD_SPI_CS, 0);
  delayMicroseconds(1);
}

// CLCD SPI - Chip Deselect
void CLCD::spi_deselect (void) {
  WRITE(CLCD_SPI_CS, 1);
}

void CLCD::reset (void) {
  WRITE(CLCD_MOD_RESET, 0);
  delay(6); /* minimum time for power-down is 5ms */
  WRITE(CLCD_MOD_RESET, 1);
  delay(21); /* minimum time to allow from rising PD_N to first access is 20ms */
}

void CLCD::test_pulse(void)
{
  #if defined(CLCD_AUX_0)
    WRITE(CLCD_AUX_0, 1);
    delayMicroseconds(10);
    WRITE(CLCD_AUX_0, 0);
  #endif
}

#if defined(CLCD_USE_SOFT_SPI)
  uint8_t CLCD::_soft_spi_transfer (uint8_t spiOutByte) {
    uint8_t spiIndex  = 0x80;
    uint8_t spiInByte = 0;
    uint8_t k;

    for(k = 0; k <8; k++) {         // Output and Read each bit of spiOutByte and spiInByte
      if(spiOutByte & spiIndex) {   // Output MOSI Bit
        WRITE(CLCD_SOFT_SPI_MOSI, 1);
      }
      else {
        WRITE(CLCD_SOFT_SPI_MOSI, 0);
      }
      WRITE(CLCD_SOFT_SPI_SCLK, 1);   // Pulse Clock
      WRITE(CLCD_SOFT_SPI_SCLK, 0);

      if(READ(CLCD_SOFT_SPI_MISO)) {
        spiInByte |= spiIndex;
      }

      spiIndex >>= 1;
    }
    return spiInByte;
  }
#endif

#if defined(CLCD_USE_SOFT_SPI)
  void CLCD::_soft_spi_send (uint8_t spiOutByte) {
    uint8_t spiIndex  = 0x80;
    uint8_t k;

    for(k = 0; k <8; k++) {         // Output each bit of spiOutByte
      if(spiOutByte & spiIndex) {   // Output MOSI Bit
        WRITE(CLCD_SOFT_SPI_MOSI, 1);
      }
      else {
        WRITE(CLCD_SOFT_SPI_MOSI, 0);
      }
      WRITE(CLCD_SOFT_SPI_SCLK, 1);   // Pulse Clock
      WRITE(CLCD_SOFT_SPI_SCLK, 0);

      spiIndex >>= 1;
    }
  }
#endif

void CLCD::spi_send(uint8_t spiOutByte) {
  #if defined(CLCD_USE_SOFT_SPI)
    _soft_spi_send(spiOutByte);
  #elif defined(USE_MARLIN_IO)
    spiSend(spiOutByte);
  #else
    SPI.transfer(spiOutByte);
  #endif
}

uint8_t CLCD::spi_recv() {
  #if defined(CLCD_USE_SOFT_SPI)
    return _soft_spi_transfer(0x00);
  #elif defined(USE_MARLIN_IO)
    return spiRec();
  #else
    return SPI.transfer(0x00);
  #endif
}

// MEMORY READ FUNCTIONS

// Write 4-Byte Address
void CLCD::mem_read_addr (uint32_t reg_address) {
  spi_send((reg_address >> 16) & 0x3F);  // Address [21:16]
  spi_send((reg_address >> 8 ) & 0xFF);  // Address [15:8]
  spi_send((reg_address >> 0)  & 0xFF);  // Address [7:0]
  spi_send(0x00);                        // Dummy Byte
}

// Write 4-Byte Address, Read Multiple Bytes
void CLCD::mem_read_bulk (uint32_t reg_address, uint8_t *data, uint16_t len) {
  spi_select();
  mem_read_addr(reg_address);
  #if defined(USE_MARLIN_IO)
    spiRead(data, len);
  #else
    while(len--) {
      *data++ = spi_recv();
    }
  #endif
  spi_deselect();
}

// Write 4-Byte Address, Read 1-Byte Data
uint8_t CLCD::mem_read_8 (uint32_t reg_address) {
  spi_select();
  mem_read_addr(reg_address);
  uint8_t r_data = spi_recv();
  spi_deselect();
  return r_data;
}

// Write 4-Byte Address, Read 2-Bytes Data
uint16_t CLCD::mem_read_16 (uint32_t reg_address) {
  spi_select();
  mem_read_addr(reg_address);
  uint16_t r_data =  (((uint16_t) spi_recv()) << 0) |
                     (((uint16_t) spi_recv()) << 8);
  spi_deselect();
  return r_data;
}

// Write 4-Byte Address, Read 4-Bytes Data
uint32_t CLCD::mem_read_32 (uint32_t reg_address) {
  spi_select();
  mem_read_addr(reg_address);
  uint32_t r_data =  (((uint32_t) spi_recv()) <<  0) |
                     (((uint32_t) spi_recv()) <<  8) |
                     (((uint32_t) spi_recv()) << 16) |
                     (((uint32_t) spi_recv()) << 24);
  spi_deselect();
  return r_data;
}

// MEMORY WRITE FUNCTIONS

 // Write 3-Byte Address
void CLCD::mem_write_addr (uint32_t reg_address) {
  spi_send((reg_address >> 16) | 0x80); // Address [21:16]
  spi_send((reg_address >> 8 ) & 0xFF); // Address [15:8]
  spi_send((reg_address >> 0)  & 0xFF); // Address [7:0]
}

// Write 3-Byte Address, Multiple Bytes, plus padding bytes
void CLCD::mem_write_bulk (uint32_t reg_address, const void *data, uint16_t len, uint8_t padding) {
  const uint8_t* p = (const uint8_t *)data;
  spi_select();
  mem_write_addr(reg_address);
  // Write data bytes
  while(len--) {
    #if defined(USE_MARLIN_IO)
      spiSend(*p++);
    #else
      spi_send(*p++);
    #endif
  }
  // Write padding bytes
  while(padding--) {
    spi_send(0);
  }
  spi_deselect();
}

void CLCD::mem_write_bulk (uint32_t reg_address, progmem_str str, uint16_t len, uint8_t padding) { // Write 3-Byte Address, Multiple Bytes, plus padding bytes
  const uint8_t* p = (const uint8_t *) str;
  spi_select();
  mem_write_addr(reg_address);
  // Write data bytes
  while(len--) {
    #if defined(USE_MARLIN_IO)
      spiSend(pgm_read_byte(p++));
    #else
      spi_send(pgm_read_byte(p++));
    #endif
  }
  // Write padding bytes
  while(padding--) {
    spi_send(0);
  }
  spi_deselect();
}

// Write 3-Byte Address, Write 1-Byte Data
void CLCD::mem_write_8 (uint32_t reg_address, uint8_t w_data) {
  spi_select();
  mem_write_addr(reg_address);
  spi_send(w_data);
  spi_deselect();
}

// Write 3-Byte Address, Write 2-Bytes Data
void CLCD::mem_write_16 (uint32_t reg_address, uint16_t w_data) {
  spi_select();
  mem_write_addr(reg_address);
  spi_send((uint8_t) ((w_data >> 0) & 0x00FF));
  spi_send((uint8_t) ((w_data >> 8) & 0x00FF));
  spi_deselect();
}

// Write 3-Byte Address, Write 4-Bytes Data
void CLCD::mem_write_32 (uint32_t reg_address, uint32_t w_data) {
  spi_select();
  mem_write_addr(reg_address);
  spi_send(w_data >> 0);
  spi_send(w_data >> 8);
  spi_send(w_data >> 16);
  spi_send(w_data >> 24);
  spi_deselect();
}

#endif // EXTENSIBLE_UI