/******************
 * ui_storage.cpp *
 ******************/

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

#include "../../../sd/SdFile.h"
#include "../../../sd/cardreader.h"

#include "ui.h"

#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"
#include "ftdi_eve_spi.h"

#include "ui_filereader.h"
#include "ui_storage.h"

#if ENABLED(EXTENSIBLE_UI)

using namespace SPI;
using namespace SPI::most_significant_byte_first;

bool UIStorage::is_present = false;

enum {
  READ_STATUS_1 = 0x05,
  READ_STATUS_2 = 0x35,
  READ_STATUS_3 = 0x33,
  WRITE_ENABLE  = 0x06,
  WRITE_DISABLE = 0x04,
  READ_ID       = 0x90,
  READ_JEDEC_ID = 0x9F,
  READ_DATA     = 0x03,
  PAGE_PROGRAM  = 0x02,
  SECTOR_ERASE  = 0x20,
  BLOCK_ERASE   = 0xD8,
  CHIP_ERASE    = 0xC7
};

/* If a SPI Flash is present, it is used for storing persistent data
 * for the UI, as well as a boot-time video. The format of the SPI
 * flash is as follows:
 *
 *  0x00000000 -   0x00000FFF  Persistent data for UIStorage
 *  0x00001000 -   0x000010FF  Index, consisting of file sizes
 *          [0]                Size of boot video (0xFFFFFFFF if no video)
 *          [4]                Reserved, 0xFFFFFFFF
 *          ...
 *  0x00001100 ... 0x20000000  File storage area
 */
constexpr uint16_t erase_unit_size = 4 * 1024; // Minimum erase unit
constexpr uint16_t write_page_size = 256;      // Minimum page write unit

constexpr uint32_t data_addr = 0;
constexpr uint32_t indx_addr = erase_unit_size;
constexpr uint32_t file_addr = erase_unit_size + write_page_size;

