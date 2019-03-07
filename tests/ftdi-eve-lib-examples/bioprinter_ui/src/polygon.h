/*************
 * polygon.h *
 *************/

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

using namespace FTDI;

class Polygon {
 private:
   uint16_t origin_x;
   uint16_t origin_y;
   uint16_t origin_s;

   uint16_t x_min, y_min, x_max, y_max;

   typedef void (Polygon::*path_op)();
   typedef void (Polygon::*vertex_op)(uint16_t x, uint16_t y);

   void add_to_bounds(uint16_t x, uint16_t y) {
    x_min = min(x_min, x);
    x_max = max(x_max, x);
    y_min = min(y_min, y);
    y_max = max(y_max, y);
   }

   void add_vertex_2f(uint16_t x, uint16_t y) {
     CommandProcessor cmd;
     cmd.cmd(VERTEX2F(x, y));
   }

   void draw_edge_strip_b() {
     CommandProcessor cmd;
     cmd.cmd(BEGIN(EDGE_STRIP_B));
   }

   void draw_line_strip() {
     CommandProcessor cmd;
     cmd.cmd(BEGIN(LINE_STRIP));
   }

   void noop() {
     CommandProcessor cmd;
     cmd.cmd(BEGIN(LINE_STRIP));
   }

 public:
   Polygon(uint16_t x_min, uint16_t y_min, uint16_t x_max, uint16_t y_max) {
    origin_s = max(x_max - x_min, y_max - y_min) * 16;
    origin_x = x_min * 16;
    origin_y = y_min * 16;
   }

   void vertices(const uint16_t data[], size_t n, path_op p_op, vertex_op v_op) {
    uint16_t poly_start_x = 0xFFFF, poly_start_y;
    uint32_t last_x = -1, last_y = -1;
    CommandProcessor cmd;
    //cmd.cmd(BEGIN(primitive));
    (this->*p_op)();
    for(size_t i = 0; i < n; i += 2) {
      const uint16_t x = pgm_read_word_far(&data[i+0]);
      if(x == 0xFFFF) {
        if(poly_start_x != 0xFFFF) {
          (this->*v_op)(poly_start_x, poly_start_y);
          //
          poly_start_x = 0xFFFF;
        }
        //cmd.cmd(BEGIN(primitive));
        (this->*p_op)();
        i += 1;
        continue;
        poly_start_x = 0xFFFF;
      }
      const uint16_t y = pgm_read_word_far(&data[i+1]);
      const uint32_t scaled_x = uint32_t(x) * origin_s / 0xFFFE + origin_x;
      const uint32_t scaled_y = uint32_t(y) * origin_s / 0xFFFE + origin_y;
      if(last_x != scaled_x || last_y != scaled_y)
        //cmd.cmd(VERTEX2F(scaled_x, scaled_y));
        (this->*v_op)(scaled_x, scaled_y);
      last_x = scaled_x;
      last_y = scaled_y;
      if(poly_start_x == 0xFFFF) {
        poly_start_x = scaled_x;
        poly_start_y = scaled_y;
      }
    }
    (this->*v_op)(poly_start_x, poly_start_y);
    //cmd.cmd(VERTEX2F(poly_start_x, poly_start_y));
  }

  void fill(const uint16_t data[], size_t n) {
    CommandProcessor cmd;
    cmd.cmd(SAVE_CONTEXT());
    cmd.cmd(CLEAR(0,1,0));
    cmd.cmd(COLOR_MASK(0,0,0,0));
    cmd.cmd(STENCIL_OP(STENCIL_OP_KEEP, STENCIL_OP_INVERT));
    cmd.cmd(STENCIL_FUNC(STENCIL_FUNC_ALWAYS, 255, 255));
    vertices(data, n, &draw_edge_strip_b, &add_vertex_2f);
    cmd.cmd(RESTORE_CONTEXT());

    cmd.cmd(SAVE_CONTEXT());
    cmd.cmd(STENCIL_FUNC(STENCIL_FUNC_NOTEQUAL, 0, 255));
    cmd.cmd(BEGIN(RECTS));
    cmd.cmd(VERTEX2F( 0  * 16,  0  * 16));
    cmd.cmd(VERTEX2F(display_width * 16, display_height * 16));
    cmd.cmd(RESTORE_CONTEXT());
  }

  void stroke(const uint16_t data[], size_t n) {
    vertices(data, n, &draw_line_strip, &add_vertex_2f);
  }

  void shadow(const uint16_t data[], size_t n, uint8_t shadow_depth = 5, uint32_t color = 0xF3E0E0) {
    CommandProcessor cmd;
    cmd.cmd(SAVE_CONTEXT());
    cmd.cmd(COLOR_RGB(color));
    cmd.cmd(VERTEX_TRANSLATE_X(shadow_depth * 16));
    cmd.cmd(VERTEX_TRANSLATE_Y(shadow_depth * 16));
    fill(data, n);
    cmd.cmd(RESTORE_CONTEXT());
  }

  void button(const uint8_t t, const uint16_t data[], size_t n, uint8_t shadow_depth = 5) {
    CommandProcessor cmd;
    cmd.tag(t);
    shadow(data, n, shadow_depth);
    if(EventLoop::get_pressed_tag() == t) {
      cmd.cmd(SAVE_CONTEXT());
      cmd.cmd(VERTEX_TRANSLATE_X(2 * 16));
      cmd.cmd(VERTEX_TRANSLATE_Y(2 * 16));
      fill(data, n);
      stroke(data, n);
      cmd.cmd(RESTORE_CONTEXT());
    } else {
      fill(data, n);
      stroke(data, n);
    }
  }

  void bounds(const uint16_t data[], size_t n, uint16_t &x, uint16_t &y, uint16_t &w, uint16_t &h) {
    x_min = UINT16_MAX;
    y_min = UINT16_MAX;
    x_max = 0;
    y_max = 0;
    vertices(data, n, &noop, &add_to_bounds);
    x = x_min/16;
    y = y_min/16;
    w = (x_max - x_min)/16;
    h = (y_max - y_min)/16;
  }
};