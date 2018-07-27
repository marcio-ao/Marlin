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
    static bool is_present;

    static void check_device();
    static void wait_while_busy();

  public:
    enum error_t {
      SUCCESS,
      FILE_NOT_FOUND,
      READ_ERROR,
      VERIFY_ERROR
    };

    static void initialize  ();

    static void write_data  (const void *data, size_t size);
    static bool verify_data (const void *data, size_t size);
    static void read_data   (void *data, size_t size);
    static void erase_data  ();

    static error_t write_file  (progmem_str file);

    class BootMediaReader;
};

class UIStorage::BootMediaReader {
  private:
    uint32_t addr;
    uint32_t bytes_remaining;

  public:
    bool isAvailable();
    int16_t read(void *buffer, size_t const size);

    static int16_t read(void *obj, void *buffer, const size_t size);
};
#endif // EXTENSIBLE_UI