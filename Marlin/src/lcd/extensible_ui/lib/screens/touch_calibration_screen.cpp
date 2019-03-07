/********************************
 * touch_calibration_screen.cpp *
 ********************************/

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
using namespace Theme;

void TouchCalibrationScreen::onEntry() {
  // Clear the display
  CommandProcessor cmd;
  cmd.cmd(CMD_DLSTART)
     .cmd(CLEAR_COLOR_RGB(background))
     .cmd(CLEAR(true,true,true))
     .cmd(DL::DL_DISPLAY)
     .cmd(CMD_SWAP)
     .execute();

  // Wait for the touch to release before starting,
  // as otherwise the first calibration point could
  // be misinterpreted.
  while(CLCD::is_touching()) {
    #if defined(UI_FRAMEWORK_DEBUG)
      SERIAL_ECHO_START();
      SERIAL_ECHOLNPGM("Waiting for touch release");
    #endif
  }
  BaseScreen::onEntry();
}

void TouchCalibrationScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(background))
     .cmd(CLEAR(true,true,true))
  #define GRID_COLS 4
  #define GRID_ROWS 16
  #if defined(USE_PORTRAIT_ORIENTATION)
    .font(font_large)
    .text  ( BTN_POS(1,8), BTN_SIZE(4,1), F("Touch the dots"))
    .text  ( BTN_POS(1,9), BTN_SIZE(4,1), F("to calibrate"))
  #else
    .font(
      #if defined(LCD_800x480)
        font_large
      #else
        font_medium
      #endif
    )
    .text  ( BTN_POS(1,1), BTN_SIZE(4,16), F("Touch the dots to calibrate"))
  #endif
  #undef GRID_COLS
  #undef GRID_ROWS
    .cmd(CMD_CALIBRATE);
}

void TouchCalibrationScreen::onIdle() {
  if(!CommandProcessor::is_processing()) {
    GOTO_SCREEN(StatusScreen);
  }
}

#endif // EXTENSIBLE_UI