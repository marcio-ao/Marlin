/**************************************
 * confirm_erase_flash_dialog_box.cpp *
 **************************************/

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

#include "../config.h"

#if ENABLED(EXTENSIBLE_UI) && ENABLED(DEVELOPER_SCREENS)

#include "screens.h"

#include "../io/flash_storage.h"

using namespace FTDI;

void ConfirmEraseFlashDialogBox::onRedraw(draw_mode_t what) {
  drawMessage(
    F("Are you sure?"),
    F("SPI flash will be erased.")
  );
  drawYesNoButtons();
}

bool ConfirmEraseFlashDialogBox::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:
      SpinnerDialogBox::show(F("Erasing..."));
      UIFlashStorage::erase_chip();
      SpinnerDialogBox::hide();
      AlertDialogBox::show(F("SPI flash erased"));
      // Remove ConfirmEraseFlashDialogBox from the stack
      // so the alert box doesn't return to me.
      current_screen.forget();
      return true;
    default:
      return DialogBoxBaseClass::onTouchEnd(tag);
  }
}

#endif // EXTENSIBLE_UI