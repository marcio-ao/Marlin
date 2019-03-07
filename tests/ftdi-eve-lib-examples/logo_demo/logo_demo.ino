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
#include "src/LulzBot_Logo.h"

using namespace FTDI;

/***************** SCREEN DEFINITIONS *****************/

class LogoScreen : public UIScreen, public UncachedScreen {
  private:
    static uint32_t color;

  public:
    static void polygon_vertices(const uint16_t data[], size_t n, uint16_t origin_x, uint16_t origin_y, uint16_t scale, begin_t primitive) {
      uint16_t poly_start_x = 0xFFFF, poly_start_y;
      uint32_t last_x = -1, last_y = -1;
      CommandProcessor cmd;
      cmd.cmd(BEGIN(primitive));
      for(size_t i = 0; i < n; i += 2) {
        const uint16_t x = pgm_read_word_far(&data[i+0]);
        if(x == 0xFFFF) {
          if(poly_start_x != 0xFFFF) {
            cmd.cmd(VERTEX2F(poly_start_x, poly_start_y));
            poly_start_x = 0xFFFF;
          }
          cmd.cmd(BEGIN(primitive));
          i += 1;
          continue;
          poly_start_x = 0xFFFF;
        }
        const uint16_t y = pgm_read_word_far(&data[i+1]);
        const uint32_t scaled_x = uint32_t(x) * scale / 0xFFFE + origin_x;
        const uint32_t scaled_y = uint32_t(y) * scale / 0xFFFE + origin_y;
        if(last_x != scaled_x || last_y != scaled_y)
          cmd.cmd(VERTEX2F(scaled_x, scaled_y));
        last_x = scaled_x;
        last_y = scaled_y;
        if(poly_start_x == 0xFFFF) {
          poly_start_x = scaled_x;
          poly_start_y = scaled_y;
        }
      }
      cmd.cmd(VERTEX2F(poly_start_x, poly_start_y));
    }

    static void polygon_fill(const uint16_t data[], size_t n, uint16_t origin_x, uint16_t origin_y, uint16_t scale) {
      CommandProcessor cmd;
      cmd.cmd(SAVE_CONTEXT());
      cmd.cmd(COLOR_MASK(0,0,0,0));
      cmd.cmd(STENCIL_OP(STENCIL_OP_KEEP, STENCIL_OP_INVERT));
      cmd.cmd(STENCIL_FUNC(STENCIL_FUNC_ALWAYS, 255, 255));
      polygon_vertices(data, n, origin_x, origin_y, scale, EDGE_STRIP_B);
      cmd.cmd(RESTORE_CONTEXT());

      cmd.cmd(SAVE_CONTEXT());
      cmd.cmd(STENCIL_FUNC(STENCIL_FUNC_NOTEQUAL, 0, 255));
      cmd.cmd(BEGIN(RECTS));
      cmd.cmd(VERTEX2F( 0  * 16,  0  * 16));
      cmd.cmd(VERTEX2F(display_width * 16, display_height * 16));
      cmd.cmd(RESTORE_CONTEXT());
    }

    static void polygon_outline(const uint16_t data[], size_t n, uint16_t origin_x, uint16_t origin_y, uint16_t scale) {
      polygon_vertices(data, n, origin_x, origin_y, scale, LINE_STRIP);
    }

    static void onRedraw(draw_mode_t what) {
      constexpr uint32_t background_rgb = 0xDEEA5C;
      constexpr uint32_t foreground_rgb = 0xC1D82F;

      CommandProcessor cmd;
      cmd.cmd(CLEAR_COLOR_RGB(background_rgb));
      cmd.cmd(CLEAR(true,true,true));

      cmd.cmd(COLOR_RGB(color));
      polygon_fill(logo, sizeof(logo)/sizeof(logo[0]), 15 * 16, 30 * 16, 250 * 16);

      #define GRID_COLS 4
      #define GRID_ROWS 7
      cmd.fgcolor(foreground_rgb);
      cmd.font(30).tag(1).button(BTN_POS(2,6), BTN_SIZE(2,1), F("Okay"));
    }

    static bool onTouchEnd(uint8_t tag) {
      switch(tag) {
        case 1: {
          color = random(0xFFFFFF);
          return true;
        }
        default: return false;
      }
    }
};

uint32_t LogoScreen::color = 0xFFFFFF;

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
