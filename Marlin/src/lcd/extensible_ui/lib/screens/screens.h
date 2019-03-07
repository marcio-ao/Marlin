/*************
 * screens.h *
 *************/

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

#pragma once

#include "../ftdi_eve_lib/ftdi_eve_lib.h"
#include "../theme/theme.h"

#define ROUND(val) uint16_t((val)+0.5)

extern tiny_timer_t refresh_timer;

/********************************* DL CACHE SLOTS ******************************/

// In order to reduce SPI traffic, we cache display lists (DL) in RAMG. This
// is done using the CLCD::DLCache class, which takes a unique ID for each
// cache location. These IDs are defined here:

enum {
  STATUS_SCREEN_CACHE,
  MENU_SCREEN_CACHE,
  TUNE_SCREEN_CACHE,
  ADJUST_OFFSETS_SCREEN_CACHE,
  ALERT_BOX_CACHE,
  SPINNER_CACHE,
  ADVANCED_SETTINGS_SCREEN_CACHE,
  MOVE_AXIS_SCREEN_CACHE,
  TEMPERATURE_SCREEN_CACHE,
  STEPS_SCREEN_CACHE,
  ZOFFSET_SCREEN_CACHE,
  NOZZLE_OFFSET_SCREEN_CACHE,
  BACKLASH_COMPENSATION_SCREEN_CACHE,
  MAX_FEEDRATE_SCREEN_CACHE,
  MAX_VELOCITY_SCREEN_CACHE,
  MAX_ACCELERATION_SCREEN_CACHE,
  DEFAULT_ACCELERATION_SCREEN_CACHE,
#if ENABLED(JUNCTION_DEVIATION)
  JUNC_DEV_SCREEN_CACHE,
#else
  JERK_SCREEN_CACHE,
#endif
#if ENABLED(LIN_ADVANCE) || ENABLED(FILAMENT_RUNOUT_SENSOR)
  FILAMENT_OPTIONS_CACHE,
#endif
  CHANGE_FILAMENT_SCREEN_CACHE,
  INTERFACE_SETTINGS_SCREEN_CACHE,
  INTERFACE_SOUNDS_SCREEN_CACHE,
  LOCK_SCREEN_CACHE,
  FILES_SCREEN_CACHE
};

// To save MCU RAM, the status message is "baked" in to the status screen
// cache, so we reserve a large chunk of memory for the DL cache

#define STATUS_SCREEN_DL_SIZE        2048
#define ALERT_BOX_DL_SIZE            3072
#define SPINNER_DL_SIZE              3072
#define FILE_SCREEN_DL_SIZE          3072

/************************* MENU SCREEN DECLARATIONS *************************/

class BaseScreen : public UIScreen {
  protected:
    enum {
      NORMAL    = 0x00,
      DISABLED  = 0x01,
      RED_BTN   = 0x02,
      LIGHT_BTN = 0x04
    };

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
    static void onRedraw(draw_mode_t);
    static void onIdle();
};

class AboutScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

#if ENABLED(PRINTCOUNTER)
  class StatisticsScreen : public BaseScreen, public UncachedScreen {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchEnd(uint8_t tag);
  };
#endif

class KillScreen {
  // The KillScreen is behaves differently than the
  // others, so we do not bother extending UIScreen.
  public:
    static void show(progmem_str msg);
};

class DialogBoxBaseClass : public BaseScreen {
  protected:
    static void drawMessage(const progmem_str, const progmem_str = 0, const progmem_str = 0, int16_t font = 0);
    static void drawYesNoButtons();
    static void drawOkayButton();
    static void drawSpinner();

    static void onRedraw(draw_mode_t) {};
  public:
    static bool onTouchEnd(uint8_t tag);
};

class AlertDialogBox : public DialogBoxBaseClass, public CachedScreen<ALERT_BOX_CACHE,ALERT_BOX_DL_SIZE> {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t);
    static void show(const progmem_str, const progmem_str = 0, const progmem_str = 0);
    static void showError(const progmem_str, const progmem_str = 0, const progmem_str = 0);
};

class RestoreFailsafeDialogBox : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class SaveSettingsDialogBox : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);

    static void promptToSaveSettings();
};

