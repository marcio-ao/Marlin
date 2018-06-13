/****************
 * ui_storage.h *
 ****************/

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

#ifndef _UI_STORAGE_
#define _UI_STORAGE_

class UIStorage {
  private:
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

    static bool is_present;

    static void spi_select();
    static void spi_deselect();

    static void checkDeviceID();

    static void waitWhileBusy();
  public:
    static void initialize();

    static void writePersistentData  (const void *data, size_t size);
    static bool verifyPersistentData (const void *data, size_t size);
    static void readPersistentData   (void *data, size_t size);
};

#endif // EXTENSIBLE_UI