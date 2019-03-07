/*****************
 * screen_data.h *
 *****************/

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

// To save RAM, store state information related to a particular screen
// in a union. The values should be initialized in the onEntry method.

struct base_numeric_adjustment_t {uint8_t increment;};

union screen_data_t {
  struct base_numeric_adjustment_t             BaseNumericAdjustmentScreen;
  struct {uint8_t volume; uint8_t brightness;} InterfaceSettingsScreen;
  struct {char passcode[5];}                   LockScreen;
  struct {bool isError;}                       AlertDialogBox;
  struct {uint8_t e_tag, t_tag, repeat_tag;}   ChangeFilamentScreen;
  struct {
    struct {
      uint8_t is_dir  : 1;
      uint8_t is_root : 1;
    } flags;
    uint8_t  selected_tag;
    uint8_t  num_page;
    uint8_t  cur_page;
  } FilesScreen;
  struct {
    struct base_numeric_adjustment_t placeholder;
    float e_rel[ExtUI::extruderCount];
  } MoveAxisScreen;
#if ENABLED(BABYSTEPPING)
  struct {
    struct base_numeric_adjustment_t placeholder;
    int16_t rel[XYZ];
    #if EXTRUDERS > 1
      bool  link_nozzles;
    #endif
    bool show_offsets;
  } NudgeNozzleScreen;
#endif
};

extern screen_data_t screen_data;