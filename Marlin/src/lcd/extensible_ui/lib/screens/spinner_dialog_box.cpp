/**************************
 * spinner_dialog_box.cpp *
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

#include "../config.h"

#if ENABLED(EXTENSIBLE_UI)

#include "screens.h"

using namespace FTDI;


void SpinnerDialogBox::onRedraw(draw_mode_t what) {
}

void SpinnerDialogBox::show(const progmem_str line1, const progmem_str line2, const progmem_str line3) {
  drawMessage(line1, line2, line3);
  drawSpinner();
  storeBackground();
}

void SpinnerDialogBox::hide() {
  CommandProcessor cmd;
  cmd.stop().execute();
}

#endif // EXTENSIBLE_UI