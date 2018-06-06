/******************
 * ui_screens.cpp *
 ******************/

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

#include "../../../sd/SdFile.h"
#include "../../../sd/cardreader.h"

#if ENABLED(EXTENSIBLE_UI)

#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"
#include "ftdi_eve_panels.h"
#include "ftdi_eve_dl.h"

#include "ui_builder.h"
#include "ui_sounds.h"
#include "ui_bitmaps.h"
#include "ui_screens.h"
#include "ui_theme.h"
#include "ui_event_loop.h"

using namespace FTDI;

#define N_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define THEME(t) fgcolor(Theme::t)
#define BTN_EN_THEME(en, theme) enabled(en)

/****************************** SCREEN STATIC DATA *************************/

// To save RAM, store state information related to a particular screen
// in a union. The values should be initialized in the onEntry method.

static union {
  struct {uint8_t increment;}          ValueAdjusters;
  struct {uint8_t page, selected_tag;} FilesScreen;
} screen_data;

/******************************* MENU SCREEN TABLE ******************************/

SCREEN_TABLE {
  //DECL_SCREEN(WidgetsScreen),
  DECL_SCREEN(BootScreen),
  DECL_SCREEN(AboutScreen),
  DECL_SCREEN(AlertBoxScreen),
  DECL_SCREEN(RestoreFailsafeScreen),
  DECL_SCREEN(ConfirmAbortPrint),
  DECL_SCREEN(CalibrationScreen),
  DECL_SCREEN(StatusScreen),
  DECL_SCREEN(MenuScreen),
  DECL_SCREEN(TuneScreen),
  DECL_SCREEN(MoveAxisScreen),
  DECL_SCREEN(AdvancedSettingsScreen),
  DECL_SCREEN(StepsScreen),
#if HAS_BED_PROBE
  DECL_SCREEN(ZOffsetScreen),
#endif
  DECL_SCREEN(FeedrateScreen),
  DECL_SCREEN(VelocityScreen),
  DECL_SCREEN(AccelerationScreen),
  DECL_SCREEN(JerkScreen),
  DECL_SCREEN(TemperatureScreen),
  DECL_SCREEN(CalibrationRegistersScreen),
  DECL_SCREEN(FilesScreen),
  DECL_SCREEN(MediaPlayerScreen),
};

SCREEN_TABLE_POST

/****************** BUTTON HIGHTLIGHTING ROUTINE ***********************/

enum {
  STYLE_DISABLED = 0x01,
  STYLE_RED_BTN  = 0x02
};

void UIScreenWithStyles::onEntry() {
  CommandProcessor cmd;
  cmd.set_button_style_callback(buttonStyleCallback);
  UIScreen::onEntry();
}

bool UIScreenWithStyles::buttonStyleCallback(uint8_t tag, uint8_t &style, uint16_t &options, bool post) {
  using namespace FTDI;
  CommandProcessor cmd;

  if(post) {
    default_button_colors();
    return false;
  }

  if(tag != 0 && get_pressed_tag() == tag) {
    options = OPT_FLAT;
    return false;
  }

  if(style & STYLE_RED_BTN) {
    if(style & STYLE_DISABLED) {
      cmd.cmd(COLOR_RGB(Theme::red_btn::rgb_disabled))
         .fgcolor(Theme::red_btn::fg_disabled)
         .tag(0);
      style &= ~STYLE_DISABLED; // Clear the disabled flag
    } else {
      cmd.cmd(COLOR_RGB(Theme::red_btn::rgb_enabled))
         .fgcolor(Theme::red_btn::fg_enabled);
    }
    return true;              // Call me again to reset the colors
  }

  if(style & STYLE_DISABLED) {
    cmd.cmd(COLOR_RGB(Theme::default_btn::rgb_disabled))
       .fgcolor(Theme::default_btn::fg_disabled)
       .tag(0);
    style &= ~STYLE_DISABLED; // Clear the disabled flag
    return true;              // Call me again to reset the colors
  }
  return false;
}

void UIScreenWithStyles::default_button_colors() {
  using namespace FTDI;
  CommandProcessor cmd;
  cmd.cmd(COLOR_RGB(Theme::default_btn::rgb_enabled))
     .fgcolor(Theme::default_btn::fg_enabled);
}

/******************************** BOOT SCREEN ****************************/

void BootScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background));
  cmd.cmd(CLEAR(true,true,true));

  CLCD::turn_on_backlight();
  FTDI::SoundPlayer::set_volume(255);
}

void BootScreen::onIdle() {
  if(CLCD::is_touching()) {
    // If the user is touching the screen at startup, then
    // assume the user wants to re-calibrate the screen.
    // This gives the user the ability to recover a
    // miscalibration that has been stored to EEPROM.
    GOTO_SCREEN(CalibrationScreen);
  } else {
    GOTO_SCREEN(StatusScreen);
  }
}

/******************************** ABOUT SCREEN ****************************/

void AboutScreen::onEntry() {
  UIScreen::onEntry();
  sound.play(chimes, PLAY_ASYNCHRONOUS);
}

void AboutScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background));
  cmd.cmd(CLEAR(true,true,true));

  #define GRID_COLS 4
  #define GRID_ROWS 8

  cmd       .font(Theme::font_large) .text(  BTN_POS(1,2), BTN_SIZE(4,1), F("Color Touch Panel"))
     .tag(2).font(Theme::font_medium).text(  BTN_POS(1,3), BTN_SIZE(4,1), F("(c) 2018 Aleph Objects, Inc."))
                                     .text(  BTN_POS(1,5), BTN_SIZE(4,1), getFirmwareName())
     .tag(1)                         .button(BTN_POS(2,7), BTN_SIZE(2,1), F("Okay"));

  #undef GRID_COLS
  #undef GRID_ROWS
}

bool AboutScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:        GOTO_PREVIOUS();                         return true;
    case 2:        GOTO_SCREEN(CalibrationRegistersScreen); return true;
    default:                                                return false;
  }
}

/**************************** GENERIC DIALOG BOX SCREEN ****************************/

void DialogBoxBaseClass::onEntry() {
  repaintBackground();
  UIScreen::onEntry();
}

void DialogBoxBaseClass::drawDialog(const progmem_str lines[], size_t n_lines, progmem_str btn1, progmem_str btn2 ) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background));
  cmd.cmd(CLEAR(true,true,true));

  #define GRID_COLS 2
  #define GRID_ROWS 8

  cmd.font(Theme::font_large);
  for(uint8_t line = 0; line < n_lines; line++) {
    cmd.text  ( BTN_POS(1,3-n_lines/2+line), BTN_SIZE(2,1), lines[line]);
  }

  cmd.font(Theme::font_medium);
  if(btn1 && btn2) {
    cmd.tag(1).button( BTN_POS(1,8), BTN_SIZE(1,1), btn1);
    cmd.tag(2).button( BTN_POS(2,8), BTN_SIZE(1,1), btn2);
  } else if(btn1) {
    cmd.tag(1).button( BTN_POS(1,8), BTN_SIZE(2,1), btn1);
  } else if(btn2) {
    cmd.tag(2).button( BTN_POS(1,8), BTN_SIZE(2,1), btn2);
  }

  #undef GRID_COLS
  #undef GRID_ROWS
}

bool DialogBoxBaseClass::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1: GOTO_PREVIOUS(); return true;
    case 2: GOTO_PREVIOUS(); return true;
    default:                 return false;
  }
}

/****************************** ALERT BOX SCREEN *****************************/

void AlertBoxScreen::onEntry() {
  UIScreen::onEntry();
}

void AlertBoxScreen::onRedraw(draw_mode_t what) {
  DialogBoxBaseClass::onRedraw(what);
}

void AlertBoxScreen::show(const progmem_str line1, const progmem_str line2, const progmem_str line3) {
  CommandProcessor cmd;
  cmd.cmd(CMD_DLSTART);

  progmem_str lines[] = {line1, line2, line3};
  drawDialog(lines, line3 ? 3 : line2 ? 2 : 1, F("Okay"), 0);

  if(!storeBackground()) {
    #if defined (SERIAL_PROTOCOLLNPAIR)
      SERIAL_PROTOCOLLN("Unable to set the confirmation message, not enough DL cache space");
    #else
      #if defined(UI_FRAMEWORK_DEBUG)
        Serial.print(F("Unable to set the confirmation message, not enough DL cache space"));
      #endif
    #endif
  }

  sound.play(c_maj_arpeggio, PLAY_SYNCHRONOUS);
  GOTO_SCREEN(AlertBoxScreen);
}

/**************************** RESTORE FAILSAFE SCREEN ***************************/

void RestoreFailsafeScreen::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    progmem_str lines[] = {
      F("Are you sure?"),
      F("Customizations will be lost.")
    };
    drawDialog(lines, N_ELEMENTS(lines), F("Yes"), F("No"));
  }
}

bool RestoreFailsafeScreen::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 1:
      enqueueCommands(F("M502\nM500"));
      AlertBoxScreen::show(F("Factory settings restored."));
      // Remove RestoreFailsafeScreen from the stack
      // so the alert box doesn't return to it.
      current_screen.forget();
      return true;
    default:
      return DialogBoxBaseClass::onTouchEnd(tag);
  }
}

/**************************** RESTORE FAILSAFE SCREEN ***************************/

void ConfirmAbortPrint::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    progmem_str lines[] = {
      F("Are you sure you want"),
      F("to stop the print?")
    };
    drawDialog(lines, N_ELEMENTS(lines), F("Yes"), F("No"));
  }
}

