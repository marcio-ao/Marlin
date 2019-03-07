/******************************
 * change_filament_screen.cpp *
 ******************************/

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
#include "screen_data.h"

using namespace FTDI;
using namespace Theme;

#define COOL_TEMP  40
#define LOW_TEMP  180
#define MED_TEMP  200
#define HIGH_TEMP 220

/****************** COLOR SCALE ***********************/

uint32_t getWarmColor(uint16_t temp, uint16_t cool, uint16_t low, uint16_t med, uint16_t high) {
  FTDI::rgb_t R0, R1, mix;

  float t;
  if(temp < cool) {
    R0 = cool_rgb;
    R1 = low_rgb;
    t  = 0;
  }
  else if(temp < low) {
    R0 = cool_rgb;
    R1 = low_rgb;
    t = (float(temp)-cool)/(low-cool);
  }
  else if(temp < med) {
    R0 = low_rgb;
    R1 = med_rgb;
    t = (float(temp)-low)/(med-low);
  }
  else if(temp < high) {
    R0 = med_rgb;
    R1 = high_rgb;
    t = (float(temp)-med)/(high-med);
  }
  else if(temp >= high) {
    R0 = med_rgb;
    R1 = high_rgb;
    t = 1;
  }
  FTDI::rgb_t::lerp(t, R0, R1, mix);
  return mix;
}

void ChangeFilamentScreen::drawTempGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  CommandProcessor cmd;
  cmd.cmd(SCISSOR_XY   (x, y))
     .cmd(SCISSOR_SIZE (w, h/2))
     .gradient         (x, y,     high_rgb, x, y+h/2, med_rgb)
     .cmd(SCISSOR_XY   (x, y+h/2))
     .cmd(SCISSOR_SIZE (w, h/2))
     .gradient         (x, y+h/2, med_rgb,  x, y+h, low_rgb)
     .cmd(SCISSOR_XY   ())
     .cmd(SCISSOR_SIZE ());
}

void ChangeFilamentScreen::onEntry() {
  screen_data.ChangeFilamentScreen.e_tag = ExtUI::getActiveTool() + 10;
  screen_data.ChangeFilamentScreen.t_tag = 0;
  screen_data.ChangeFilamentScreen.repeat_tag = 0;
}

