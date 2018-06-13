/****************
 * ui_screens.h *
 ****************/

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

#ifndef _UI_SCREENS_
#define _UI_SCREENS_

#include "ui_framework.h"
#include "ui_dl_cache.h"

typedef const __FlashStringHelper *progmem_str;

/********************************* DL CACHE SLOTS ******************************/

// In order to reduce SPI traffic, we cache display lists (DL) in RAMG. This
// is done using the CLCD::DLCache class, which takes a unique ID for each
// cache location. These IDs are defined here:

enum {
  STATUS_SCREEN_CACHE,
  MENU_SCREEN_CACHE,
  TUNE_SCREEN_CACHE,
  ALERT_BOX_CACHE,
  ADVANCED_SETTINGS_SCREEN_CACHE,
  MOVE_AXIS_SCREEN_CACHE,
  TEMPERATURE_SCREEN_CACHE,
  STEPS_SCREEN_CACHE,
  ZOFFSET_SCREEN_CACHE,
  FEEDRATE_SCREEN_CACHE,
  VELOCITY_SCREEN_CACHE,
  ACCELERATION_SCREEN_CACHE,
  JERK_SCREEN_CACHE,
  CHANGE_FILAMENT_SCREEN_CACHE,
  INTERFACE_SETTINGS_SCREEN_CACHE,
  LOCK_SCREEN_CACHE,
  FILES_SCREEN_CACHE
};

// To save MCU RAM, the status message is "baked" in to the status screen
// cache, so we reserve a large chunk of memory for the DL cache

#define STATUS_SCREEN_DL_SIZE        2048
#define ALERT_BOX_DL_SIZE            3072

/************************** REFRESH METHOD SHIMS ***************************/

class UncachedScreen {
  public:
    static void onRefresh(){
      using namespace FTDI;
      CLCD::CommandFifo cmd;
      cmd.cmd(CMD_DLSTART);

      current_screen.onRedraw(BOTH);

      cmd.cmd(DL::DL_DISPLAY);
      cmd.cmd(CMD_SWAP);
      cmd.execute();
    }
};

template<uint8_t DL_SLOT,uint32_t DL_SIZE = 0>
class CachedScreen {
  protected:
    static bool storeBackground(){
      DLCache dlcache(DL_SLOT);
      if(!dlcache.store(DL_SIZE)) {
        #if defined (SERIAL_PROTOCOLLNPAIR)
          SERIAL_PROTOCOLLN("CachedScreen::storeBackground() failed: not enough DL cache space");
        #else
          Serial.print(CachedScreen::storeBackground() failed: not enough DL cache space);
        #endif
        return false;
      }
      return true;
    }

    static void repaintBackground(){
      using namespace FTDI;
      DLCache dlcache(DL_SLOT);
      CLCD::CommandFifo cmd;

      cmd.cmd(CMD_DLSTART);
      current_screen.onRedraw(BACKGROUND);

      dlcache.store(DL_SIZE);
    }

  public:
    static void onRefresh(){
      using namespace FTDI;
      DLCache dlcache(DL_SLOT);
      CLCD::CommandFifo cmd;

      cmd.cmd(CMD_DLSTART);

      if(dlcache.has_data()) {
        dlcache.append();
      } else {
        current_screen.onRedraw(BACKGROUND);
        dlcache.store(DL_SIZE);
      }

      current_screen.onRedraw(FOREGROUND);

      cmd.cmd(DL::DL_DISPLAY);
      cmd.cmd(CMD_SWAP);
      cmd.execute();
    }
};

/************************* MENU SCREEN DECLARATIONS *************************/

class BaseScreen : public UIScreen {
  protected:
    #if defined(MENU_TIMEOUT)
      static uint32_t last_interaction;
    #endif

    static void default_button_colors();
  public:
    static bool buttonStyleCallback(uint8_t tag, uint8_t &style, uint16_t &options, bool post);

    static void reset_menu_timeout();

    static void onEntry();
    static void onIdle();
};

class BootScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static void onIdle();
};

class AboutScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class KillScreen {
  // The KillScreen is behaves differently than the
  // others, so we do not bother extending UIScreen.
  public:
    static void show(progmem_str msg);
};

class DialogBoxBaseClass : public BaseScreen  {
  protected:
    static void drawMessage(const progmem_str line1, const progmem_str line2 = 0, const progmem_str line3 = 0);
    static void drawYesNoButtons();
    static void drawOkayButton();

    static void onRedraw(draw_mode_t what) {};
  public:
    static bool onTouchEnd(uint8_t tag);
};

class AlertBoxScreen : public DialogBoxBaseClass, public CachedScreen<ALERT_BOX_CACHE,ALERT_BOX_DL_SIZE> {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static void show(const progmem_str line1, const progmem_str line2 = 0, const progmem_str line3 = 0);
};