bool ConfirmAbortPrint::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 1:
      #if defined (SERIAL_PROTOCOLLNPAIR)
        SERIAL_PROTOCOLLN("Abort confirmed");
      #else
        #if defined(UI_FRAMEWORK_DEBUG)
          Serial.print(F("Abort confirmed"));
        #endif
      #endif
      GOTO_PREVIOUS();
      stopPrint();
      return true;
    default:
      return DialogBoxBaseClass::onTouchEnd(tag);
  }
}

/************************************ KILL SCREEN *******************************/

// The kill screen is an oddball that happens after Marlin has killed the events
// loop. So we only have a show() method rather than onRedraw(). The KillScreen
// should not be used as a model for other UI screens as it is an exception.

void KillScreen::show(progmem_str message) {
  CommandProcessor cmd;

  cmd.cmd(CMD_DLSTART)
     .cmd(CLEAR_COLOR_RGB(Theme::background))
     .cmd(CLEAR(true,true,true));

  #define GRID_COLS 4
  #define GRID_ROWS 8

  cmd.font(Theme::font_large)
     .text(BTN_POS(1,2), BTN_SIZE(4,1), message)
     .text(BTN_POS(1,3), BTN_SIZE(4,1), F("PRINTER HALTED"))
     .text(BTN_POS(1,6), BTN_SIZE(4,1), F("Please reset"));

  #undef GRID_COLS
  #undef GRID_ROWS

  cmd.cmd(DL::DL_DISPLAY)
     .cmd(CMD_SWAP)
     .execute();

  sound.play(sad_trombone, PLAY_SYNCHRONOUS);
}

/*********************************** STATUS SCREEN ******************************/
#if defined(USE_PORTRAIT_ORIENTATION)
  #define GRID_ROWS 9
#else
  #define GRID_ROWS 8
#endif

void StatusScreen::draw_axis_position(draw_mode_t what) {
  CommandProcessor cmd;

  #define GRID_COLS 3

  if(what & BACKGROUND) {
    cmd.tag(6)
    #if defined(USE_PORTRAIT_ORIENTATION)
      .THEME(axis_label) .font(Theme::font_large)
                         .button( BTN_POS(1,5), BTN_SIZE(2,1), F(""), OPT_FLAT)
                         .button( BTN_POS(1,6), BTN_SIZE(2,1), F(""), OPT_FLAT)
                         .button( BTN_POS(1,7), BTN_SIZE(2,1), F(""), OPT_FLAT)

                         .font(Theme::font_small)
                         .text  ( BTN_POS(1,5), BTN_SIZE(1,1), F("X"))
                         .text  ( BTN_POS(1,6), BTN_SIZE(1,1), F("Y"))
                         .text  ( BTN_POS(1,7), BTN_SIZE(1,1), F("Z"))

                         .font(Theme::font_medium)
      .THEME(x_axis)     .button( BTN_POS(2,5), BTN_SIZE(2,1), F(""), OPT_FLAT)
      .THEME(y_axis)     .button( BTN_POS(2,6), BTN_SIZE(2,1), F(""), OPT_FLAT)
      .THEME(z_axis)     .button( BTN_POS(2,7), BTN_SIZE(2,1), F(""), OPT_FLAT);
    #else
      .THEME(axis_label) .font(Theme::font_large)
                         .button( BTN_POS(1,5), BTN_SIZE(1,2), F(""),  OPT_FLAT)
                         .button( BTN_POS(2,5), BTN_SIZE(1,2), F(""),  OPT_FLAT)
                         .button( BTN_POS(3,5), BTN_SIZE(1,2), F(""),  OPT_FLAT)

                         .font(Theme::font_small)
                         .text  ( BTN_POS(1,5), BTN_SIZE(1,1), F("X"))
                         .text  ( BTN_POS(2,5), BTN_SIZE(1,1), F("Y"))
                         .text  ( BTN_POS(3,5), BTN_SIZE(1,1), F("Z"))
                         .font(Theme::font_medium)

      .THEME(x_axis)     .button( BTN_POS(1,6), BTN_SIZE(1,1), F(""), OPT_FLAT)
      .THEME(y_axis)     .button( BTN_POS(2,6), BTN_SIZE(1,1), F(""), OPT_FLAT)
      .THEME(z_axis)     .button( BTN_POS(3,6), BTN_SIZE(1,1), F(""), OPT_FLAT);
    #endif
  }

  if(what & FOREGROUND) {
    using namespace Extensible_UI_API;
    char x_str[15];
    char y_str[15];
    char z_str[15];

    dtostrf(getAxisPosition_mm(X), 5, 1, x_str);
    dtostrf(getAxisPosition_mm(Y), 5, 1, y_str);
    dtostrf(getAxisPosition_mm(Z), 5, 1, z_str);

    strcat_P(x_str, PSTR(" mm"));
    strcat_P(y_str, PSTR(" mm"));
    strcat_P(z_str, PSTR(" mm"));

    cmd.tag(6).font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
         .text  ( BTN_POS(2,5), BTN_SIZE(2,1), x_str)
         .text  ( BTN_POS(2,6), BTN_SIZE(2,1), y_str)
         .text  ( BTN_POS(2,7), BTN_SIZE(2,1), z_str);
    #else
         .text  ( BTN_POS(1,6), BTN_SIZE(1,1), x_str)
         .text  ( BTN_POS(2,6), BTN_SIZE(1,1), y_str)
         .text  ( BTN_POS(3,6), BTN_SIZE(1,1), z_str);
    #endif
  }

  #undef GRID_COLS
}

#if defined(USE_PORTRAIT_ORIENTATION)
  #define GRID_COLS 8
#else
  #define GRID_COLS 12
#endif

#define ROUND(val) uint16_t((val)+0.5)

void StatusScreen::draw_temperature(draw_mode_t what) {
  CommandProcessor cmd;

  if(what & BACKGROUND) {
    cmd.font(Theme::font_small)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .tag(5)
       .THEME(temp)      .button( BTN_POS(1,1), BTN_SIZE(4,2), F(""), OPT_FLAT)
                         .button( BTN_POS(1,1), BTN_SIZE(8,1), F(""), OPT_FLAT)
       .THEME(fan_speed) .button( BTN_POS(5,2), BTN_SIZE(4,1), F(""), OPT_FLAT)
       .tag(0)
       .THEME(progress)  .button( BTN_POS(1,3), BTN_SIZE(4,1), F(""), OPT_FLAT)
                         .button( BTN_POS(5,3), BTN_SIZE(4,1), F(""), OPT_FLAT);
    #else
       .tag(5)
       .THEME(temp)      .button( BTN_POS(1,1), BTN_SIZE(4,2), F(""), OPT_FLAT)
                         .button( BTN_POS(1,1), BTN_SIZE(8,1), F(""), OPT_FLAT)
       .THEME(fan_speed) .button( BTN_POS(5,2), BTN_SIZE(4,1), F(""), OPT_FLAT)
       .tag(0)
       .THEME(progress)  .button( BTN_POS(9,1), BTN_SIZE(4,1), F(""), OPT_FLAT)
                         .button( BTN_POS(9,2), BTN_SIZE(4,1), F(""), OPT_FLAT);
    #endif

    // Draw Extruder Bitmap on Extruder Temperature Button

    cmd.tag(5)
       .cmd(BITMAP_SOURCE(Extruder_Icon_Info))
       .cmd(BITMAP_LAYOUT(Extruder_Icon_Info))
       .cmd(BITMAP_SIZE  (Extruder_Icon_Info))
       .icon (BTN_POS(1,1), BTN_SIZE(1,1),  Extruder_Icon_Info, Theme::icon_scale)
       .icon (BTN_POS(5,1), BTN_SIZE(1,1),  Extruder_Icon_Info, Theme::icon_scale);

    // Draw Bed Heat Bitmap on Bed Heat Button
    cmd.cmd(BITMAP_SOURCE(Bed_Heat_Icon_Info))
       .cmd(BITMAP_LAYOUT(Bed_Heat_Icon_Info))
       .cmd(BITMAP_SIZE  (Bed_Heat_Icon_Info))
       .icon (BTN_POS(1,2), BTN_SIZE(1,1), Bed_Heat_Icon_Info, Theme::icon_scale);

    // Draw Fan Percent Bitmap on Bed Heat Button

    cmd.cmd(BITMAP_SOURCE(Fan_Icon_Info))
       .cmd(BITMAP_LAYOUT(Fan_Icon_Info))
       .cmd(BITMAP_SIZE  (Fan_Icon_Info))
       .icon  (BTN_POS(5,2), BTN_SIZE(1,1), Fan_Icon_Info, Theme::icon_scale);
  }

  if(what & FOREGROUND) {
    using namespace Extensible_UI_API;
    char e0_str[15];
    char e1_str[15];
    char bed_str[15];
    char fan_str[15];

    sprintf_P(
      fan_str,
      PSTR("%-3d %%"),
      int8_t(getFan_percent(0))
    );

    sprintf_P(
      bed_str,
      PSTR("%-3d / %-3d  " ),
      ROUND(getActualTemp_celsius(0)),
      ROUND(getTargetTemp_celsius(0))
    );

    sprintf_P(
      e0_str,
      PSTR("%-3d / %-3d C"),
      ROUND(getActualTemp_celsius(1)),
      ROUND(getTargetTemp_celsius(1))
    );

    #if EXTRUDERS == 2
      sprintf_P(
        e1_str,
        PSTR("%-3d / %-3d C"),
        ROUND(getActualTemp_celsius(2)),
        ROUND(getTargetTemp_celsius(2))
      );
    #else
      strcpy_P(
        e1_str,
        PSTR("-")
      );
    #endif

    cmd.tag(5)
       .font(Theme::font_medium)
       .text(BTN_POS(2,1), BTN_SIZE(3,1), e0_str)
       .text(BTN_POS(6,1), BTN_SIZE(3,1), e1_str)
       .text(BTN_POS(2,2), BTN_SIZE(3,1), bed_str)
       .text(BTN_POS(6,2), BTN_SIZE(3,1), fan_str);
  }
}

