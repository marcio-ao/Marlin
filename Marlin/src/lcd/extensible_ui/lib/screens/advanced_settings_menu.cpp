/*****************************
 * advance_settings_menu.cpp *
 *****************************/

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
using namespace ExtUI;

void AdvancedSettingsMenu::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true));
  }

  if(what & FOREGROUND) {
    CommandProcessor cmd;
    default_button_colors();
    cmd.font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_ROWS 7
      #define GRID_COLS 2
      #if HAS_BED_PROBE
        .enabled(1)
      #else
        .enabled(0)
      #endif
      .tag(2) .button( BTN_POS(1,1), BTN_SIZE(1,1), F("Z Offset "))
      .enabled(1)
      .tag(3) .button( BTN_POS(1,2), BTN_SIZE(1,1), F("Steps/mm"))
      #if HOTENDS > 1
      .enabled(1)
      #else
      .enabled(0)
      #endif
      .tag(4) .button( BTN_POS(1,4), BTN_SIZE(1,1), F("Nozzle Offsets"))
      #if ENABLED(LIN_ADVANCE) || ENABLED(FILAMENT_RUNOUT_SENSOR)
      .enabled(1)
      #else
      .enabled(0)
      #endif
      .tag(11).button( BTN_POS(1,3), BTN_SIZE(1,1), F("Filament"))
      .tag(9) .button( BTN_POS(1,5), BTN_SIZE(2,1), F("Interface Settings"))
      .tag(10).button( BTN_POS(1,6), BTN_SIZE(2,1), F("Restore Factory Defaults"))
      .tag(5) .button( BTN_POS(2,1), BTN_SIZE(1,1), F("Velocity "))
      .tag(6) .button( BTN_POS(2,2), BTN_SIZE(1,1), F("Acceleration"))
      #if ENABLED(JUNCTION_DEVIATION)
      .tag(7) .button( BTN_POS(2,3), BTN_SIZE(1,1), F("Junc Dev"))
      #else
      .tag(7) .button( BTN_POS(2,3), BTN_SIZE(1,1), F("Jerk"))
      #endif
      #if ENABLED(BACKLASH_GCODE)
      .enabled(1)
      #else
      .enabled(0)
      #endif
      .tag(8).button( BTN_POS(2,4), BTN_SIZE(1,1), F("Axis Backlash"))
      .style(LIGHT_BTN)
      .tag(1) .button( BTN_POS(1,7), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #else
      #define GRID_ROWS 6
      #define GRID_COLS 2
      #if HAS_BED_PROBE
        .enabled(1)
      #else
        .enabled(0)
      #endif
      .tag(2) .button( BTN_POS(1,1),  BTN_SIZE(1,1), F("Z Offset "))
      .enabled(1)
      .tag(3) .button( BTN_POS(1,2),  BTN_SIZE(1,1), F("Steps/mm"))
      #if ENABLED(BACKLASH_GCODE)
      .enabled(1)
      #else
      .enabled(0)
      #endif
      .tag(8).button( BTN_POS(1,3),  BTN_SIZE(1,1), F("Axis Backlash"))
      #if HOTENDS > 1
      .enabled(1)
      #else
      .enabled(0)
      #endif
      .tag(4) .button( BTN_POS(1,4),  BTN_SIZE(1,1), F("Nozzle Offsets"))
      .tag(5) .button( BTN_POS(2,1),  BTN_SIZE(1,1), F("Velocity "))
      .tag(6) .button( BTN_POS(2,2),  BTN_SIZE(1,1), F("Acceleration"))
      #if ENABLED(JUNCTION_DEVIATION)
      .tag(7) .button( BTN_POS(2,3),  BTN_SIZE(1,1), F("Junc Dev"))
      #else
      .tag(7) .button( BTN_POS(2,3),  BTN_SIZE(1,1), F("Jerk"))
      #endif
      .tag(11).button( BTN_POS(2,4), BTN_SIZE(1,1),  F("Filament"))
      .tag(9) .button( BTN_POS(2,5),  BTN_SIZE(1,1), F("Interface Settings"))
      .tag(10).button( BTN_POS(1,5),  BTN_SIZE(1,1), F("Restore Defaults"))
      .style(LIGHT_BTN)
      .tag(1) .button( BTN_POS(1,6),  BTN_SIZE(2,1), F("Back"));
    #endif
  }
}

bool AdvancedSettingsMenu::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1: SaveSettingsDialogBox::promptToSaveSettings(); break;
    #if HAS_BED_PROBE
    case 2:  GOTO_SCREEN(ZOffsetScreen);              break;
    #endif
    case 3:  GOTO_SCREEN(StepsScreen);                break;
    #if HOTENDS > 1
    case 4:  GOTO_SCREEN(NozzleOffsetScreen);         break;
    #endif
    case 5:  GOTO_SCREEN(MaxVelocityScreen);          break;
    case 6:  GOTO_SCREEN(DefaultAccelerationScreen);  break;
    case 7:
      #if ENABLED(JUNCTION_DEVIATION)
        GOTO_SCREEN(JunctionDeviationScreen);
      #else
        GOTO_SCREEN(JerkScreen);
      #endif
      break;
    #if ENABLED(BACKLASH_GCODE)
    case 8:  GOTO_SCREEN(BacklashCompensationScreen); break;
    #endif
    #if ENABLED(LIN_ADVANCE) || ENABLED(FILAMENT_RUNOUT_SENSOR)
    case 11: GOTO_SCREEN(FilamentOptionsScreen);      break;
    #endif
    case 9:  GOTO_SCREEN(InterfaceSettingsScreen);  LockScreen::check_passcode(); break;
    case 10: GOTO_SCREEN(RestoreFailsafeDialogBox); LockScreen::check_passcode(); break;
    default: return false;
  }
  return true;
}

#endif // EXTENSIBLE_UI