class RestoreFailsafeScreen : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class SaveSettingsScreen : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class ConfirmAbortPrint : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class StatusScreen : public BaseScreen, public CachedScreen<STATUS_SCREEN_CACHE,STATUS_SCREEN_DL_SIZE> {
  private:
    static void draw_axis_position(draw_mode_t what);
    static void draw_temperature(draw_mode_t what);
    static void draw_progress(draw_mode_t what);
    static void draw_interaction_buttons(draw_mode_t what);
    static void draw_status_message(draw_mode_t what, const char * const message);

  public:
    static void setStatusMessage(const char * message);
    static void setStatusMessage(progmem_str message);
    static void onRedraw(draw_mode_t what);
    static void onStartup();
    static void onEntry();
    static void onIdle();
    static bool onTouchEnd(uint8_t tag);
};

class MenuScreen : public BaseScreen, public CachedScreen<MENU_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class TuneScreen : public BaseScreen, public CachedScreen<TUNE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class CalibrationScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static void onIdle();
};

class CalibrationRegistersScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class AdvancedSettingsScreen : public BaseScreen, public CachedScreen<ADVANCED_SETTINGS_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class ChangeFilamentScreen : public BaseScreen, public CachedScreen<CHANGE_FILAMENT_SCREEN_CACHE> {
  private:
    static uint8_t getSoftenTemp();
    static uint8_t getExtruder();
    static void drawTempGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    static uint32_t getTempColor(uint32_t temp);
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
    static bool onTouchHeld(uint8_t tag);
    static void onIdle();
};

class ValueAdjusters : public BaseScreen {
  protected:

    class widgets_t {
      private:
        draw_mode_t _what;
        uint8_t     _line;
        uint32_t    _color;
        uint8_t     _decimals;
        const char *_units;

      protected:
        void _draw_increment_btn(uint8_t line, const uint8_t tag);

      public:
        widgets_t(draw_mode_t what);

        inline widgets_t &color(uint32_t color)       {_color = color; return *this;}
        inline widgets_t &units(const char *units)    {_units = units; return *this;}
        inline widgets_t &precision(uint8_t decimals) {_decimals = decimals; return *this;}

        void heading  (const char *label);
        void adjuster (uint8_t tag, const char *label, float value=0);
        void increments();
    };

    static float getIncrement();

  public:
    static void onEntry();
    static bool onTouchEnd(uint8_t tag);
};

class MoveAxisScreen : public ValueAdjusters, public CachedScreen<MOVE_AXIS_SCREEN_CACHE> {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

class StepsScreen : public ValueAdjusters, public CachedScreen<STEPS_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

#if HAS_BED_PROBE
  class ZOffsetScreen : public ValueAdjusters, public CachedScreen<ZOFFSET_SCREEN_CACHE> {
    public:
      static void onRedraw(draw_mode_t what);
      static bool onTouchHeld(uint8_t tag);
  };
#endif

class FeedrateScreen : public ValueAdjusters, public CachedScreen<FEEDRATE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

class VelocityScreen : public ValueAdjusters, public CachedScreen<VELOCITY_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

class AccelerationScreen : public ValueAdjusters, public CachedScreen<ACCELERATION_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

class JerkScreen : public ValueAdjusters, public CachedScreen<JERK_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};


class TemperatureScreen : public ValueAdjusters, public CachedScreen<TEMPERATURE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t what);
    static bool onTouchHeld(uint8_t tag);
};

class InterfaceSettingsScreen : public BaseScreen, public CachedScreen<INTERFACE_SETTINGS_SCREEN_CACHE> {
  private:
    struct persistent_data_t {
      uint32_t magic_word;
      uint16_t version;
      uint8_t  sound_volume;
      uint8_t  screen_brightness;
      uint16_t passcode;
    };

  public:
    static void saveSettings();
    static void loadSettings();
    static void defaultSettings();

    static void onStartup();
    static void onEntry();
    static void onExit();
    static void onRedraw(draw_mode_t what);
    static bool onTouchStart(uint8_t tag);
    static bool onTouchEnd(uint8_t tag);
    static void onIdle();
};

class LockScreen : public BaseScreen, public CachedScreen<LOCK_SCREEN_CACHE> {
  private:
    friend InterfaceSettingsScreen;

    static uint16_t passcode;

    static char & message_style();
    static uint16_t compute_checksum();
    static void onPasscodeEntered();
  public:
    static bool is_enabled();
    static void check_passcode();
    static void enable();
    static void disable();

    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class FilesScreen : public BaseScreen, public CachedScreen<FILES_SCREEN_CACHE> {
  private:
    static const char *getSelectedShortFilename();
    static uint8_t getTagForIndex(uint16_t index);
    static uint16_t getIndexForTag(uint8_t tag);
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchEnd(uint8_t tag);
};

class WidgetsScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t what);
    static bool onTouchStart(uint8_t tag);
    static void onIdle();
};

class MediaPlayerScreen : public BaseScreen, public UncachedScreen {
  public:
    static void lookForAutoPlayMedia();

    static void onEntry();
    static void onRedraw(draw_mode_t what);
};

#endif // _UI_SCREENS_