void ChangeFilamentScreen::onRedraw(draw_mode_t what) {
  using namespace ExtUI;
  CommandProcessor cmd;

  #if defined(USE_PORTRAIT_ORIENTATION)
    #define GRID_COLS 2
    #define GRID_ROWS 11
  #else
    #define GRID_COLS 4
    #define GRID_ROWS 6
  #endif

  if(what & BACKGROUND) {
    cmd.cmd(CLEAR_COLOR_RGB(background))
       .cmd(CLEAR(true,true,true))
       .tag(0)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .font(font_large)
    #else
       .font(font_medium)
    #endif
       .text(BTN_POS(1,1), BTN_SIZE(2,1), F("Extruder Selection:"))
    #if defined(USE_PORTRAIT_ORIENTATION)
       .text(BTN_POS(1,7), BTN_SIZE(1,1), F("Current Temp:"))
    #else
       .text(BTN_POS(3,1), BTN_SIZE(2,1), F("Current Temp:"))
       .font(font_small)
    #endif
       .text(BTN_POS(1,3), BTN_SIZE(2,1), F("Removal Temp:"));
    drawTempGradient(BTN_POS(1,4), BTN_SIZE(1,3));
  }

  if(what & FOREGROUND) {
    char e_str[15];

    sprintf_P(
      e_str,
      PSTR("%-3d / %-3d C"),
      ROUND(getActualTemp_celsius(getExtruder())),
      ROUND(getTargetTemp_celsius(getExtruder()))
    );

    const rgb_t tcol = getWarmColor(getActualTemp_celsius(getExtruder()), COOL_TEMP, LOW_TEMP, MED_TEMP, HIGH_TEMP);
    cmd.cmd(COLOR_RGB(tcol))
       .tag(15)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .rectangle(BTN_POS(2,7), BTN_SIZE(1,1))
    #else
       .rectangle(BTN_POS(3,2), BTN_SIZE(2,1))
    #endif
       .cmd(COLOR_RGB(tcol.luminance() > 128 ? 0x000000 : 0xFFFFFF))
       .font(font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .text(BTN_POS(2,7), BTN_SIZE(1,1), e_str);
    #else
       .text(BTN_POS(3,2), BTN_SIZE(2,1), e_str);
    #endif

    default_button_colors();

    const bool t_ok = getActualTemp_celsius(getExtruder()) > getSoftenTemp() - 10;

    if(screen_data.ChangeFilamentScreen.t_tag && !t_ok) {
      cmd.text(BTN_POS(1,6), BTN_SIZE(1,1), F("Heating..."));
    } else if(getActualTemp_celsius(getExtruder()) > 100) {
      cmd.cmd(COLOR_RGB(0xFF0000))
         .text(BTN_POS(1,4), BTN_SIZE(1,1), F("Caution:"));
      default_button_colors();
      cmd.text(BTN_POS(1,6), BTN_SIZE(1,1), F("Hot!"));
    }

    const uint32_t tog2  = screen_data.ChangeFilamentScreen.t_tag == 2  ? LIGHT_BTN : 0;
    const uint32_t tog3  = screen_data.ChangeFilamentScreen.t_tag == 3  ? LIGHT_BTN : 0;
    const uint32_t tog4  = screen_data.ChangeFilamentScreen.t_tag == 4  ? LIGHT_BTN : 0;
    const uint32_t tog10 = screen_data.ChangeFilamentScreen.e_tag == 10 ? LIGHT_BTN : 0;
    #if HOTENDS > 1
    const uint32_t tog11 = screen_data.ChangeFilamentScreen.e_tag == 11 ? LIGHT_BTN : 0;
    #endif

    #if defined(USE_PORTRAIT_ORIENTATION)
      cmd.font(font_large)
    #else
      cmd.font(font_medium)
    #endif
       .tag(10).style(tog10).button (BTN_POS(1,2), BTN_SIZE(1,1), F("1"))
    #if HOTENDS < 2
       .enabled(false)
    #else
       .style(tog11)
    #endif
       .tag(11)             .button (BTN_POS(2,2), BTN_SIZE(1,1), F("2"))
       .style(0);

    if(!t_ok) reset_menu_timeout();

    const uint16_t tag7_style = screen_data.ChangeFilamentScreen.repeat_tag == 7 ? LIGHT_BTN : 0;
    const uint16_t tag8_style = screen_data.ChangeFilamentScreen.repeat_tag == 8 ? LIGHT_BTN : 0;

    #if defined(USE_PORTRAIT_ORIENTATION)
      cmd.font(font_large)
    #else
      cmd.font(font_small)
    #endif
       .tag(2) .style(tog2) .button (BTN_POS(2,6), BTN_SIZE(1,1), F( STRINGIFY(LOW_TEMP)  "C (PLA)"))
       .tag(3) .style(tog3) .button (BTN_POS(2,5), BTN_SIZE(1,1), F( STRINGIFY(MED_TEMP)  "C (ABS)"))
       .tag(4) .style(tog4) .button (BTN_POS(2,4), BTN_SIZE(1,1), F( STRINGIFY(HIGH_TEMP) "C (High)"))
       .style(0)

    // Add tags to color gradient
    .cmd(COLOR_MASK(0,0,0,0))
    .tag(2) .rectangle(BTN_POS(1,6), BTN_SIZE(1,1))
    .tag(3) .rectangle(BTN_POS(1,5), BTN_SIZE(1,1))
    .tag(4) .rectangle(BTN_POS(1,4), BTN_SIZE(1,1))
    .cmd(COLOR_MASK(1,1,1,1))

    .cmd(COLOR_RGB(t_ok ? text_enabled : text_disabled))
    #if defined(USE_PORTRAIT_ORIENTATION)
       .font(font_large)
       .tag(0)                                .text   (BTN_POS(1,8),  BTN_SIZE(1,1), F("Unload"))
                                              .text   (BTN_POS(2,8),  BTN_SIZE(1,1), F("Load/Extrude"))
       .tag(5)                  .enabled(t_ok).button (BTN_POS(1,9),  BTN_SIZE(1,1), F("Momentary"))
       .tag(6)                  .enabled(t_ok).button (BTN_POS(2,9),  BTN_SIZE(1,1), F("Momentary"))
       .tag(7).style(tag7_style).enabled(t_ok).button (BTN_POS(1,10), BTN_SIZE(1,1), F("Continuous"))
       .tag(8).style(tag8_style).enabled(t_ok).button (BTN_POS(2,10), BTN_SIZE(1,1), F("Continuous"))
       .tag(1).style(LIGHT_BTN)         .button (BTN_POS(1,11), BTN_SIZE(2,1), F("Back"));
    #else
       .font(font_small)
       .tag(0)                                .text   (BTN_POS(3,3), BTN_SIZE(1,1), F("Unload"))
                                              .text   (BTN_POS(4,3), BTN_SIZE(1,1), F("Load/Extrude"))
       .tag(5)                  .enabled(t_ok).button (BTN_POS(3,4), BTN_SIZE(1,1), F("Momentary"))
       .tag(6)                  .enabled(t_ok).button (BTN_POS(4,4), BTN_SIZE(1,1), F("Momentary"))
       .tag(7).style(tag7_style).enabled(t_ok).button (BTN_POS(3,5), BTN_SIZE(1,1), F("Continuous"))
       .tag(8).style(tag8_style).enabled(t_ok).button (BTN_POS(4,5), BTN_SIZE(1,1), F("Continuous"))
       .font(font_medium)
       .tag(1).style(LIGHT_BTN)         .button (BTN_POS(3,6), BTN_SIZE(2,1), F("Back"));
    #endif
  }
  #undef GRID_COLS
  #undef GRID_ROWS
}

uint8_t ChangeFilamentScreen::getSoftenTemp() {
  switch(screen_data.ChangeFilamentScreen.t_tag) {
    case 2:  return LOW_TEMP;
    case 3:  return MED_TEMP;
    case 4:  return HIGH_TEMP;
    default: return EXTRUDE_MINTEMP;
  }
}

ExtUI::extruder_t ChangeFilamentScreen::getExtruder() {
  switch(screen_data.ChangeFilamentScreen.e_tag) {
    case 13: return ExtUI::E3;
    case 12: return ExtUI::E2;
    case 11: return ExtUI::E1;
    default: return ExtUI::E0;
  }
}

bool ChangeFilamentScreen::onTouchEnd(uint8_t tag) {
  using namespace ExtUI;
  switch(tag) {
    case 1: GOTO_PREVIOUS(); break;
    case 2:
    case 3:
    case 4:
      // Change temperature
      screen_data.ChangeFilamentScreen.t_tag = tag;
      setTargetTemp_celsius(getSoftenTemp(), getExtruder());
      break;
    case 7:
      screen_data.ChangeFilamentScreen.repeat_tag = (screen_data.ChangeFilamentScreen.repeat_tag == 7) ? 0 : 7;
      break;
    case 8:
      screen_data.ChangeFilamentScreen.repeat_tag = (screen_data.ChangeFilamentScreen.repeat_tag == 8) ? 0 : 8;
      break;
    case 10:
    case 11:
      // Change extruder
      screen_data.ChangeFilamentScreen.e_tag      = tag;
      screen_data.ChangeFilamentScreen.t_tag      = 0;
      screen_data.ChangeFilamentScreen.repeat_tag = 0;
      setActiveTool(getExtruder(), true);
      break;
    case 15: GOTO_SCREEN(TemperatureScreen); break;
  }
  return true;
}

bool ChangeFilamentScreen::onTouchHeld(uint8_t tag) {
  if(ExtUI::isMoving()) return false; // Don't allow moves to accumulate
  constexpr float increment = 1;
  #define UI_INCREMENT_AXIS(axis) MoveAxisScreen::setManualFeedrate(axis, increment); UI_INCREMENT(AxisPosition_mm, axis);
  #define UI_DECREMENT_AXIS(axis) MoveAxisScreen::setManualFeedrate(axis, increment); UI_DECREMENT(AxisPosition_mm, axis);
  switch(tag) {
    case 5: case 7: UI_DECREMENT_AXIS(getExtruder()); break;
    case 6: case 8: UI_INCREMENT_AXIS(getExtruder()); break;
    default: return false;
  }
  #undef UI_DECREMENT_AXIS
  #undef UI_INCREMENT_AXIS
  return true;
}

void ChangeFilamentScreen::onIdle() {
  if(screen_data.ChangeFilamentScreen.repeat_tag) onTouchHeld(screen_data.ChangeFilamentScreen.repeat_tag);
  if(refresh_timer.elapsed(STATUS_UPDATE_INTERVAL)) {
    onRefresh();
    refresh_timer.start();
  }
  BaseScreen::onIdle();
}

#endif // EXTENSIBLE_UI