/****************
 * ui_builder.h *
 ****************/

/****************************************************************************
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

#ifndef _UI_BUILDER_H_
#define _UI_BUILDER_H_

/**************************** GRID LAYOUT MACROS **************************/

/* The grid layout macros allow buttons to be arranged on a grid so
 * that their locations become independent of the display size. The
 * layout model is similar to that of HTML TABLEs.
 *
 * These macros are meant to be evaluated into constants at compile
 * time, so resolution independence can be as efficient as using
 * hard-coded coordinates.
 */

// Margin defines the margin (in pixels) on each side of a button in
// the layout

#if defined(LCD_800x480)
  #define MARGIN_L         5
  #define MARGIN_R         5
  #define MARGIN_T         5
  #define MARGIN_B         5
#else
  #define MARGIN_L         3
  #define MARGIN_R         3
  #define MARGIN_T         3
  #define MARGIN_B         3
#endif

// EDGE_R adds some black space on the right edge of the display
// This shifts some of the screens left to visually center them.

#define EDGE_R           0

// GRID_X and GRID_Y computes the positions of the divisions on
// the layout grid.
#if defined(USE_PORTRAIT_ORIENTATION)
  #define GRID_X(x)        ((x)*(Vsize-EDGE_R)/GRID_COLS)
  #define GRID_Y(y)        ((y)*Hsize/GRID_ROWS)
#else
  #define GRID_X(x)        ((x)*(Hsize-EDGE_R)/GRID_COLS)
  #define GRID_Y(y)        ((y)*Vsize/GRID_ROWS)
#endif

// BTN_X, BTN_Y, BTN_W and BTN_X returns the top-left and width
// and height of a button, taking into account the button margins.

#define BTN_X(x)         (GRID_X((x)-1) + MARGIN_L)
#define BTN_Y(y)         (GRID_Y((y)-1) + MARGIN_T)
#define BTN_W(w)         (GRID_X(w)   - MARGIN_L - MARGIN_R)
#define BTN_H(h)         (GRID_Y(h)   - MARGIN_T - MARGIN_B)

// Abbreviations for common phrases, to allow a button to be
// defined in one line of source.
#define BTN_POS(x,y)     BTN_X(x), BTN_Y(y)
#define BTN_SIZE(w,h)    BTN_W(w), BTN_H(h)

// Draw a reference grid for ease of spacing out widgets.
#define DRAW_LAYOUT_GRID \
  { \
    cmd.cmd(LINE_WIDTH(4)); \
    for(int i = 1; i < GRID_COLS; i++) { \
      cmd.cmd(BEGIN(LINES)); \
      cmd.cmd(VERTEX2II(GRID_X(i),0)); \
      cmd.cmd(VERTEX2II(GRID_X(i),Vsize)); \
    } \
    for(int i = 1; i < GRID_ROWS; i++) { \
      cmd.cmd(BEGIN(LINES)); \
      cmd.cmd(VERTEX2II(0,     GRID_Y(i))); \
      cmd.cmd(VERTEX2II(Hsize, GRID_Y(i))); \
    } \
    cmd.cmd(LINE_WIDTH(16)); \
  }

/**************************** Enhanced Command Processor **************************/

/* The CommandProcessor class wraps the CommandFifo with several features to make
 * defining user interfaces much easier.
 *
 *   - Implements chaining on all methods
 *   - Automatically adds text to button, toggle, text and keys.
 *   - Constrains all widgets to fit inside a box for ease of layout.
 *   - Font size is specified using a chained modifier.
 *   - Option argument is given the default OPT_3D value.
 */

class CommandProcessor : public CLCD::CommandFifo {
  private:
    int16_t  _font    = 26;

  public:
    inline CommandProcessor& cmd     (uint32_t cmd32)           {CLCD::CommandFifo::cmd(cmd32); return *this;}
    inline CommandProcessor& cmd     (void* data, uint16_t len) {CLCD::CommandFifo::cmd(data, len); return *this;}

    inline CommandProcessor& fgcolor (uint32_t rgb)             {cmd(FTDI::CMD_FGCOLOR); cmd(rgb); return *this;}
    inline CommandProcessor& bgcolor (uint32_t rgb)             {cmd(FTDI::CMD_BGCOLOR); cmd(rgb); return *this;}
    inline CommandProcessor& tag     (uint8_t  tag)             {cmd(FTDI::TAG(tag)); return *this;}

    inline CommandProcessor& font    (int16_t  font)            {_font = font; return *this;}

    template<class style>
    inline CommandProcessor& enabled(bool enabled) {
      using namespace FTDI;
      if(enabled) {
        cmd(COLOR_RGB(style::rgb_enabled))
           .fgcolor(style::fg_enabled);
      } else {
        cmd(COLOR_RGB(style::rgb_disabled))
           .fgcolor(style::fg_disabled)
           .cmd(TAG(0));
      }
      return *this;
    }

