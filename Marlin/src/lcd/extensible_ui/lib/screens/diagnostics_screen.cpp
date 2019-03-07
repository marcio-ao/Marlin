/**************************
 * diagnostics_screen.cpp *
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

#if ENABLED(EXTENSIBLE_UI) && ENABLED(DEVELOPER_SCREENS)

#include "screens.h"

using namespace FTDI;
using namespace Theme;

void DiagnosticsScreen::onEntry() {
  BaseScreen::onEntry();
  #if defined(LULZBOT_SET_PROBE_PINS_STATE)
    LULZBOT_SET_PROBE_PINS_STATE(true)
  #endif
}

void DiagnosticsScreen::onExit() {
  BaseScreen::onExit();
  #if defined(LULZBOT_SET_PROBE_PINS_STATE)
    LULZBOT_SET_PROBE_PINS_STATE(false)
  #endif
}

void DiagnosticsScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(background))
     .cmd(CLEAR(true,true,true))
     .tag(0);

  #define GRID_ROWS 5
  #define GRID_COLS 3

  #define PIN_BTN(X,Y,PIN)          button(BTN_POS(X,Y), BTN_SIZE(1,1), F(#PIN))
  #define PIN_ENABLED(PIN,INV,X,Y)  cmd.enabled(1).style(READ(PIN##_PIN) != INV ? RED_BTN : 0).PIN_BTN(X,Y,PIN);
  #define PIN_DISABLED(PIN,INV,X,Y) cmd.enabled(0).PIN_BTN(X,Y,PIN);

  cmd.font(font_large).text(BTN_POS(1,1), BTN_SIZE(3,1), F("Pin States")).font(26);
  #if PIN_EXISTS(X_MAX)
    PIN_ENABLED (X_MAX,X_MAX_ENDSTOP_INVERTING,1,2)
  #else
    PIN_DISABLED(X_MAX,X_MAX_ENDSTOP_INVERTING,1,2)
  #endif
  #if PIN_EXISTS(Y_MAX)
    PIN_ENABLED (Y_MAX,Y_MAX_ENDSTOP_INVERTING,2,2)
  #else
    PIN_DISABLED(Y_MAX,Y_MAX_ENDSTOP_INVERTING,2,2)
  #endif
  #if PIN_EXISTS(Z_MAX)
    PIN_ENABLED (Z_MAX,Z_MAX_ENDSTOP_INVERTING,3,2)
  #else
    PIN_DISABLED(Z_MAX,Z_MAX_ENDSTOP_INVERTING,3,2)
  #endif
  #if PIN_EXISTS(X_MIN)
    PIN_ENABLED (X_MIN,X_MIN_ENDSTOP_INVERTING,1,3)
  #else
    PIN_DISABLED(X_MIN,X_MIN_ENDSTOP_INVERTING,1,3)
  #endif
  #if PIN_EXISTS(Y_MIN)
    PIN_ENABLED (Y_MIN,Y_MIN_ENDSTOP_INVERTING,2,3)
  #else
    PIN_DISABLED(Y_MIN,Y_MIN_ENDSTOP_INVERTING,2,3)
  #endif
  #if PIN_EXISTS(Z_MIN)
    PIN_ENABLED (Z_MIN,Z_MIN_ENDSTOP_INVERTING,3,3)
  #else
    PIN_DISABLED(Z_MIN,Z_MIN_ENDSTOP_INVERTING,3,3)
  #endif
  #if ENABLED(FILAMENT_RUNOUT_SENSOR) && PIN_EXISTS(FIL_RUNOUT)
    PIN_ENABLED (FIL_RUNOUT, FIL_RUNOUT_INVERTING,1,4)
  #else
    PIN_DISABLED(FIL_RUNOUT, FIL_RUNOUT_INVERTING,1,4)
  #endif
  #if ENABLED(FILAMENT_RUNOUT_SENSOR) && PIN_EXISTS(FIL_RUNOUT2)
    PIN_ENABLED (FIL_RUNOUT2,FIL_RUNOUT_INVERTING,2,4)
  #else
    PIN_DISABLED(FIL_RUNOUT2,FIL_RUNOUT_INVERTING,2,4)
  #endif
  #if PIN_EXISTS(Z_MIN_PROBE)
    PIN_ENABLED (Z_MIN_PROBE,Z_MIN_PROBE_ENDSTOP_INVERTING,3,4)
  #else
    PIN_DISABLED(Z_MIN_PROBE,Z_MIN_PROBE_ENDSTOP_INVERTING,3,4)
  #endif

  cmd.style(LIGHT_BTN).tag(1).font(font_medium)
     .button( BTN_POS(1,5), BTN_SIZE(3,1), F("Back"));
  #undef GRID_COLS
  #undef GRID_ROWS
}

bool DiagnosticsScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:        GOTO_PREVIOUS();                 break;
    default:
      return false;
  }
  return true;
}

void DiagnosticsScreen::onIdle() {
  constexpr uint32_t DIAGNOSTICS_UPDATE_INTERVAL = 100;

  if(refresh_timer.elapsed(DIAGNOSTICS_UPDATE_INTERVAL)) {
    onRefresh();
    refresh_timer.start();
    reset_menu_timeout();
  }
  BaseScreen::onIdle();
}

#endif // EXTENSIBLE_UI