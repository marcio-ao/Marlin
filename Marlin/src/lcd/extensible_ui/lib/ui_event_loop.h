/*******************
 * ui_event_loop.h *
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

#ifndef _UI_EVENT_LOOP_
#define _UI_EVENT_LOOP_

#define DISPLAY_UPDATE_INTERVAL    1000
#define TRACKING_UPDATE_INTERVAL     50
#define TOUCH_REPEATS_PER_SECOND      4
#define DEBOUNCE_PERIOD             100

void start_tracking(int16_t x, int16_t y, int16_t w, int16_t h, int16_t tag, bool rotary);

#endif // _UI_EVENT_LOOP_