class ConfirmAbortPrintDialogBox : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

#if ENABLED(CALIBRATION_GCODE)
class ConfirmAutoCalibrationDialogBox : public DialogBoxBaseClass, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};
#endif

class SpinnerDialogBox : public DialogBoxBaseClass, public CachedScreen<SPINNER_CACHE,SPINNER_DL_SIZE> {
  public:
    static void onRedraw(draw_mode_t);
    static void show(const progmem_str, const progmem_str = 0, const progmem_str = 0);
    static void hide();
};

class StatusScreen : public BaseScreen, public CachedScreen<STATUS_SCREEN_CACHE,STATUS_SCREEN_DL_SIZE> {
  private:
    static void draw_axis_position(draw_mode_t);
    static void draw_temperature(draw_mode_t);
    static void draw_progress(draw_mode_t);
    static void draw_interaction_buttons(draw_mode_t);
    static void draw_status_message(draw_mode_t, const char * const);

  public:
    static void setStatusMessage(const char *);
    static void setStatusMessage(progmem_str);
    static void onRedraw(draw_mode_t);
    static void onStartup();
    static void onEntry();
    static void onIdle();
    static bool onTouchEnd(uint8_t tag);
};

class MainMenu : public BaseScreen, public CachedScreen<MENU_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class TuneMenu : public BaseScreen, public CachedScreen<TUNE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class TouchCalibrationScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t);
    static void onIdle();
};

class TouchRegistersScreen : public BaseScreen, public UncachedScreen {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class AdvancedSettingsMenu : public BaseScreen, public CachedScreen<ADVANCED_SETTINGS_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class ChangeFilamentScreen : public BaseScreen, public CachedScreen<CHANGE_FILAMENT_SCREEN_CACHE> {
  private:
    static uint8_t getSoftenTemp();
    static ExtUI::extruder_t getExtruder();
    static void drawTempGradient(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    static uint32_t getTempColor(uint32_t temp);
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
    static bool onTouchHeld(uint8_t tag);
    static void onIdle();
};

class BaseNumericAdjustmentScreen : public BaseScreen {
  public:
    enum precision_default_t {
      DEFAULT_LOWEST,
      DEFAULT_MIDRANGE,
      DEFAULT_HIGHEST
    };

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
        widgets_t(draw_mode_t);

        widgets_t &color(uint32_t color)       {_color = color; return *this;}
        widgets_t &units(const char *units)    {_units = units; return *this;}
        widgets_t &draw_mode(draw_mode_t what) {_what  = what;  return *this;}
        widgets_t &precision(uint8_t decimals, precision_default_t = DEFAULT_HIGHEST);

        void heading       (const char *label);
        void adjuster_sram_val (uint8_t tag,  const char *label, const char *value,  bool is_enabled = true);
        void adjuster          (uint8_t tag,  const char *label, const char *value,  bool is_enabled = true);
        void adjuster          (uint8_t tag,  const char *label, float value=0,      bool is_enabled = true);
        void button            (uint8_t tag,  const char *label,                     bool is_enabled = true);
        void text_field        (uint8_t tag,  const char *label, const char *value,  bool is_enabled = true);
        void two_buttons       (uint8_t tag1, const char *label1,
                                uint8_t tag2, const char *label2,                    bool is_enabled = true);
        void toggle            (uint8_t tag,  const char *label, const char *text, bool value, bool is_enabled = true);
        void home_buttons      (uint8_t tag);
        void increments        ();
    };

    static float getIncrement();

  public:
    static void onEntry();
    static bool onTouchEnd(uint8_t tag);
};

class MoveAxisScreen : public BaseNumericAdjustmentScreen, public CachedScreen<MOVE_AXIS_SCREEN_CACHE> {
  private:
    static float getManualFeedrate(uint8_t axis, float increment_mm);
  public:
    static void setManualFeedrate(ExtUI::axis_t, float increment_mm);
    static void setManualFeedrate(ExtUI::extruder_t, float increment_mm);