void StatusScreen::draw_progress(draw_mode_t what) {
  using namespace Extensible_UI_API;
  CommandProcessor cmd;

  if(what & BACKGROUND) {
    cmd.tag(0).font(Theme::font_small)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .THEME(progress) .button(BTN_POS(1,3), BTN_SIZE(4,1), F(""), OPT_FLAT)
                        .button(BTN_POS(5,3), BTN_SIZE(4,1), F(""), OPT_FLAT);
    #else
       .THEME(progress) .button(BTN_POS(9,1), BTN_SIZE(4,1), F(""), OPT_FLAT)
                        .button(BTN_POS(9,2), BTN_SIZE(4,1), F(""), OPT_FLAT);
    #endif
  }

  if(what & FOREGROUND) {
    const uint32_t elapsed = getProgress_seconds_elapsed();
    const uint8_t hrs = elapsed/3600;
    const uint8_t min = (elapsed/60)%60;

    char time_str[10];
    char progress_str[10];

    sprintf_P(time_str,     PSTR(" %02d : %02d"), hrs, min);
    sprintf_P(progress_str, PSTR("%-3d %%"),      getProgress_percent() );

    cmd.font(Theme::font_small)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .tag(0).text(BTN_POS(1,3), BTN_SIZE(4,1), time_str)
              .text(BTN_POS(5,3), BTN_SIZE(4,1), progress_str);
    #else
       .tag(0).text(BTN_POS(9,1), BTN_SIZE(4,1), time_str)
              .text(BTN_POS(9,2), BTN_SIZE(4,1), progress_str);
    #endif
  }
}

#undef GRID_COLS


void StatusScreen::draw_interaction_buttons(draw_mode_t what) {
  CommandProcessor cmd;
  default_button_colors();

  #define GRID_COLS 4

  if(what & BACKGROUND) {
    using namespace Extensible_UI_API;

    // Draw the media icon

    if(isMediaInserted()) {
      cmd.tag(3);
    } else {
      cmd.tag(0)
         .cmd(COLOR_RGB(Theme::status_bg))
         .THEME(status_bg);
    }

    cmd.font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .button( BTN_POS(1,9), BTN_SIZE(2,1), F(""));
    #else
       .button( BTN_POS(3,7), BTN_SIZE(1,2), F(""));
    #endif

    if(!isMediaInserted()) {
      cmd.cmd(COLOR_RGB(Theme::status_dark));
    }

    // Draw Thumb Drive Bitmap on USB Button

    cmd.cmd(BITMAP_SOURCE(TD_Icon_Info))
       .cmd(BITMAP_LAYOUT(TD_Icon_Info))
       .cmd(BITMAP_SIZE  (TD_Icon_Info))

    #if defined(USE_PORTRAIT_ORIENTATION)
       .icon  (BTN_POS(1,9), BTN_SIZE(2,1), TD_Icon_Info, Theme::icon_scale);
    #else
       .icon  (BTN_POS(3,7), BTN_SIZE(1,2), TD_Icon_Info, Theme::icon_scale);
    #endif

    cmd.cmd(COLOR_RGB(0xFFFFFF)).fgcolor(Theme::default_btn::fg_enabled); // Reset foreground color
  }

  if(what & FOREGROUND) {
    using namespace Extensible_UI_API;
    CommandProcessor cmd;
    cmd
       .font(Theme::font_medium)
       .style(STYLE_RED_BTN)
       .enabled(isPrintingFromMedia())
    #if defined(USE_PORTRAIT_ORIENTATION)
       .tag(1).button( BTN_POS(1,8), BTN_SIZE(4,1), F("STOP"))
    #else
       .tag(1).button( BTN_POS(1,7), BTN_SIZE(2,2), F("STOP"))
    #endif
       .style(0)
      #if defined(USE_PORTRAIT_ORIENTATION)
       .tag(4).button( BTN_POS(3,9), BTN_SIZE(2,1), F("MENU"));
      #else
       .tag(4).button( BTN_POS(4,7), BTN_SIZE(1,2), F("MENU"));
    #endif
  }

  #undef  GRID_COLS
}

void StatusScreen::draw_status_message(draw_mode_t what, const char * const message) {
  #define GRID_COLS 1
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.THEME(status_msg)
       .tag(0)
       .font(Theme::font_large)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .button( BTN_POS(1,4), BTN_SIZE(1,1), message, OPT_FLAT);
    #else
       .button( BTN_POS(1,3), BTN_SIZE(1,2), message, OPT_FLAT);
    #endif
  }
  #undef  GRID_COLS
}

void StatusScreen::setStatusMessage(progmem_str message) {
  char buff[strlen_P((const char * const)message)+1];
  strcpy_P(buff, (const char * const) message);
  setStatusMessage(buff);
}

void StatusScreen::setStatusMessage(const char * const message) {
  CommandProcessor cmd;
  cmd.cmd(CMD_DLSTART)
     .cmd(CLEAR_COLOR_RGB(Theme::status_bg))
     .cmd(CLEAR(true,true,true));

  draw_temperature(BACKGROUND);
  draw_progress(BACKGROUND);
  draw_axis_position(BACKGROUND);
  draw_status_message(BACKGROUND, message);
  draw_interaction_buttons(BACKGROUND);

  if(!storeBackground()) {
    #if defined (SERIAL_PROTOCOLLNPAIR)
      SERIAL_PROTOCOLLNPAIR("Unable to set the status message, not enough DL cache space: ", message);
    #else
      #if defined(UI_FRAMEWORK_DEBUG)
        Serial.print(F("Unable to set the status message, not enough DL cache space: "));
        Serial.println(message);
      #endif
    #endif
  }

  if(current_screen.getType() == current_screen.lookupScreen(StatusScreen::onRedraw)) {
    current_screen.onRefresh();
  }
}

void StatusScreen::onStartup() {
  // Load the bitmaps for the status screen

  CLCD::flash_write_rgb332_bitmap(TD_Icon_Info.RAMG_addr,       TD_Icon,       sizeof(TD_Icon));
  CLCD::flash_write_rgb332_bitmap(Extruder_Icon_Info.RAMG_addr, Extruder_Icon, sizeof(Extruder_Icon));
  CLCD::flash_write_rgb332_bitmap(Bed_Heat_Icon_Info.RAMG_addr, Bed_Heat_Icon, sizeof(Bed_Heat_Icon));
  CLCD::flash_write_rgb332_bitmap(Fan_Icon_Info.RAMG_addr,      Fan_Icon,      sizeof(Fan_Icon));

  setStatusMessage(F(WELCOME_MSG));
}

void StatusScreen::onRedraw(draw_mode_t what) {
  if(what & FOREGROUND) {
    draw_temperature(FOREGROUND);
    draw_progress(FOREGROUND);
    draw_axis_position(FOREGROUND);
    draw_interaction_buttons(FOREGROUND);
  }
}

void StatusScreen::onEntry() {
  onRefresh();
}

void StatusScreen::onIdle() {
  static tiny_timer_t status_timer;

  if(status_timer.elapsed(DISPLAY_UPDATE_INTERVAL)) {
    onRefresh();
    status_timer.start();
  }
}

bool StatusScreen::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 1:
      #if defined(UI_FRAMEWORK_DEBUG)
        #if defined (SERIAL_PROTOCOLLNPGM)
          SERIAL_PROTOCOLLNPGM("Aborting print");
        #endif
      #endif
      GOTO_SCREEN(ConfirmAbortPrint);
      break;
    case 3:  GOTO_SCREEN(FilesScreen);       break;
    case 4:
      if(isPrinting()) {
        GOTO_SCREEN(TuneScreen);
      } else {
        GOTO_SCREEN(MenuScreen);
      }
      break;
    case 5:  GOTO_SCREEN(TemperatureScreen); break;
    case 6:
      if(!isPrinting()) {
        GOTO_SCREEN(MoveAxisScreen);
      }
      break;
  }
  return true;
}

#undef GRID_ROWS

/************************************ MENU SCREEN *******************************/