#ifdef SPI_FLASH_SS
  void UIStorage::initialize() {
    check_device();
  }

  void UIStorage::wait_while_busy() {
    uint8_t status;
    do {
     spi_flash_select();
     spi_write_8(READ_STATUS_1);
     status = spi_read_8();
     spi_flash_deselect();
     safe_delay(1);
    } while(status & 1);
  }

  void UIStorage::check_device() {
    spi_flash_select();
    spi_write_8(READ_JEDEC_ID);
    const uint8_t manufacturer_id = spi_recv();
    const uint8_t device_type     = spi_recv();
    const uint8_t capacity        = spi_recv();
    spi_flash_deselect ();

    if((manufacturer_id == 0x01) && (device_type == 0x40) && (capacity == 0x15)) {
      is_present = true;
    } else {
      is_present = false;
      SERIAL_ECHO_START(); SERIAL_ECHOLNPGM("Unable to locate Cypress S25FL116K SPI Flash Memory.");
      SERIAL_ECHO_START(); SERIAL_ECHOLNPAIR("  Manufacturer ID != 0x01, got: ", manufacturer_id);
      SERIAL_ECHO_START(); SERIAL_ECHOLNPAIR("  Device Type     != 0x40, got: ", device_type);
      SERIAL_ECHO_START(); SERIAL_ECHOLNPAIR("  Capacity        != 0x15, got: ", capacity);
    }
  }

  bool UIStorage::verify_data(const void *data, size_t size) {
    if(!is_present) return false;

    spi_flash_select();
    spi_write_8(READ_DATA);
    spi_write_24(data_addr);
    bool ok = spi_verify_bulk(data,size);
    spi_flash_deselect();
    return ok;
  }

  void UIStorage::read_data(void *data, size_t size) {
    if(!is_present) return;

    spi_flash_select();
    spi_write_8(READ_DATA);
    spi_write_24(data_addr);
    spi_read_bulk (data, size);
    spi_flash_deselect();
  }

  // Write persistent data, up to 4k, in the first sector of
  // the SPI flash.

  void UIStorage::write_data(const void *data, size_t size) {
    if(!is_present) {
      SERIAL_ECHO_START(); SERIAL_ECHOLNPGM("SPI Flash chip not present. Not saving UI settings.");
      return;
    }

    // Since Flash storage has a limited number of write cycles,
    // make sure that the data is different before rewriting.

    if(verify_data(data, size)) {
      SERIAL_ECHO_START(); SERIAL_ECHOLNPGM("Persistent data already written, skipping write.");
      return;
    }

    SERIAL_ECHO_START(); SERIAL_ECHOPGM("Writing persistent data to SPI Flash...");

    wait_while_busy();

    spi_flash_select();
    spi_write_8(WRITE_ENABLE);
    spi_flash_deselect();

    spi_flash_select();
    spi_write_8(SECTOR_ERASE);
    spi_write_24(data_addr);
    spi_flash_deselect();

    wait_while_busy();

    spi_flash_select();
    spi_write_8(WRITE_ENABLE);
    spi_flash_deselect();

    spi_flash_select();
    spi_write_8(PAGE_PROGRAM);
    spi_write_24(data_addr);
    spi_write_bulk<ram_write>(data, size);
    spi_flash_deselect();

    wait_while_busy();

    SERIAL_ECHOLNPGM("DONE");
  }

  void UIStorage::write_file(progmem_str filename) {
    uint32_t addr;
    uint8_t buff[write_page_size];

    strcpy_P( (char*) buff, (const char*) filename);

    MediaFileReader reader;
    if(!reader.open((char*) buff)) {
      SERIAL_ECHO_START(); SERIAL_ECHOLNPGM("Unable to find media file");
      return;
    }

    SERIAL_ECHO_START(); SERIAL_ECHOPGM("Erasing SPI Flash...");

    wait_while_busy();

    // Erase enough of the flash chip for storing the index + file

    const uint32_t index_size  = write_page_size;
    const uint32_t write_size  = index_size + reader.size();
    const uint32_t num_sectors = (write_size + erase_unit_size - 1) / erase_unit_size;
    const uint32_t start_addr  = indx_addr;
    const uint32_t end_addr    = start_addr + num_sectors * erase_unit_size;

    for(addr = start_addr; addr < end_addr; addr += erase_unit_size) {
      spi_flash_select();
      spi_write_8(WRITE_ENABLE);
      spi_flash_deselect();

      spi_flash_select();
      spi_write_8(SECTOR_ERASE);
      spi_write_24(addr);
      spi_flash_deselect();

      wait_while_busy();
    }

    SERIAL_ECHOLNPGM("DONE");

    SERIAL_ECHO_START(); SERIAL_ECHOPGM("Writing SPI Flash...");

    // Write out the index followed by the file

    addr = indx_addr;

    spi_flash_select();
    spi_write_8(WRITE_ENABLE);
    spi_flash_deselect();

    spi_flash_select();
    spi_write_8(PAGE_PROGRAM);
    spi_write_24(addr);
    spi_write_32(reader.size());
    spi_flash_deselect();

    addr = file_addr;

    // Write out the file itself
    for(;;) {
      const int16_t nBytes = reader.read(buff, write_page_size);
      if(nBytes == -1) {
        SERIAL_ECHOLNPGM("Failed to read from file");
        return;
      }

      spi_flash_select();
      spi_write_8(WRITE_ENABLE);
      spi_flash_deselect();

      spi_flash_select();
      spi_write_8(PAGE_PROGRAM);
      spi_write_24(addr);
      spi_write_bulk<ram_write>(buff, nBytes);
      spi_flash_deselect();
      addr += nBytes;

      if(nBytes != write_page_size)
        break;

      wait_while_busy();
      #if ENABLED(EXTENSIBLE_UI)
        Extensible_UI_API::yield();
      #endif
    }

    SERIAL_ECHOLNPGM("DONE");

    SERIAL_ECHO_START(); SERIAL_ECHOPGM("Verifying SPI Flash...");

    bool verifyOk = true;

    // Verify the file index

    addr = indx_addr;

    spi_flash_select();
    spi_write_8(READ_DATA);
    spi_write_24(addr);
    if(spi_read_32() != reader.size()) {
      SERIAL_ECHOPGM("File index verification failed");
      verifyOk = false;
    }
    spi_flash_deselect();

    addr = file_addr;

    // Verify the file itself

    reader.rewind();

    while(verifyOk) {
      const int16_t nBytes = reader.read(buff, write_page_size);
      if(nBytes == -1) {
        SERIAL_ECHOPGM("Failed to read from file");
        verifyOk = false;
        break;
      }

      spi_flash_select();
      spi_write_8(READ_DATA);
      spi_write_24(addr);
      if(!spi_verify_bulk(buff, nBytes)) {
        verifyOk = false;
        break;
      }
      spi_flash_deselect();

      addr += nBytes;
      if(nBytes != write_page_size) break;
      #if ENABLED(EXTENSIBLE_UI)
        Extensible_UI_API::yield();
      #endif
    };
    spi_flash_deselect();

    if(verifyOk) {
      SERIAL_ECHOLNPGM("DONE");
    } else {
      SERIAL_ECHOLNPGM("FAIL");
    }
  }

  bool UIStorage::BootMediaReader::isAvailable() {
    spi_flash_select();
    spi_write_8(READ_DATA);
    spi_write_24(indx_addr);
    bytes_remaining = spi_read_32();
    spi_flash_deselect();

    if(bytes_remaining != 0xFFFFFFFFUL) {
      SERIAL_ECHO_START(); SERIAL_ECHOLNPAIR("Boot media file size:", bytes_remaining);
      addr = file_addr;
    }
    return true;
  }

  int16_t UIStorage::BootMediaReader::read(void *data, const size_t size) {
    if(bytes_remaining == 0xFFFFFFFFUL) return -1;

    if(size > bytes_remaining)
      return read(data, bytes_remaining);

    if(size > 0) {
      spi_flash_select();
      spi_write_8(READ_DATA);
      spi_write_24(addr);
      spi_read_bulk(data, size);
      spi_flash_deselect();
      addr += size;
      bytes_remaining -= size;
    }

    return size;
  }

  int16_t UIStorage::BootMediaReader::read(void *obj, void *data, const size_t size) {
    return reinterpret_cast<UIStorage::BootMediaReader*>(obj)->read(data, size);
  }

#else
  void UIStorage::initialize() {}
  void UIStorage::write_data(const void *, size_t)                       {}
  bool UIStorage::verify_data(const void *, size_t)                      {return false;}
  void UIStorage::read_data(void *, size_t)                              {}
  void UIStorage::write_file(progmem_str)                                {}

  bool UIStorage::BootMediaReader::isAvailable()                         {return false;}
  int16_t UIStorage::BootMediaReader::read(void *, const size_t)         {return -1;}
  int16_t UIStorage::BootMediaReader::read(void *, void *, const size_t) {return -1;}
#endif // SPI_FLASH_SS
#endif // EXTENSIBLE_UI