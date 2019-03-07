/*****************
 * logo_demo.ino *
 *****************/

/****************************************************************************
 *   Written By Marcio Teixeira 2019 - Aleph Objects, Inc.                  *
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

#include "src/ftdi_eve_lib/ftdi_eve_lib.h"
#include "src/bioprinter_ui.h"
#include "src/polygon.h"

using namespace FTDI;

/***************** SCREEN DEFINITIONS *****************/

class LogoScreen : public UIScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what) {
      constexpr uint32_t background_rgb = 0xFFFFFF;
      constexpr uint32_t foreground_rgb = 0xC1D82F;

      CommandProcessor cmd;
      cmd.cmd(CLEAR_COLOR_RGB(background_rgb));
      cmd.cmd(CLEAR(true,true,true));
      cmd.tag(0);

      #define POLY(A) A, sizeof(A)/sizeof(A[0])

      Polygon p(x_min, y_min, x_max, y_max); 

      // Paint the shadows
      cmd.cmd(COLOR_RGB(0xF3E0E0));
      p.shadow(POLY(usb_btn));
      p.shadow(POLY(menu_btn));
      p.shadow(POLY(syringe_outline), 3);

      cmd.cmd(COLOR_RGB(0xF4F4FF));
      p.fill(POLY(syringe_outline));
      cmd.cmd(COLOR_RGB(0x00AA00));
      p.fill(POLY(syringe_fluid));
      cmd.cmd(COLOR_RGB(0x000000));
      p.fill(POLY(syringe));
      
      // Stroke the polygons
      cmd.cmd(LINE_WIDTH(24));
      cmd.cmd(COLOR_RGB(0x00AA00));
      p.button(1, POLY(x_pos));
      p.button(2, POLY(y_pos));
      p.button(3, POLY(z_pos));
      p.button(4, POLY(e_pos));
      p.button(5, POLY(x_neg));
      p.button(6, POLY(y_neg));
      p.button(7, POLY(z_neg));
      p.button(8, POLY(e_neg));

      uint16_t x, y, h, v;

      cmd.cmd(COLOR_RGB(0x000000));
      cmd.fgcolor(0x00AA00);
      
      p.bounds(POLY(usb_btn), x, y, h, v);
      cmd.font(28).tag(9).button(x, y, h, v, F("USB Drive"));

      p.bounds(POLY(menu_btn), x, y, h, v);
      cmd.font(28).tag(10).button(x, y, h, v, F("Menu"));
    }

    static bool LogoScreen::onTouchEnd(uint8_t tag) {
      switch(tag) {
        case 1: {return true;
        }
        default: return false;
      }
    }
};

/***************** LIST OF SCREENS *****************/

SCREEN_TABLE {
  DECL_SCREEN(LogoScreen),
};
SCREEN_TABLE_POST

/***************** MAIN PROGRAM *****************/

void setup() {
  Serial.begin(9600);
  EventLoop::setup();
  CLCD::turn_on_backlight();
  SoundPlayer::set_volume(255);
}

void loop() {
  EventLoop::loop();
}