void MenuScreen::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true));
  }

  if(what & FOREGROUND) {
    CommandProcessor cmd;
    default_button_colors();
    cmd.font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_ROWS 7
      #define GRID_COLS 2
        .tag(2).enabled(1).button( BTN_POS(1,1), BTN_SIZE(1,1), F("Auto Home"))
        .tag(3).enabled(1).button( BTN_POS(2,1), BTN_SIZE(1,1), F("Level X Axis"))
        .tag(4).enabled(1).button( BTN_POS(1,2), BTN_SIZE(1,1), F("Move Axis"))
        .tag(5).enabled(1).button( BTN_POS(2,2), BTN_SIZE(1,1), F("Motors Off"))
        .tag(6).enabled(1).button( BTN_POS(1,3), BTN_SIZE(2,1), F("Temperature"))
        .tag(7).enabled(0).button( BTN_POS(1,4), BTN_SIZE(2,1), F("Change Filament"))
        .tag(8).enabled(1).button( BTN_POS(1,5), BTN_SIZE(2,1), F("Advanced Settings"))
        .tag(9).enabled(1).button( BTN_POS(1,6), BTN_SIZE(2,1), F("About Firmware"))
        .THEME(back_btn)
        .tag(1).enabled(1).button( BTN_POS(1,7), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #else
      #define GRID_ROWS 5
      #define GRID_COLS 2
        .tag(2).enabled(1).button( BTN_POS(1,1), BTN_SIZE(1,1), F("Auto Home"))
        .tag(3).enabled(1).button( BTN_POS(2,1), BTN_SIZE(1,1), F("Level X Axis"))
        .tag(4).enabled(1).button( BTN_POS(1,2), BTN_SIZE(1,1), F("Move Axis"))
        .tag(5).enabled(1).button( BTN_POS(2,2), BTN_SIZE(1,1), F("Motors Off"))
        .tag(6).enabled(1).button( BTN_POS(1,3), BTN_SIZE(1,1), F("Temperature"))
        .tag(7).enabled(0).button( BTN_POS(2,3), BTN_SIZE(1,1), F("Change Filament"))
        .tag(8).enabled(1).button( BTN_POS(1,4), BTN_SIZE(1,1), F("Advanced Settings"))
        .tag(9).enabled(1).button( BTN_POS(2,4), BTN_SIZE(1,1), F("About Firmware"))
        .tag(1).enabled(1).button( BTN_POS(1,5), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #endif
  }
}

bool MenuScreen::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 1:  GOTO_PREVIOUS();                                         break;
    case 2:  enqueueCommands(F("G28"));                               break;
    #if defined(LULZBOT_MENU_AXIS_LEVELING_COMMANDS)
    case 3:  enqueueCommands(F(LULZBOT_MENU_AXIS_LEVELING_COMMANDS)); break;
    #endif
    case 4:  GOTO_SCREEN(MoveAxisScreen);                             break;
    case 5:  enqueueCommands(F("M84"));                               break;
    case 6:  GOTO_SCREEN(TemperatureScreen);                          break;
    case 8:  GOTO_SCREEN(AdvancedSettingsScreen);                     break;
    case 9:  GOTO_SCREEN(AboutScreen);                                break;
    default:
      return false;
  }
  return true;
}

/************************************ TUNE SCREEN *******************************/

void TuneScreen::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true))
       .font(Theme::font_medium)

    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_ROWS 5
      #define GRID_COLS 2
       .tag(2).enabled(1)      .button( BTN_POS(1,1), BTN_SIZE(2,1), F("Temperature"))
       .tag(3).enabled(0)      .button( BTN_POS(1,2), BTN_SIZE(2,1), F("Change Filament"))
       .tag(4)
      #if HAS_BED_PROBE
        .enabled(1)
      #else
        .enabled(0)
      #endif
                               .button( BTN_POS(1,3), BTN_SIZE(2,1), F("Z Offset"))
       .tag(5).enabled(1)      .button( BTN_POS(1,4), BTN_SIZE(2,1), F("Print Speed"))
       .tag(1).THEME(back_btn) .button( BTN_POS(1,5), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #else
      #define GRID_ROWS 3
      #define GRID_COLS 2
       .tag(2).enabled(1)      .button( BTN_POS(1,1), BTN_SIZE(1,1), F("Temperature"))
       .tag(3).enabled(0)      .button( BTN_POS(1,2), BTN_SIZE(1,1), F("Change Filament"))
       .tag(4)
      #if HAS_BED_PROBE
       .enabled(1)
      #else
       .enabled(0)
      #endif
                               .button( BTN_POS(2,1), BTN_SIZE(1,1), F("Z Offset"))
       .tag(5).enabled(1)      .button( BTN_POS(2,2), BTN_SIZE(1,1), F("Print Speed"))
       .tag(1).THEME(back_btn) .button( BTN_POS(1,3), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #endif
  }
}

bool TuneScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:  GOTO_PREVIOUS();                    break;
    case 2:  GOTO_SCREEN(TemperatureScreen);     break;
    #if HAS_BED_PROBE
    case 4:  GOTO_SCREEN(ZOffsetScreen);         break;
    #endif
    case 5:  GOTO_SCREEN(FeedrateScreen);        break;
    default:
      return false;
  }
  return true;
}

/******************************* CONFIGURATION SCREEN ****************************/

void AdvancedSettingsScreen::onRedraw(draw_mode_t what) {
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true));
  }

  if(what & FOREGROUND) {
    CommandProcessor cmd;
    default_button_colors();
    cmd.font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_ROWS 7
      #define GRID_COLS 2
      #if HAS_BED_PROBE
        .enabled(1)
      #else
        .enabled(0)
      #endif
      .tag(4) .button( BTN_POS(1,1), BTN_SIZE(1,2), F("Z Offset "))
      .enabled(1)
      .tag(5) .button( BTN_POS(1,3), BTN_SIZE(1,1), F("Steps/mm"))
      .tag(6) .button( BTN_POS(2,1), BTN_SIZE(1,1), F("Velocity "))
      .tag(7) .button( BTN_POS(2,2), BTN_SIZE(1,1), F("Acceleration"))
      .tag(8) .button( BTN_POS(2,3), BTN_SIZE(1,1), F("Jerk"))
      .tag(9) .button( BTN_POS(1,4), BTN_SIZE(2,1), F("Recalibrate Screen"))
      .tag(10).button( BTN_POS(1,5), BTN_SIZE(2,1), F("Restore Factory Settings"))
      .tag(2) .button( BTN_POS(1,6), BTN_SIZE(2,1), F("Save As Default"))
      .THEME(back_btn)
      .tag(1) .button( BTN_POS(1,7), BTN_SIZE(2,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #else
      #define GRID_ROWS 4
      #define GRID_COLS 2
      #if HAS_BED_PROBE
        .enabled(1)
      #else
        .enabled(0)
      #endif
      .tag(4) .button( BTN_POS(1,1), BTN_SIZE(1,1), F("Z Offset "))
      .enabled(1)
      .tag(5) .button( BTN_POS(1,2), BTN_SIZE(1,1), F("Steps/mm"))
      .tag(6) .button( BTN_POS(2,1), BTN_SIZE(1,1), F("Velocity "))
      .tag(7) .button( BTN_POS(2,2), BTN_SIZE(1,1), F("Acceleration"))
      .tag(8) .button( BTN_POS(2,3), BTN_SIZE(1,1), F("Jerk"))
      .tag(10).button( BTN_POS(1,3), BTN_SIZE(1,1), F("Restore Failsafe"))
      .tag(2) .button( BTN_POS(1,4), BTN_SIZE(1,1), F("Save"))
      .THEME(back_btn)
      .tag(1) .button( BTN_POS(2,4), BTN_SIZE(1,1), F("Back"));
      #undef GRID_COLS
      #undef GRID_ROWS
    #endif
  }
}

bool AdvancedSettingsScreen::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 1:  GOTO_PREVIOUS();                    break;
    case 2:
      enqueueCommands(F("M500"));
      AlertBoxScreen::show(F("Settings saved!"));
      break;
    #if HAS_BED_PROBE
    case 4:  GOTO_SCREEN(ZOffsetScreen);         break;
    #endif
    case 5:  GOTO_SCREEN(StepsScreen);           break;
    case 6:  GOTO_SCREEN(VelocityScreen);        break;
    case 7:  GOTO_SCREEN(AccelerationScreen);    break;
    case 8:  GOTO_SCREEN(JerkScreen);            break;
    case 9:  GOTO_SCREEN(CalibrationScreen);     break;
    case 10: GOTO_SCREEN(RestoreFailsafeScreen); break;
    default:
      return false;
  }
  return true;
}

/******************************** CALIBRATION SCREEN ****************************/

void CalibrationScreen::onEntry() {
  // Clear the display
  CommandProcessor cmd;
  cmd.cmd(CMD_DLSTART)
     .cmd(CLEAR_COLOR_RGB(Theme::background))
     .cmd(CLEAR(true,true,true))
     .cmd(DL::DL_DISPLAY)
     .cmd(CMD_SWAP)
     .execute();

  // Wait for the touch to release before starting,
  // as otherwise the first calibration point could
  // be misinterpreted.
  while(CLCD::is_touching()) {
    #if defined(UI_FRAMEWORK_DEBUG)
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("Waiting for touch release");
      #endif
    #endif
  }
  UIScreen::onEntry();
}

void CalibrationScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
     .cmd(CLEAR(true,true,true))
  #define GRID_COLS 4
  #define GRID_ROWS 16
  #if defined(USE_PORTRAIT_ORIENTATION)
    .font(Theme::font_large)
    .text  ( BTN_POS(1,8), BTN_SIZE(4,1), F("Touch the dots"))
    .text  ( BTN_POS(1,9), BTN_SIZE(4,1), F("to calibrate"))
  #else
    .font(
      #if defined(LCD_800x480)
        Theme::font_large
      #else
        Theme::font_medium
      #endif
    )
    .text  ( BTN_POS(1,1), BTN_SIZE(4,16), F("Touch the dots to calibrate"))
  #endif
  #undef GRID_COLS
  #undef GRID_ROWS
    .cmd(CMD_CALIBRATE);
}