    template<typename T>
    FORCEDINLINE CommandProcessor& toggle(int16_t x, int16_t y, int16_t w, int16_t h, T text, bool state, uint16_t options = FTDI::OPT_3D) {
      FontMetrics fm;
      CLCD::get_font_metrics(_font, fm);
      const int16_t widget_h    = fm.height * 20.0/16;
      const int16_t outer_bar_r = widget_h / 2;
      const int16_t knob_r      = outer_bar_r - 1.5;
      // The y coordinate of the toggle is the baseline of the text,
      // so we must introduce a fudge factor based on the line height to
      // actually center the control.
      const int16_t fudge_y = fm.height*5/16;
      CLCD::CommandFifo::toggle(x + h, y + h/2 - widget_h/2 + fudge_y, w - h*2, _font, options, state);
      CLCD::CommandFifo::str(text);
      return *this;
    }

    // Contrained drawing routines. These constrain the widget inside a box for easier layout.
    // The FORCEDINLINE ensures that the code is inlined so that all the math is done at compile time.

    FORCEDINLINE CommandProcessor& track(int16_t x, int16_t y, int16_t w, int16_t h, int16_t tag, bool rotary) {
      if(rotary)
        CLCD::CommandFifo::track(x + w/2, y + h/2, 1, 1, tag);
      else
        CLCD::CommandFifo::track(x + h, y + h / 4, w - h*2, h / 2, tag);
      return *this;
    }

    FORCEDINLINE CommandProcessor& clock(int16_t x, int16_t y, int16_t w, int16_t h, int16_t hr, int16_t m, int16_t s, int16_t ms, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::clock(x + w/2, y + h/2, min(w,h)/2, options, hr, m, s, ms);
      return *this;
    }

    FORCEDINLINE CommandProcessor& gauge(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t major, uint16_t minor, uint16_t val, uint16_t range, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::gauge(x + w/2, y + h/2, min(w,h)/2, options, major, minor, val, range);
      return *this;
    }

    FORCEDINLINE CommandProcessor& dial(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t val, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::dial(x + w/2, y + h/2, min(w,h)/2, options, val);
      return *this;
    }

    FORCEDINLINE CommandProcessor& slider(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t val, uint16_t range, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::slider(x + h, y + h / 4, w - h*2, h / 2, options, val, range);
      return *this;
    }

    FORCEDINLINE CommandProcessor& progress(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t val, uint16_t range, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::progress(x + h, y + h / 4, w - h*2, h / 2, options, val, range);
      return *this;
    }

    FORCEDINLINE CommandProcessor& scrollbar(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t val, uint16_t size, uint16_t range, uint16_t options = 0) {
      CLCD::CommandFifo::scrollbar(x + h, y + h / 4, w - h*2, h / 2, options, val, size, range);
      return *this;
    }

    template<typename T> FORCEDINLINE
    CommandProcessor& text(int16_t x, int16_t y, int16_t w, int16_t h, T text, uint16_t options = FTDI::OPT_CENTER) {
      using namespace FTDI;
      CLCD::CommandFifo::text(
        x + ((options & OPT_CENTERX) ? w/2 : ((options & OPT_RIGHTX) ? w : 0)),
        y + ((options & OPT_CENTERY) ? h/2 : h),
        _font, options);
      CLCD::CommandFifo::str(text);
      return *this;
    }

    FORCEDINLINE CommandProcessor& icon(int16_t x, int16_t y, int16_t w, int16_t h, const FTDI::bitmap_info_t& info, const float scale = 1) {
      using namespace FTDI;
      cmd(BEGIN(BITMAPS));
      if(scale != 1) {
        cmd(BITMAP_TRANSFORM_A(uint32_t(float(256)/scale)));
        cmd(BITMAP_TRANSFORM_E(uint32_t(float(256)/scale)));
      }
      cmd(BITMAP_SIZE(info.filter, info.wrapx, info.wrapy, info.width*scale, info.height*scale));
      cmd(VERTEX2F((x + w/2 - info.width*scale/2)*16, (y + h/2 - info.height*scale/2)*16));
      if(scale != 1) {
        cmd(BITMAP_TRANSFORM_A(256));
        cmd(BITMAP_TRANSFORM_E(256));
      }
      return *this;
    }

    template<typename T>
    CommandProcessor& button(int16_t x, int16_t y, int16_t w, int16_t h, T text, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::button(x, y, w, h, _font, options);
      CLCD::CommandFifo::str(text);
      return *this;
    }

    template<typename T>
    CommandProcessor& keys(int16_t x, int16_t y, int16_t w, int16_t h, T keys, uint16_t options = FTDI::OPT_3D) {
      CLCD::CommandFifo::keys(x, y, w, h, _font, options);
      CLCD::CommandFifo::str(keys);
      return *this;
    }
};

#endif // _UI_BUILDER_H_