    static void onEntry();
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
    static void onIdle();
};

class StepsScreen : public BaseNumericAdjustmentScreen, public CachedScreen<STEPS_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

#if HAS_BED_PROBE
  class ZOffsetScreen : public BaseNumericAdjustmentScreen, public CachedScreen<ZOFFSET_SCREEN_CACHE> {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
  };
#endif

#if HOTENDS > 1
  class NozzleOffsetScreen : public BaseNumericAdjustmentScreen, public CachedScreen<NOZZLE_OFFSET_SCREEN_CACHE> {
    public:
      static void onEntry();
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
  };
#endif

#if ENABLED(BABYSTEPPING)
  class NudgeNozzleScreen : public BaseNumericAdjustmentScreen, public CachedScreen<ADJUST_OFFSETS_SCREEN_CACHE> {
    public:
      static void onEntry();
      static void onRedraw(draw_mode_t);
      static bool onTouchEnd(uint8_t tag);
      static bool onTouchHeld(uint8_t tag);
      static void onIdle();
  };
#endif

#if ENABLED(BACKLASH_GCODE)
  class BacklashCompensationScreen : public BaseNumericAdjustmentScreen, public CachedScreen<BACKLASH_COMPENSATION_SCREEN_CACHE> {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
  };
#endif

class MaxFeedrateScreen : public BaseNumericAdjustmentScreen, public CachedScreen<MAX_FEEDRATE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

class MaxVelocityScreen : public BaseNumericAdjustmentScreen, public CachedScreen<MAX_VELOCITY_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

class MaxAccelerationScreen : public BaseNumericAdjustmentScreen, public CachedScreen<MAX_ACCELERATION_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

class DefaultAccelerationScreen : public BaseNumericAdjustmentScreen, public CachedScreen<DEFAULT_ACCELERATION_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

#if ENABLED(JUNCTION_DEVIATION)
  class JunctionDeviationScreen : public BaseNumericAdjustmentScreen, public CachedScreen<JUNC_DEV_SCREEN_CACHE> {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
  };
#else
  class JerkScreen : public BaseNumericAdjustmentScreen, public CachedScreen<JERK_SCREEN_CACHE> {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
  };
#endif

#if ENABLED(LIN_ADVANCE) || ENABLED(FILAMENT_RUNOUT_SENSOR)
  class FilamentOptionsScreen : public BaseNumericAdjustmentScreen, public CachedScreen<FILAMENT_OPTIONS_CACHE> {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchHeld(uint8_t tag);
      static bool onTouchEnd(uint8_t tag);
  };
#endif

class TemperatureScreen : public BaseNumericAdjustmentScreen, public CachedScreen<TEMPERATURE_SCREEN_CACHE> {
  public:
    static void onRedraw(draw_mode_t);
    static bool onTouchHeld(uint8_t tag);
};

class InterfaceSoundsScreen : public BaseScreen, public CachedScreen<INTERFACE_SOUNDS_SCREEN_CACHE> {
  public:
    enum event_t {
      PRINTING_STARTED  = 0,
      PRINTING_FINISHED = 1,
      PRINTING_FAILED   = 2
    };
    static constexpr uint8_t NUM_EVENTS = 3;

  private:
    friend class InterfaceSettingsScreen;

    static uint8_t event_sounds[NUM_EVENTS];

    static const char* getSoundSelection(event_t);
    static void toggleSoundSelection(event_t);
    static void setSoundSelection(event_t, const FTDI::SoundPlayer::sound_t*);

  public:
    static void playEventSound(event_t, FTDI::play_mode_t = FTDI::PLAY_ASYNCHRONOUS);

    static void defaultSettings();