void CalibrationScreen::onIdle() {
  if(!CommandProcessor::is_processing()) {
    #if defined(UI_FRAMEWORK_DEBUG)
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("Calibration finished");
      #endif
    #endif
    GOTO_SCREEN(StatusScreen);
  }
  #if defined(UI_FRAMEWORK_DEBUG)
    else {
      #if defined (SERIAL_PROTOCOLLNPGM)
        SERIAL_PROTOCOLLNPGM("Waiting for calibration to finish.");
      #endif
    }
  #endif
}

/*************************** GENERIC VALUE ADJUSTMENT SCREEN ******************************/

#if defined(USE_PORTRAIT_ORIENTATION)
  #define GRID_COLS 13
  #define GRID_ROWS 10
#else
  #define GRID_COLS 18
  #define GRID_ROWS  6
#endif

ValueAdjusters::widgets_t::widgets_t(draw_mode_t what) : _what(what) {
  if(what & BACKGROUND) {
    CommandProcessor cmd;
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true));
  }

  if(what & FOREGROUND) {
    CommandProcessor cmd;
    cmd.font(Theme::font_medium)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .THEME(back_btn).tag(1).button( BTN_POS(1,10), BTN_SIZE(13,1), F("Back"));
    #else
       .THEME(back_btn).tag(1).button( BTN_POS(15,6), BTN_SIZE(4,1), F("Back"));
    #endif
  }

  _line = 1;
}

void ValueAdjusters::widgets_t::heading(const char *label) {
  CommandProcessor cmd;
  cmd.font(Theme::font_medium);
  if(_what & BACKGROUND) {
    #if defined(USE_PORTRAIT_ORIENTATION)
      cmd.tag(0).THEME(background).button( BTN_POS(1, _line), BTN_SIZE(12,1), (progmem_str) label, OPT_FLAT);
    #else
      cmd.tag(0).THEME(background).button( BTN_POS(5, _line), BTN_SIZE(8,1),  (progmem_str) label, OPT_FLAT);
    #endif
  }

  _line++;
}

#if defined(USE_PORTRAIT_ORIENTATION)
  #if defined(LCD_800x480)
    #undef EDGE_R
    #define EDGE_R 20
  #else
    #undef EDGE_R
    #define EDGE_R 10
  #endif
#endif

void ValueAdjusters::widgets_t::_draw_increment_btn(uint8_t line, const uint8_t tag) {
  CommandProcessor  cmd;
  const char        *label = PSTR("?");
  uint8_t            pos;

  if(screen_data.ValueAdjusters.increment == 0) {
    screen_data.ValueAdjusters.increment = tag; // Set the default value to be the first.
  }

  if(screen_data.ValueAdjusters.increment == tag) {
    cmd.THEME(toggle_on);
  } else {
    cmd.THEME(toggle_off);
  }

  switch(tag) {
    case 240: label = PSTR(   ".001"); pos = _decimals - 3; break;
    case 241: label = PSTR(   ".01" ); pos = _decimals - 2; break;
    case 242: label = PSTR(  "0.1"  ); pos = _decimals - 1; break;
    case 243: label = PSTR(  "1"    ); pos = _decimals + 0; break;
    case 244: label = PSTR( "10"    ); pos = _decimals + 1; break;
    case 245: label = PSTR("100"    ); pos = _decimals + 2; break;
    default:
      #if defined(UI_FRAMEWORK_DEBUG)
        #if defined(SERIAL_PROTOCOLLNPAIR)
        SERIAL_PROTOCOLLNPAIR("Unknown tag for increment btn: ", tag);
        #else
        Serial.print(F("Unknown tag for increment btn:"));
        Serial.println(tag);
        #endif
      #endif
      ;
  }

  cmd.tag(tag)
  #if defined(USE_PORTRAIT_ORIENTATION)
    .font(Theme::font_small);
  #else
    .font(Theme::font_medium);
  #endif
  switch(pos) {
    #if defined(USE_PORTRAIT_ORIENTATION)
      case 0: cmd.button( BTN_POS(5,_line), BTN_SIZE(2,1), progmem_str(label)); break;
      case 1: cmd.button( BTN_POS(7,_line), BTN_SIZE(2,1), progmem_str(label)); break;
      case 2: cmd.button( BTN_POS(9,_line), BTN_SIZE(2,1), progmem_str(label)); break;
    #else
      case 0: cmd.button( BTN_POS(15,2), BTN_SIZE(4,1), progmem_str(label)); break;
      case 1: cmd.button( BTN_POS(15,3), BTN_SIZE(4,1), progmem_str(label)); break;
      case 2: cmd.button( BTN_POS(15,4), BTN_SIZE(4,1), progmem_str(label)); break;
    #endif
  }
}

void ValueAdjusters::widgets_t::increments() {
  CommandProcessor cmd;

  if(_what & BACKGROUND) {
    cmd.THEME(background)
       .tag(0)
    #if defined(USE_PORTRAIT_ORIENTATION)
       .font(Theme::font_small).button( BTN_POS(1, _line),  BTN_SIZE(4,1), F("Increment:"), OPT_FLAT);
    #else
       .font(Theme::font_medium).button( BTN_POS(15,1),     BTN_SIZE(4,1), F("Increment:"), OPT_FLAT);
    #endif
  }

  if(_what & FOREGROUND) {
      _draw_increment_btn(_line+1, 244 - _decimals);
      _draw_increment_btn(_line+1, 243 - _decimals);
      _draw_increment_btn(_line+1, 245 - _decimals);
  }

  _line += 2;
}

void ValueAdjusters::widgets_t::adjuster(uint8_t tag, const char *label,float value) {
  CommandProcessor cmd;

  if(_what & BACKGROUND) {
    progmem_str   str  = (progmem_str) label;
    cmd.enabled(1)
       .font(Theme::font_small)
       .fgcolor(_color)  .tag(0    ).button( BTN_POS(5,_line),  BTN_SIZE(5,1),  F(""),  OPT_FLAT)
       .THEME(background).tag(0    ).button( BTN_POS(1,_line),  BTN_SIZE(4,1),  str,    OPT_FLAT);
  }

  if(_what & FOREGROUND) {
    char b[32];

    default_button_colors();
    cmd.enabled(1)
       .font(Theme::font_medium)
       .tag(tag  ).button( BTN_POS(10,_line), BTN_SIZE(2,1),  F("-"), OPT_3D)
       .tag(tag+1).button( BTN_POS(12,_line), BTN_SIZE(2,1),  F("+"), OPT_3D);

    dtostrf(value, 5, _decimals, b);
    strcat_P(b, PSTR(" "));
    strcat_P(b, (const char*) _units);

    cmd.tag(0).font(Theme::font_small).text ( BTN_POS(5,_line), BTN_SIZE(5,1), b);
  }

  _line++;
}

#undef EDGE_R
#define EDGE_R 0

#undef GRID_COLS
#undef GRID_ROWS

void ValueAdjusters::onEntry() {
  screen_data.ValueAdjusters.increment = 0; // This will force the increment to be picked while drawing.
  UIScreen::onEntry();
}

bool ValueAdjusters::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:           GOTO_PREVIOUS();                             return true;
    case 240 ... 245: screen_data.ValueAdjusters.increment = tag;  break;
    default:          return current_screen.onTouchHeld(tag);
  }
  return true;
}

float ValueAdjusters::getIncrement() {
  switch(screen_data.ValueAdjusters.increment) {
    case 240: return   0.001;
    case 241: return   0.01;
    case 242: return   0.1;
    case 243: return   1.0;
    case 244: return  10.0;
    case 245: return 100.0;
    default:  return   0.0;
  }
}

/******************************** MOVE AXIS SCREEN ******************************/

void MoveAxisScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(1);
  w.units(PSTR("mm"));

  w.heading(                             PSTR("Move Axis"));
  w.color(Theme::x_axis  ).adjuster(  2, PSTR("X:"),  getAxisPosition_mm(X));
  w.color(Theme::y_axis  ).adjuster(  4, PSTR("Y:"),  getAxisPosition_mm(Y));
  w.color(Theme::z_axis  ).adjuster(  6, PSTR("Z:"),  getAxisPosition_mm(Z));
  #if EXTRUDERS == 1
    w.color(Theme::e_axis).adjuster(  8, PSTR("E0:"), getAxisPosition_mm(E0));
  #else
    w.color(Theme::e_axis).adjuster(  8, PSTR("E0:"), getAxisPosition_mm(E0));
    w.color(Theme::e_axis).adjuster( 10, PSTR("E1:"), getAxisPosition_mm(E1));
  #endif
  w.increments();
}

bool MoveAxisScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  // We don't want to stack up moves, so wait until the
  // machine is idle before sending another.
  if(isMoving()) {
    return false;
  }

  float inc = getIncrement();
  axis_t axis;
  const float feedrate_mm_s = inc * TOUCH_REPEATS_PER_SECOND;

  switch(tag) {
    case  2: axis = X;  inc *= -1;  break;
    case  3: axis = X;  inc *=  1;  break;
    case  4: axis = Y;  inc *= -1;  break;
    case  5: axis = Y;  inc *=  1;  break;
    case  6: axis = Z;  inc *= -1;  break;
    case  7: axis = Z;  inc *=  1;  break;
    case  8: axis = E0; inc *= -1;  break;
    case  9: axis = E0; inc *=  1;  break;
    case 10: axis = E1; inc *= -1;  break;
    case 11: axis = E1; inc *=  1;  break;
    default:
      return false;
  }
  setAxisPosition_mm(axis, getAxisPosition_mm(axis) + inc, feedrate_mm_s);
  onRefresh();
  return true;
}

