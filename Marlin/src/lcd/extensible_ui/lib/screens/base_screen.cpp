/*******************
 * base_screen.cpp *
 *******************/

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

void BaseScreen::onEntry() {
  CommandProcessor cmd;
  cmd.set_button_style_callback(buttonStyleCallback);
  UIScreen::onEntry();
}

bool BaseScreen::buttonStyleCallback(uint8_t tag, uint8_t &style, uint16_t &options, bool post) {
  CommandProcessor cmd;

  if(post) {
    default_button_colors();
    return false;
  }

  #if defined(MENU_TIMEOUT)
  if(EventLoop::get_pressed_tag() != 0) {
    reset_menu_timeout();
  }
  #endif

  if(tag != 0 && EventLoop::get_pressed_tag() == tag) {
    options = OPT_FLAT;
  }

  if(style & RED_BTN) {
    if(style & DISABLED) {
      cmd.cmd(COLOR_RGB(red_btn::rgb_disabled))
         .fgcolor(red_btn::fg_disabled)
         .tag(0);
      style &= ~DISABLED; // Clear the disabled flag
    } else {
      cmd.cmd(COLOR_RGB(red_btn::rgb_enabled))
         .fgcolor(red_btn::fg_enabled);
    }
    return true;              // Call me again to reset the colors
  }

  if(style & LIGHT_BTN) {
    if(style & DISABLED) {
      cmd.cmd(COLOR_RGB(light_btn::rgb_disabled))
         .fgcolor(light_btn::fg_disabled)
         .gradcolor(0xFFFFFF)
         .tag(0);
      style &= ~DISABLED; // Clear the disabled flag
    } else {
      cmd.cmd(COLOR_RGB(light_btn::rgb_enabled))
         .gradcolor(light_btn::grad_enabled)
         .fgcolor(light_btn::fg_enabled);
    }
    return true;              // Call me again to reset the colors
  }

  if(style & DISABLED) {
    cmd.cmd(COLOR_RGB(default_btn::rgb_disabled))
       .fgcolor(default_btn::fg_disabled)
       .tag(0);
    style &= ~DISABLED; // Clear the disabled flag
    return true;              // Call me again to reset the colors
  }
  return false;
}

void BaseScreen::default_button_colors() {
  CommandProcessor cmd;
  cmd.cmd(COLOR_RGB(default_btn::rgb_enabled))
     .gradcolor(default_btn::grad_enabled)
     .fgcolor(default_btn::fg_enabled);
}

void BaseScreen::onIdle() {
  #if defined(MENU_TIMEOUT)
    const uint32_t elapsed = millis() - last_interaction;
    if(elapsed > uint32_t(MENU_TIMEOUT) * 1000) {
      GOTO_SCREEN(StatusScreen);
      reset_menu_timeout();
    }
  #endif
}

void BaseScreen::reset_menu_timeout() {
  #if defined(MENU_TIMEOUT)
    last_interaction = millis();
  #endif
}

#if defined(MENU_TIMEOUT)
  uint32_t BaseScreen::last_interaction;
#endif

#endif // EXTENSIBLE_UI