    static void onRedraw(draw_mode_t);
    static bool onTouchStart(uint8_t tag);
    static bool onTouchEnd(uint8_t tag);
    static void onIdle();
};

class InterfaceSettingsScreen : public BaseScreen, public CachedScreen<INTERFACE_SETTINGS_SCREEN_CACHE> {
  private:
    struct persistent_data_t {
      static constexpr uint32_t MAGIC_WORD = 0x4C756C7A; // 'Lulz'
      uint32_t magic_word;
      uint16_t version;
      uint8_t  sound_volume;
      uint8_t  screen_brightness;
      uint8_t  bit_flags;
      uint16_t passcode;
      uint32_t touch_transform_a;
      uint32_t touch_transform_b;
      uint32_t touch_transform_c;
      uint32_t touch_transform_d;
      uint32_t touch_transform_e;
      uint32_t touch_transform_f;
      uint8_t event_sounds[InterfaceSoundsScreen::NUM_EVENTS];
      #if ENABLED(LULZBOT_BACKUP_EEPROM_INFORMATION)
        #if ENABLED(PRINTCOUNTER)
          // Keep a backup of the print counter information in SPI EEPROM
          // since the emulated EEPROM on the Due HAL does not survive
          // a reflash.
          uint16_t total_prints;
          uint16_t finished_prints;
          uint32_t total_print_time;
          uint32_t longest_print;
          float    total_filament_used;
        #endif
        float nozzle_offsets_mm[XYZ];
        float nozzle_z_offset;
      #endif
      // TODO: The following should really be stored in the EEPROM
      #if ENABLED(BACKLASH_GCODE)
        float backlash_distance_mm[XYZ];
        float backlash_correction;
        #ifdef BACKLASH_SMOOTHING_MM
          float backlash_smoothing_mm;
        #endif
      #endif
      #if ENABLED(FILAMENT_RUNOUT_SENSOR)
        bool runout_sensor_enabled;
        #if defined(FILAMENT_RUNOUT_DISTANCE_MM)
          float runout_sensor_mm;
        #endif
      #endif
    };

  public:
    static void saveSettings();
    static void loadSettings();
    static void defaultSettings();

    static void onStartup();
    static void onEntry();
    static void onRedraw(draw_mode_t);
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
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

class FilesScreen : public BaseScreen, public CachedScreen<FILES_SCREEN_CACHE, FILE_SCREEN_DL_SIZE> {
  private:
    #if defined(USE_PORTRAIT_ORIENTATION)
      static constexpr uint8_t header_h       = 2;
      static constexpr uint8_t footer_h       = 2;
      static constexpr uint8_t files_per_page = 11;
    #else
      static constexpr uint8_t header_h       = 1;
      static constexpr uint8_t footer_h       = 1;
      static constexpr uint8_t files_per_page = 6;
    #endif

    static uint8_t  getTagForLine(uint8_t line) {return line + 2;}
    static uint8_t  getLineForTag(uint8_t tag)  {return  tag - 2;}
    static uint16_t getFileForTag(uint8_t tag);

    static const char *getSelectedShortFilename();
    static void drawFileButton(const char* filename, uint8_t tag, bool is_dir, bool is_highlighted);
    static void drawFileList();
    static void drawHeader();
    static void drawFooter();
    static void drawSelectedFile();

    static void gotoPage(uint8_t);
  public:
    static void onEntry();
    static void onRedraw(draw_mode_t);
    static bool onTouchEnd(uint8_t tag);
};

#if ENABLED(DEVELOPER_SCREENS)
  class DeveloperMenu : public BaseScreen, public UncachedScreen {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchEnd(uint8_t tag);
  };

  class ConfirmEraseFlashDialogBox : public DialogBoxBaseClass, public UncachedScreen {
    public:
      static void onRedraw(draw_mode_t);
      static bool onTouchEnd(uint8_t tag);
  };

  class WidgetsScreen : public BaseScreen, public UncachedScreen {
    public:
      static void onEntry();
      static void onRedraw(draw_mode_t);
      static bool onTouchStart(uint8_t tag);
      static void onIdle();
  };

  class DiagnosticsScreen : public BaseScreen, public UncachedScreen {
    public:
      static void onEntry();
      static void onExit();
      static void onRedraw(draw_mode_t);
      static bool onTouchEnd(uint8_t tag);
      static void onIdle();
  };
#endif

class MediaPlayerScreen : public BaseScreen, public UncachedScreen {
  private:
    typedef int16_t media_streamer_func_t(void *obj, void *buff, size_t bytes);

  public:
    static bool playCardMedia();
    static bool playBootMedia();

    static void onEntry();
    static void onRedraw(draw_mode_t);

    static void playStream(void *obj, media_streamer_func_t*);
};