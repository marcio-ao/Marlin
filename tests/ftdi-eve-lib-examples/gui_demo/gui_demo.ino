/****************
 * gui_demo.ino *
 ****************/

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

using namespace FTDI;

/***************** SOUND SEQUENCES *****************/

const PROGMEM SoundPlayer::sound_t twinkle[] = {
  {GLOCKENSPIEL, NOTE_C4,  1},
  {GLOCKENSPIEL, NOTE_E4,  1},
  {GLOCKENSPIEL, NOTE_G4,  16},
  {SILENCE,      END_SONG, 0}
};

/***************** SCREEN DEFINITIONS *****************/

class SecondScreen;

class MainScreen : public UIScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class SecondScreen : public UIScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

void MainScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(0x888888));
  cmd.cmd(CLEAR(true,true,true));

  #define GRID_COLS 4
  #define GRID_ROWS 7
  cmd.font(30)
     .tag(0).text(  BTN_POS(1,3), BTN_SIZE(4,1), F("Hello World!"))
     .tag(1).button(BTN_POS(2,5), BTN_SIZE(2,1), F("Okay"));
}
    
bool MainScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:
      sound.play(twinkle, PLAY_ASYNCHRONOUS);
      GOTO_SCREEN(SecondScreen);
      return true;
    default:
      return false;
  }
}

void SecondScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(0xFF8888));
  cmd.cmd(CLEAR(true,true,true));

  #define GRID_COLS 4
  #define GRID_ROWS 7
  cmd.font(30)
     .tag(0).text(  BTN_POS(1,3), BTN_SIZE(4,1), F("Hi there!"))
     .tag(1).button(BTN_POS(2,5), BTN_SIZE(2,1), F("Back"));
}

bool SecondScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:
      GOTO_PREVIOUS();
      return true;
    default:
      return false;
  }
}

/***************** LIST OF SCREENS *****************/

SCREEN_TABLE {
  DECL_SCREEN(MainScreen),
  DECL_SCREEN(SecondScreen)
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