/******************************* TEMPERATURE SCREEN ******************************/

void TemperatureScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);

  w.precision(0).color(Theme::temp).units(PSTR("C"));

  w.heading(         PSTR("Temperature:"));
  #if EXTRUDERS == 1
    w.adjuster(   2, PSTR("Nozzle:"),       getTargetTemp_celsius(1));
  #else
    w.adjuster(   2, PSTR("Nozzle 1:"),     getTargetTemp_celsius(1));
    w.adjuster(   4, PSTR("Nozzle 2:"),     getTargetTemp_celsius(2));
  #endif
  w.adjuster(    20, PSTR("Bed:"),          getTargetTemp_celsius(0));

  w.color(Theme::fan_speed).units(PSTR("%")).adjuster(10, PSTR("Fan Speed:"), getFan_percent(0));
  w.increments();
}

bool TemperatureScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 20: setTargetTemp_celsius(0, getTargetTemp_celsius(0) - getIncrement()); break;
    case 21: setTargetTemp_celsius(0, getTargetTemp_celsius(0) + getIncrement()); break;
    case  2: setTargetTemp_celsius(1, getTargetTemp_celsius(1) - getIncrement()); break;
    case  3: setTargetTemp_celsius(1, getTargetTemp_celsius(1) + getIncrement()); break;
    case  4: setTargetTemp_celsius(2, getTargetTemp_celsius(2) - getIncrement()); break;
    case  5: setTargetTemp_celsius(2, getTargetTemp_celsius(2) + getIncrement()); break;
    case 10: setFan_percent(       0, getFan_percent(0)        - getIncrement()); break;
    case 11: setFan_percent(       0, getFan_percent(0)        + getIncrement()); break;
    default:
      return false;
  }
  onRefresh();
  return true;
}

/******************************* STEPS SCREEN ******************************/

void StepsScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(0);
  w.units(PSTR("st/mm"));

  w.heading(                            PSTR("Steps/mm"));
  w.color(Theme::x_axis).adjuster(   2, PSTR("X:"),  getAxisSteps_per_mm(X) );
  w.color(Theme::y_axis).adjuster(   4, PSTR("Y:"),  getAxisSteps_per_mm(Y) );
  w.color(Theme::z_axis).adjuster(   6, PSTR("Z:"),  getAxisSteps_per_mm(Z) );
  #if EXTRUDERS == 1
    w.color(Theme::e_axis).adjuster( 8, PSTR("E:"),  getAxisSteps_per_mm(E0) );
  #else
    w.color(Theme::e_axis).adjuster( 8, PSTR("E0:"), getAxisSteps_per_mm(E0) );
    w.color(Theme::e_axis).adjuster(10, PSTR("E1:"), getAxisSteps_per_mm(E1) );
  #endif
  w.increments();
}

bool StepsScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  float inc = getIncrement();
  axis_t axis;

  switch(tag) {
    case  2:  axis = X;  inc *= -1;  break;
    case  3:  axis = X;  inc *=  1;  break;
    case  4:  axis = Y;  inc *= -1;  break;
    case  5:  axis = Y;  inc *=  1;  break;
    case  6:  axis = Z;  inc *= -1;  break;
    case  7:  axis = Z;  inc *=  1;  break;
    case  8:  axis = E0; inc *= -1;  break;
    case  9:  axis = E0; inc *=  1;  break;
    #if EXTRUDERS == 2
    case 10:  axis = E1; inc *= -1;  break;
    case 11:  axis = E1; inc *=  1;  break;
    #endif
    default:
      return false;
  }

  setAxisSteps_per_mm(axis, getAxisSteps_per_mm(axis) + inc);
  onRefresh();
  return true;
}

/***************************** Z-OFFSET SCREEN ***************************/

#if HAS_BED_PROBE
  void ZOffsetScreen::onRedraw(draw_mode_t what) {
    using namespace Extensible_UI_API;

    widgets_t w(what);
    w.precision(3).units(PSTR("mm"));

    w.heading(                          PSTR("Z Offset"));
    w.color(Theme::z_axis).adjuster(4,  PSTR("Z Offset:"), getZOffset_mm());
    w.increments();
  }

  bool ZOffsetScreen::onTouchHeld(uint8_t tag) {
    using namespace Extensible_UI_API;

    switch(tag) {
      case 4:  incrementZOffset_mm(-getIncrement()); break;
      case 5:  incrementZOffset_mm( getIncrement()); break;
      default:
        return false;
    }
    onRefresh();
    return true;
  }
#endif // HAS_BED_PROBE

/***************************** FEEDRATE SCREEN ***************************/

void FeedrateScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(0).units(PSTR("%"));

  w.heading(PSTR("Print Speed"));
  w.adjuster(4,  PSTR("Speed"), getFeedRate_percent());
  w.increments();
}

bool FeedrateScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  float inc = getIncrement();
  switch(tag) {
    case 4:  setFeedrate_percent(getFeedRate_percent() - inc); break;
    case 5:  setFeedrate_percent(getFeedRate_percent() + inc); break;
    default:
      return false;
  }
  onRefresh();
  return true;
}

/******************************* VELOCITY SCREEN ******************************/

void VelocityScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(0);
  w.units(PSTR("mm/s"));

  w.heading(                            PSTR("Velocity"));
  w.color(Theme::x_axis).adjuster(   2, PSTR("X:"),  getAxisMaxFeedrate_mm_s(X) );
  w.color(Theme::y_axis).adjuster(   4, PSTR("Y:"),  getAxisMaxFeedrate_mm_s(Y) );
  w.color(Theme::z_axis).adjuster(   6, PSTR("Z:"),  getAxisMaxFeedrate_mm_s(Z) );
  #if EXTRUDERS == 1
    w.color(Theme::e_axis).adjuster( 8, PSTR("E:"),  getAxisMaxFeedrate_mm_s(E0) );
  #else
    w.color(Theme::e_axis).adjuster( 8, PSTR("E0:"), getAxisMaxFeedrate_mm_s(E0) );
    w.color(Theme::e_axis).adjuster(10, PSTR("E1:"), getAxisMaxFeedrate_mm_s(E1) );
  #endif
  w.increments();
}

bool VelocityScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  float inc = getIncrement();
  axis_t axis;

  switch(tag) {
    case  2:  axis = X;  inc *= -1;  break;
    case  3:  axis = X;  inc *=  1;  break;
    case  4:  axis = Y;  inc *= -1;  break;
    case  5:  axis = Y;  inc *=  1;  break;
    case  6:  axis = Z;  inc *= -1;  break;
    case  7:  axis = Z;  inc *=  1;  break;
    case  8:  axis = E0; inc *= -1;  break;
    case  9:  axis = E0; inc *=  1;  break;
    #if EXTRUDERS == 2
    case 10:  axis = E1; inc *= -1;  break;
    case 11:  axis = E1; inc *=  1;  break;
    #endif
    default:
      return false;
  }

  setAxisMaxFeedrate_mm_s(axis, getAxisMaxFeedrate_mm_s(axis) + inc);
  onRefresh();
  return true;
}

/******************************* ACCELERATION SCREEN ******************************/

void AccelerationScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(0);
  w.units(PSTR("mm/s^2"));

  w.heading(                            PSTR("Max Acceleration"));
  w.color(Theme::x_axis).adjuster(   2, PSTR("X:"),  getAxisMaxAcceleration_mm_s2(X) );
  w.color(Theme::y_axis).adjuster(   4, PSTR("Y:"),  getAxisMaxAcceleration_mm_s2(Y) );
  w.color(Theme::z_axis).adjuster(   6, PSTR("Z:"),  getAxisMaxAcceleration_mm_s2(Z) );
  #if EXTRUDERS == 1
    w.color(Theme::e_axis).adjuster( 8, PSTR("E:"),  getAxisMaxAcceleration_mm_s2(E0) );
  #else
    w.color(Theme::e_axis).adjuster( 8, PSTR("E0:"), getAxisMaxAcceleration_mm_s2(E0) );
    w.color(Theme::e_axis).adjuster(10, PSTR("E1:"), getAxisMaxAcceleration_mm_s2(E1) );
  #endif
  w.increments();
}

bool AccelerationScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  float inc = getIncrement();
  axis_t axis;

  switch(tag) {
    case  2:  axis = X;  inc *= -1;  break;
    case  3:  axis = X;  inc *=  1;  break;
    case  4:  axis = Y;  inc *= -1;  break;
    case  5:  axis = Y;  inc *=  1;  break;
    case  6:  axis = Z;  inc *= -1;  break;
    case  7:  axis = Z;  inc *=  1;  break;
    case  8:  axis = E0; inc *= -1;  break;
    case  9:  axis = E0; inc *=  1;  break;
    #if EXTRUDERS == 2
    case 10:  axis = E1; inc *= -1;  break;
    case 11:  axis = E1; inc *=  1;  break;
    #endif
    default:
      return false;
  }

  setAxisMaxAcceleration_mm_s2(axis, getAxisMaxAcceleration_mm_s2(axis) + inc);
  onRefresh();
  return true;
}

/************************************** JERK SCREEN ************************************/

void JerkScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  widgets_t w(what);
  w.precision(1);
  w.units(PSTR("mm/s"));

  w.heading(                          PSTR("Max Jerk"));
  w.color(Theme::x_axis).adjuster( 2, PSTR("X:"),  getAxisMaxJerk_mm_s(X) );
  w.color(Theme::y_axis).adjuster( 4, PSTR("Y:"),  getAxisMaxJerk_mm_s(Y) );
  w.color(Theme::z_axis).adjuster( 6, PSTR("Z:"),  getAxisMaxJerk_mm_s(Z) );
  w.color(Theme::e_axis).adjuster( 8, PSTR("E:"),  getAxisMaxJerk_mm_s(E0) );
  w.increments();
}

bool JerkScreen::onTouchHeld(uint8_t tag) {
  using namespace Extensible_UI_API;

  float inc = getIncrement();
  axis_t axis;

  switch(tag) {
    case  2:  axis = X;  inc *= -1;  break;
    case  3:  axis = X;  inc *=  1;  break;
    case  4:  axis = Y;  inc *= -1;  break;
    case  5:  axis = Y;  inc *=  1;  break;
    case  6:  axis = Z;  inc *= -1;  break;
    case  7:  axis = Z;  inc *=  1;  break;
    case  8:  axis = E0; inc *= -1;  break;
    case  9:  axis = E0; inc *=  1;  break;
    default:
      return false;
  }

  setAxisMaxJerk_mm_s(axis, getAxisMaxJerk_mm_s(axis) + inc);
  onRefresh();
  return true;
}

/***************************** FILES SCREEN ***************************/

void FilesScreen::onEntry() {
  screen_data.FilesScreen.page            = 0;
  screen_data.FilesScreen.selected_tag    = 0xFF;
  UIScreen::onEntry();
}

const char *FilesScreen::getSelectedShortFilename() {
  using namespace Extensible_UI_API;

  Media_Iterator iterator(getIndexForTag(screen_data.FilesScreen.selected_tag));
  return iterator.shortFilename();
}

uint8_t FilesScreen::getTagForIndex(uint16_t fileIndex) {
  return fileIndex + 1;
}

uint16_t FilesScreen::getIndexForTag(uint8_t tag) {
  return tag - 1;
}

void FilesScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;

  CommandProcessor cmd;

  if(what & BACKGROUND) {
    cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
       .cmd(CLEAR(true,true,true));
  }

  if(what & FOREGROUND) {
    const uint8_t header_h = 1;
    const uint8_t footer_h = 2;

    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_COLS  6
      #define GRID_ROWS 14
    #else
      #define GRID_COLS  6
      #define GRID_ROWS  9
    #endif

    // Make sure the page value is in range
    const uint16_t filesPerPage = GRID_ROWS - header_h - footer_h;
    const uint16_t pageCount = max(1,(ceil)(float(getFileCount()) / filesPerPage));
    screen_data.FilesScreen.page = min(screen_data.FilesScreen.page, pageCount-1);

    Media_Iterator iterator(screen_data.FilesScreen.page * filesPerPage);
    bool dirSelected = false;

    #undef MARGIN_T
    #undef MARGIN_B
    #define MARGIN_T 0
    #define MARGIN_B 0

    // Make buttons for each file.
    if(iterator.count()) {
      uint8_t line = 1;
      do {
        const uint16_t tag = getTagForIndex(iterator.value());
        const bool isDir   = iterator.isDirectory();

        cmd.tag(tag);
        if(screen_data.FilesScreen.selected_tag == tag) {
          cmd.THEME(files_selected);
          dirSelected = isDir;
        } else {
          cmd.THEME(background);
        }
        cmd.font(Theme::font_medium)
           .button( BTN_POS(1,header_h+line), BTN_SIZE(6,1), F(""),               OPT_FLAT)
           .text  ( BTN_POS(1,header_h+line), BTN_SIZE(6,1), iterator.filename(), OPT_CENTERY);
        if(isDir) {
          cmd.text( BTN_POS(1,header_h+line), BTN_SIZE(6,1), F("> "),             OPT_CENTERY | OPT_RIGHTX);
        }
      } while(iterator.next() && line++ < filesPerPage);
    }

    const bool prevEnabled   = screen_data.FilesScreen.page > 0;
    const bool nextEnabled   = screen_data.FilesScreen.page < (pageCount - 1);
    const bool itemSelected  = screen_data.FilesScreen.selected_tag != 0xFF;
    const uint8_t backTag    = isAtRootDir() ? 240 : 245;

    #undef MARGIN_T
    #undef MARGIN_B
    #define MARGIN_T 0
    #define MARGIN_B 2

    char page_str[15];
    sprintf_P(page_str, PSTR("Page %d of %d"), screen_data.FilesScreen.page + 1, pageCount);

    cmd.font(Theme::font_small)
       .tag(0).text( BTN_POS(1,1), BTN_SIZE(4,1), page_str, OPT_CENTER);

    cmd.font(Theme::font_medium)
       .tag(241).BTN_EN_THEME(prevEnabled, light).button( BTN_POS(5,1),  BTN_SIZE(1,header_h), F("<"))
       .tag(242).BTN_EN_THEME(nextEnabled, light).button( BTN_POS(6,1),  BTN_SIZE(1,header_h), F(">"));

    #undef MARGIN_T
    #undef MARGIN_B
    #define MARGIN_T 15
    #define MARGIN_B 5
    const uint8_t y = GRID_ROWS - footer_h + 1;
    const uint8_t h = footer_h;
    cmd.enabled(true)
       .THEME(back_btn).tag(backTag).button( BTN_POS(1,y), BTN_SIZE(3,h), F("Back"));

    cmd.enabled(itemSelected);
    if(dirSelected) {
      cmd.tag(244).button( BTN_POS(4, y), BTN_SIZE(3,h), F("Open"));
    } else {
      cmd.tag(243).button( BTN_POS(4, y), BTN_SIZE(3,h), F("Print"));
    }

    #undef MARGIN_T
    #undef MARGIN_B
    #define MARGIN_T 5
    #define MARGIN_B 5

    #undef GRID_COLS
    #undef GRID_ROWS
  }
}

bool FilesScreen::onTouchEnd(uint8_t tag) {
  using namespace Extensible_UI_API;

  switch(tag) {
    case 240: GOTO_PREVIOUS();                  return true;
    case 241:
      if(screen_data.FilesScreen.page > 0)
        screen_data.FilesScreen.page--;
      break;
    case 242:
      // Positive bounds checking will be done in the refresh routine.
      screen_data.FilesScreen.page++;
      break;
    case 243:
      printFile(getSelectedShortFilename());
      StatusScreen::setStatusMessage(PSTR("Print Starting"));
      GOTO_SCREEN(StatusScreen);
      sound.play(start_print, PLAY_SYNCHRONOUS);
      return true;
    case 244:
      changeDir(getSelectedShortFilename());
      break;
    case 245:
      upDir();
      break;
    default:
      if(tag < 240) {
        if(screen_data.FilesScreen.selected_tag != tag) {
          screen_data.FilesScreen.selected_tag = tag;
        } else {
          // Double clicked.
        }
      }
      break;
  }
  onRefresh();
  return true;
}

/***************************** WIDGET DEMO SCREEN ***************************/

uint16_t dial_val;
uint16_t slider_val;
bool     show_grid;

void WidgetsScreen::onEntry() {
  UIScreen::onEntry();
  CLCD::turn_on_backlight();
  FTDI::SoundPlayer::set_volume(255);
}

void WidgetsScreen::onRedraw(draw_mode_t what) {
  using namespace Extensible_UI_API;
  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
     .cmd(CLEAR(true,true,true));

  cmd.bgcolor(Theme::theme_darkest)
     .THEME(theme_light);

  #define GRID_COLS 4
  #define GRID_ROWS 8

  cmd.font(Theme::font_large)
            .text      (BTN_POS(1,1),  BTN_SIZE(4,1), F("Sample Widgets"))
     .tag(1).dial      (BTN_POS(1,2),  BTN_SIZE(1,3), dial_val)
     .tag(2).dial      (BTN_POS(1,5),  BTN_SIZE(1,3), slider_val)
     .tag(0).clock     (BTN_POS(2,2),  BTN_SIZE(1,3), 3, 30, 45, 0)
            .gauge     (BTN_POS(2,5),  BTN_SIZE(1,3), 5, 4, slider_val,  0xFFFFU)

     .font(Theme::font_medium)
     .tag(3).slider    (BTN_POS(3,3),  BTN_SIZE(2,1), slider_val,        0xFFFFU)
     .tag(0).progress  (BTN_POS(3,4),  BTN_SIZE(2,1), slider_val,        0xFFFFU)
     .tag(4).scrollbar (BTN_POS(3,5),  BTN_SIZE(2,1), slider_val, 10000, 0xFFFFU)

     .font(Theme::font_small)
     .tag(0).text      (BTN_POS(3,6),  BTN_SIZE(1,1), F("Show grid:"))
     .tag(5).toggle    (BTN_POS(4,6),  BTN_SIZE(1,1), F("no\xFFyes"), show_grid)

     .font(Theme::font_medium)
     .tag(6).button    (BTN_POS(1, 8), BTN_SIZE(1,1), F("1"))
            .button    (BTN_POS(2, 8), BTN_SIZE(1,1), F("2"))
            .button    (BTN_POS(3, 8), BTN_SIZE(1,1), F("3"))
            .button    (BTN_POS(4, 8), BTN_SIZE(1,1), F("4"));

  if(show_grid) DRAW_LAYOUT_GRID
}

