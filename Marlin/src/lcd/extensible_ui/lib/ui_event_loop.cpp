/*********************
 * ui_event_loop.cpp *
 *********************/

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

#if ENABLED(EXTENSIBLE_UI)

#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"
#include "ftdi_eve_panels.h"

#include "ui_framework.h"
#include "ui_theme.h"
#include "ui_builder.h"
#include "ui_dl_cache.h"
#include "ui_event_loop.h"
#include "ui_sounds.h"

using namespace FTDI;

enum {
  UNPRESSED       = 0xFF, //255
  IGNORE_UNPRESS  = 0xFE, //254
  DEBOUNCING      = 0xFD  //253
};

tiny_timer_t            touch_timer;
bool is_tracking       = false;
bool touch_sound       = true;
uint8_t pressed_state  = UNPRESSED;

void enable_touch_sound(bool enabled) {
    touch_sound = enabled;
}

void start_tracking(int16_t x, int16_t y, int16_t w, int16_t h, int16_t tag, bool rotary) {
  CommandProcessor cmd;
  cmd.track(x, y, w, h, tag, rotary);
  cmd.execute();
  is_tracking = true;
}

void stop_tracking() {
  CommandProcessor cmd;
  is_tracking = false;
  cmd.track(0, 0, 0, 0, 0, false);
  cmd.execute();
}

uint8_t get_pressed_tag() {
  if(pressed_state < DEBOUNCING) {
    return pressed_state;
  } else {
    return 0;
  }
}

namespace Extensible_UI_API {
  void onStartup() {
    using namespace Extensible_UI_API;

    CLCD::init();
    DLCache::init();

    current_screen.start();
  }

  void onUpdate() {
    sound.onIdle();
    current_screen.onIdle();

    // If the LCD is processing commands, don't check
    // for tags since they may be changing and could
    // cause spurious events.
    if(!touch_timer.elapsed(TOUCH_UPDATE_INTERVAL) || CLCD::CommandFifo::is_processing()) {
      return;
    }

    if(is_tracking && !CLCD::is_touching()) {
      stop_tracking();
    }

    const uint8_t tag = CLCD::get_tag();

    switch(pressed_state) {
      case UNPRESSED:
        if(tag != 0) {
          #if defined(UI_FRAMEWORK_DEBUG)
            #if defined (SERIAL_PROTOCOLLNPAIR)
              SERIAL_PROTOCOLLNPAIR("Touch start: ", tag);
            #else
              Serial.print(F("Touch start: "));
              Serial.println(tag);
            #endif
          #endif

          pressed_state = tag;

          // When the user taps on a button, activate the onTouchStart handler
          const uint8_t lastScreen = current_screen.getScreen();

          current_screen.onRefresh();
          if(current_screen.onTouchStart(tag)) {
            touch_timer.start();
            if(touch_sound) sound.play(Theme::press_sound);
          }

          if(lastScreen != current_screen.getScreen()) {
            // In the case in which a touch event triggered a new screen to be
            // drawn, we don't issue a touchEnd since it would be sent to the
            // wrong screen.
            pressed_state = IGNORE_UNPRESS;
            #if defined(UI_FRAMEWORK_DEBUG)
              #if defined (SERIAL_PROTOCOLLNPAIR)
                SERIAL_PROTOCOLLNPAIR("Ignoring press", tag);
              #else
                Serial.print(F("Ignoring press"));
                Serial.println(tag);
              #endif
            #endif
          }
        } else {
          touch_timer.start();
        }
        break;
      case DEBOUNCING:
        if(tag == 0) {
          if(touch_timer.elapsed(DEBOUNCE_PERIOD)) {
            pressed_state = UNPRESSED;
            if(touch_sound) sound.play(Theme::unpress_sound);
          }
        } else {
          pressed_state = IGNORE_UNPRESS;
        }
        break;
      case IGNORE_UNPRESS:
        if(tag == 0) {
          // Ignore subsequent presses for a while to avoid bouncing
          touch_timer.start();
          pressed_state = DEBOUNCING;
        }
        break;
      default: // PRESSED
        if(tag == pressed_state) {
          // The user is holding down a button.
          if(touch_timer.elapsed(1000 / TOUCH_REPEATS_PER_SECOND) && current_screen.onTouchHeld(tag)) {
            if(touch_sound) sound.play(Theme::repeat_sound);
            touch_timer.start();
          }
        }
        else if(tag == 0) {
          #if defined(UI_FRAMEWORK_DEBUG)
            #if defined (SERIAL_PROTOCOLLNPAIR)
              SERIAL_PROTOCOLLNPAIR("Touch end: ", pressed_state);
            #else
              Serial.print(F("Touch end: "));
              Serial.println(pressed_state);
            #endif
          #endif

          const uint8_t saved_pressed_state = pressed_state;
          pressed_state = 0;
          current_screen.onTouchEnd(saved_pressed_state);
          current_screen.onRefresh();
          // Ignore subsequent presses for a while to avoid bouncing
          touch_timer.start();
          pressed_state = DEBOUNCING;
        }
        break;
    }
  }
}

#endif // EXTENSIBLE_UI