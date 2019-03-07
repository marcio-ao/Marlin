/*******************************
 * filament_options_screen.cpp *
 *******************************/

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

#if ENABLED(EXTENSIBLE_UI) && (ENABLED(LIN_ADVANCE) || ENABLED(FILAMENT_RUNOUT_SENSOR))

#include "screens.h"

using namespace FTDI;
using namespace ExtUI;
using namespace Theme;

void FilamentOptionsScreen::onRedraw(draw_mode_t what) {

  widgets_t w(what);
  w.precision(1).color(e_axis);
  #if ENABLED(LIN_ADVANCE)
    w.heading(           PSTR("Linear Advance:"));
    #if EXTRUDERS == 1
      w.adjuster(     2, PSTR("K:"),    getLinearAdvance_mm_mm_s(E0) );
    #else
      w.adjuster(     2, PSTR("K E1:"), getLinearAdvance_mm_mm_s(E0) );
      w.adjuster(     4, PSTR("K E2:"), getLinearAdvance_mm_mm_s(E1) );
      #if EXTRUDERS > 2
        w.adjuster(   6, PSTR("K E3:"), getLinearAdvance_mm_mm_s(E2) );
        #if EXTRUDERS > 3
          w.adjuster( 8, PSTR("K E4:"), getLinearAdvance_mm_mm_s(E3) );
        #endif
      #endif
    #endif
  #endif

  #if ENABLED(FILAMENT_RUNOUT_SENSOR)
    w.heading( PSTR("Runout Detection:"));
    #if defined(FILAMENT_RUNOUT_DISTANCE_MM)
      w.units(PSTR("mm"));
      if(getFilamentRunoutEnabled()) {
        w.adjuster( 10, PSTR("Distance:"), getFilamentRunoutDistance_mm() );
      } else {
        w.adjuster( 10, PSTR("Distance:"), PSTR("disabled") );
      }
    #else
      w.adjuster(   10, PSTR("Status:"), getFilamentRunoutEnabled() ? PSTR("enabled") : PSTR("disabled") );
    #endif
  #endif
    w.heading(PSTR(""));
    w.increments();
}

bool FilamentOptionsScreen::onTouchHeld(uint8_t tag) {
  using namespace ExtUI;
  const float increment = getIncrement();
  switch(tag) {
  #if ENABLED(LIN_ADVANCE)
      case  2: UI_DECREMENT(LinearAdvance_mm_mm_s, E0); break;
      case  3: UI_INCREMENT(LinearAdvance_mm_mm_s, E0); break;
      #if EXTRUDERS > 1
        case  4: UI_DECREMENT(LinearAdvance_mm_mm_s, E1);  break;
        case  5: UI_INCREMENT(LinearAdvance_mm_mm_s, E1); break;
        #if EXTRUDERS > 2
          case  6: UI_DECREMENT(LinearAdvance_mm_mm_s, E2);  break;
          case  7: UI_INCREMENT(LinearAdvance_mm_mm_s, E2);  break;
          #if EXTRUDERS > 3
            case  8: UI_DECREMENT(LinearAdvance_mm_mm_s, E3);  break;
            case  9: UI_INCREMENT(LinearAdvance_mm_mm_s, E3);  break;
          #endif
        #endif
      #endif
    #endif
    #if ENABLED(FILAMENT_RUNOUT_SENSOR)
      #if defined(FILAMENT_RUNOUT_DISTANCE_MM)
        case  10: UI_DECREMENT(FilamentRunoutDistance_mm); break;
        case  11: UI_INCREMENT(FilamentRunoutDistance_mm); break;
      #else
        case  10: setFilamentRunoutEnabled(false); break;
        case  11: setFilamentRunoutEnabled(true);  break;
      #endif
    #endif
    default:
      return false;
  }

  #if ENABLED(FILAMENT_RUNOUT_SENSOR) && defined(FILAMENT_RUNOUT_DISTANCE_MM)
    setFilamentRunoutEnabled(getFilamentRunoutDistance_mm() >= FILAMENT_RUNOUT_DISTANCE_MM);
  #endif

  onRefresh();
  return true;
}

bool FilamentOptionsScreen::onTouchEnd(uint8_t tag) {
  if(tag == 1 && !IS_PARENT_SCREEN(AdvancedSettingsMenu)) {
    /* The AdvancedSettingsMenu will prompt the user to
       save settings; any other parent, do it here */
    SaveSettingsDialogBox::promptToSaveSettings();
    return true;
  } else {
    return BaseNumericAdjustmentScreen::onTouchEnd(tag);
  }
}

#endif // EXTENSIBLE_UI