bool WidgetsScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1: start_tracking (BTN_POS(1,2), BTN_SIZE(1,3), 1, true);  break;
    case 2: start_tracking (BTN_POS(1,5), BTN_SIZE(1,3), 2, true);  break;
    case 3: start_tracking (BTN_POS(3,3), BTN_SIZE(2,1), 3, false); break;
    case 4: start_tracking (BTN_POS(3,3), BTN_SIZE(2,1), 4, false); break;
    case 5: show_grid = !show_grid; break;
    default: break;
  }

  #undef GRID_COLS
  #undef GRID_ROWS

  return false;
}

void WidgetsScreen::onIdle() {
  uint16_t value;
  switch(CLCD::get_tracker(value)) {
    case 1:
      dial_val   = value; break;
    case 2:
    case 3:
    case 4:
      slider_val = value; break;
    case 6:
      break;
    default:
      return;
  }
  onRefresh();
}

/*************** DEVELOPER MENU: CALIBRATION REGISTERS SCREEN **********************/

void CalibrationRegistersScreen::onRedraw(draw_mode_t what) {
  const uint32_t T_Transform_A = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_A);
  const uint32_t T_Transform_B = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_B);
  const uint32_t T_Transform_C = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_C);
  const uint32_t T_Transform_D = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_D);
  const uint32_t T_Transform_E = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_E);
  const uint32_t T_Transform_F = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_F);
  char b[20];

  CommandProcessor cmd;
  cmd.cmd(CLEAR_COLOR_RGB(Theme::background))
     .cmd(CLEAR(true,true,true));

  #define GRID_ROWS 7
  #define GRID_COLS 2
  cmd.tag(0)
     .font(28)
     .THEME(transformA).button( BTN_POS(1,1), BTN_SIZE(1,1), F("TOUCH TRANSFORM_A"), OPT_3D)
     .THEME(transformB).button( BTN_POS(1,2), BTN_SIZE(1,1), F("TOUCH TRANSFORM_B"), OPT_3D)
     .THEME(transformC).button( BTN_POS(1,3), BTN_SIZE(1,1), F("TOUCH TRANSFORM_C"), OPT_3D)
     .THEME(transformD).button( BTN_POS(1,4), BTN_SIZE(1,1), F("TOUCH TRANSFORM_D"), OPT_3D)
     .THEME(transformE).button( BTN_POS(1,5), BTN_SIZE(1,1), F("TOUCH TRANSFORM_E"), OPT_3D)
     .THEME(transformF).button( BTN_POS(1,6), BTN_SIZE(1,1), F("TOUCH TRANSFORM_F"), OPT_3D)

     .THEME(transformVal).button( BTN_POS(2,1), BTN_SIZE(1,1), F(""), OPT_FLAT)
     .THEME(transformVal).button( BTN_POS(2,2), BTN_SIZE(1,1), F(""), OPT_FLAT)
     .THEME(transformVal).button( BTN_POS(2,3), BTN_SIZE(1,1), F(""), OPT_FLAT)
     .THEME(transformVal).button( BTN_POS(2,4), BTN_SIZE(1,1), F(""), OPT_FLAT)
     .THEME(transformVal).button( BTN_POS(2,5), BTN_SIZE(1,1), F(""), OPT_FLAT)
     .THEME(transformVal).button( BTN_POS(2,6), BTN_SIZE(1,1), F(""), OPT_FLAT);

  sprintf_P(b, PSTR("0x%08lX"), T_Transform_A); cmd.font(28).text  ( BTN_POS(2,1), BTN_SIZE(1,1), b);
  sprintf_P(b, PSTR("0x%08lX"), T_Transform_B); cmd.font(28).text  ( BTN_POS(2,2), BTN_SIZE(1,1), b);
  sprintf_P(b, PSTR("0x%08lX"), T_Transform_C); cmd.font(28).text  ( BTN_POS(2,3), BTN_SIZE(1,1), b);
  sprintf_P(b, PSTR("0x%08lX"), T_Transform_D); cmd.font(28).text  ( BTN_POS(2,4), BTN_SIZE(1,1), b);
  sprintf_P(b, PSTR("0x%08lX"), T_Transform_E); cmd.font(28).text  ( BTN_POS(2,5), BTN_SIZE(1,1), b);
  sprintf_P(b, PSTR("0x%08lX"), T_Transform_F); cmd.font(28).text  ( BTN_POS(2,6), BTN_SIZE(1,1), b);

  cmd.THEME(back_btn).tag(1).font(Theme::font_medium).button( BTN_POS(2,7), BTN_SIZE(1,1), F("Back"));
  #undef GRID_COLS
  #undef GRID_ROWS

  sound.play(js_bach_joy, PLAY_ASYNCHRONOUS);
}

bool CalibrationRegistersScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1:        GOTO_PREVIOUS();                 break;
    default:
      return false;
  }
  return true;
}

/***************************** MEDIA DEMO SCREEN ***************************/

void MediaPlayerScreen::onEntry() {
  UIScreen::onEntry();
  CLCD::turn_on_backlight();
  FTDI::SoundPlayer::set_volume(255);
}

void MediaPlayerScreen::onRedraw(draw_mode_t what) {
}

void MediaPlayerScreen::onIdle() {
}

void MediaPlayerScreen::lookForAutoPlayMedia() {
  #if defined(USE_FTDI_FT810)
    char     buf[512];
    Sd2Card  card;
    SdVolume volume;
    SdFile   root, file;

    // Check to see if a video file exists

    card.init(SPI_SPEED, SDSS);
    volume.init(&card);
    root.openRoot(&volume);

    strcpy_P(buf, PSTR("AUTOPLAY.AVI"));

    if(!root.exists(buf)) return;

    if(!file.open(&root, buf, O_READ)) {
      #ifdef SERIAL_PROTOCOLLNPGM
        SERIAL_PROTOCOLLNPGM("Failed to open AUTOPLAY.AVI");
      #else
        Serial.println("Failed to open AUTOPLAY.AVI");
      #endif
      return;
    }

    #ifdef SERIAL_PROTOCOLLNPGM
      SERIAL_PROTOCOLLNPGM("Starting to play AUTOPLAY.AVI");
    #else
      Serial.println("Starting to play AUTOPLAY.AVI");
    #endif

    // Set up the media FIFO on the end of RAMG, as the top of RAMG
    // will be used as the framebuffer.

    const uint32_t block_size = 512;
    const uint32_t fifo_size  = block_size * 2;
    const uint32_t fifo_start = RAM_G + RAM_G_SIZE - fifo_size;

    CommandProcessor cmd;
    cmd.cmd(CMD_DLSTART)
       .cmd(CLEAR_COLOR_RGB(0x00FF00))
       .cmd(CLEAR(true,true,true))
       .mediafifo(fifo_start, fifo_size)
       .playvideo(OPT_FULLSCREEN | OPT_MEDIAFIFO | OPT_NOTEAR)
       .execute();

    uint32_t writePtr = 0;
    int16_t  nBytes;

    uint32_t t = millis();
    uint8_t timeouts;

    do {
      // Write block n
      nBytes = file.read(buf, block_size);
      if(nBytes == -1) break;

      if(millis() - t > 10) {
        Extensible_UI_API::yield();
        watchdog_reset();
        t = millis();
      }

      CLCD::mem_write_bulk (fifo_start + writePtr, buf, nBytes);

      // Wait for FTDI810 to finish playing block n-1
      timeouts = 20;
      do {
        if(millis() - t > 10) {
          Extensible_UI_API::yield();
          watchdog_reset();
          t = millis();
          timeouts--;
          if(timeouts == 0) {
            SERIAL_PROTOCOLLNPGM("Timeout playing video");
            return;
          }
        }
      } while(CLCD::mem_read_32(REG_MEDIAFIFO_READ) != writePtr);

      // Start playing block n
      writePtr = (writePtr + nBytes) % fifo_size;
      CLCD::mem_write_32(REG_MEDIAFIFO_WRITE, writePtr);
    } while(nBytes == block_size);

    file.close();

    #ifdef SERIAL_PROTOCOLLNPGM
      SERIAL_PROTOCOLLNPGM("Done playing video");
    #else
      Serial.println("Done playing video");
    #endif

    // Since playing media overwrites RAMG, we need to reinitialize
    // everything that is stored in RAMG.

    cmd.cmd(CMD_DLSTART).execute();
    DLCache::init();
    StatusScreen::onStartup();
  #endif
}

/***************************** MARLIN CALLBACKS  ***************************/

namespace Extensible_UI_API {
  void onPrinterKilled(const char* lcd_msg) {
    KillScreen::show(progmem_str(lcd_msg));
  }

  void onMediaInserted() {
    StatusScreen::setStatusMessage(F(MSG_SD_INSERTED));
    sound.play(media_inserted, PLAY_ASYNCHRONOUS);

    MediaPlayerScreen::lookForAutoPlayMedia();
  }

  void onMediaRemoved() {
    StatusScreen::setStatusMessage(F(MSG_SD_REMOVED));
    sound.play(media_removed, PLAY_ASYNCHRONOUS);
  }

  void onStatusChanged(const char* lcd_msg) {
    StatusScreen::setStatusMessage(lcd_msg);
  }

  void onStatusChanged(progmem_str lcd_msg) {
    StatusScreen::setStatusMessage(lcd_msg);
  }
}

#endif // EXTENSIBLE_UI