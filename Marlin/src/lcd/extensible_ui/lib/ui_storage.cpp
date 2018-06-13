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

#include "ui.h"

#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"

#include "ui_storage.h"

#if ENABLED(EXTENSIBLE_UI)

bool UIStorage::is_present = false;

#ifdef SPI_FLASH_SS
  void UIStorage::spi_select () {
    WRITE(SPI_FLASH_SS, 0);
    delayMicroseconds(1);
  }

  void UIStorage::spi_deselect () {
    WRITE(SPI_FLASH_SS, 1);
  }

  void UIStorage::initialize() {
    SET_OUTPUT(SPI_FLASH_SS);
    WRITE(SPI_FLASH_SS, 1);

    checkDeviceID();
  }

  void UIStorage::waitWhileBusy() {
    uint8_t status;
    do {
     spi_select();
     CLCD::spi_send(READ_STATUS_1);
     status = CLCD::spi_recv();
     spi_deselect();
     safe_delay(1);
    } while(status & 1);
  }

  void UIStorage::checkDeviceID() {
    spi_select();
    CLCD::spi_send(READ_JEDEC_ID);
    const uint8_t manufacturer_id = CLCD::spi_recv();
    const uint8_t device_type     = CLCD::spi_recv();
    const uint8_t capacity        = CLCD::spi_recv();
    spi_deselect ();

    if((manufacturer_id == 0x01) && (device_type == 0x40) && (capacity == 0x15)) {
      is_present = true;
    } else {
      is_present = false;
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("Unable to locate Cypress S25FL116K SPI Flash Memory.");
        SERIAL_PROTOCOLLNPAIR("  Manufacturer ID != 0x01, got: ", manufacturer_id);
        SERIAL_PROTOCOLLNPAIR("  Device Type     != 0x40, got: ", device_type);
        SERIAL_PROTOCOLLNPAIR("  Capacity        != 0x15, got: ", capacity);
      #else
        Serial.println(F("Unable to locate Cypress S25FL116K SPI Flash Memory."));
      #endif
    }
  }

  bool UIStorage::verifyPersistentData(const void *data, size_t size) {
    if(!is_present) return false;

    const uint8_t *byte = (const uint8_t *) data;

    spi_select();
    CLCD::spi_send(READ_DATA);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    while(size--) {
      if(*byte++ != CLCD::spi_recv()) {
        spi_deselect();
        return false;
      }
    }
    spi_deselect();
    return true;
  }

  void UIStorage::readPersistentData(void *data, size_t size) {
    if(!is_present) return;

    uint8_t *byte = (uint8_t *) data;

    spi_select();
    CLCD::spi_send(READ_DATA);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    while(size--) {
      *byte++ = CLCD::spi_recv();
    }
    spi_deselect();
  }

  // Write persistent data, up to 4k, in the first sector of
  // the SPI flash.

  void UIStorage::writePersistentData(const void *data, size_t size) {
    if(!is_present) {
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("UIStorage: SPI Flash chip not present. Not saving UI settings.");
      #else
        Serial.println(F("UIStorage: SPI Flash chip not present. Not saving UI settings."));
      #endif
      return;
    }

    // Since Flash storage has a limited number of write cycles,
    // make sure that the data is different before rewriting.

    if(verifyPersistentData(data, size)) {
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("UIStorage: Persistent data already written, skipping write.");
      #else
        Serial.println(F("UIStorage: Persistent data already written, skipping write."));
      #endif
      return;
    }

    #if defined (SERIAL_PROTOCOLPGM)
      SERIAL_PROTOCOLPGM("UIStorage: Writing persistent data to SPI Flash...");
    #else
      Serial.print(F("UIStorage: Writing persistent data to SPI Flash..."));
    #endif

    waitWhileBusy();

    spi_select();
    CLCD::spi_send(WRITE_ENABLE);
    spi_deselect();

    spi_select();
    CLCD::spi_send(SECTOR_ERASE);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    spi_deselect();

    waitWhileBusy();

    spi_select();
    CLCD::spi_send(WRITE_ENABLE);
    spi_deselect();

    spi_select();
    CLCD::spi_send(PAGE_PROGRAM);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    CLCD::spi_send(0);
    for(const uint8_t *byte = (const uint8_t *) data; size ;size--) {
      CLCD::spi_send(*byte++);
    }
    spi_deselect();

    waitWhileBusy();

    #if defined (SERIAL_PROTOCOLLNPGM)
      SERIAL_PROTOCOLLNPGM("DONE");
    #else
      Serial.println(F("DONE"));
    #endif
  }
#else
  void UIStorage::initialize() {}
  void UIStorage::writePersistentData(const void *data, size_t size)  {}
  bool UIStorage::verifyPersistentData(const void *data, size_t size) {return true;}
  void UIStorage::readPersistentData(void *data, size_t size)         {}
#endif // SPI_FLASH_SS
#endif // EXTENSIBLE_UI