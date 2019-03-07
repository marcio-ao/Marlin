/*********************************
 * interface_settings_screen.cpp *
 *********************************/

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
#include "screen_data.h"

#if ENABLED(EXTENSIBLE_UI)

#include "screens.h"

#include "../io/flash_storage.h"

#if ENABLED(LULZBOT_PRINTCOUNTER)
  #include "../../../../module/printcounter.h"
#endif

using namespace FTDI;
using namespace ExtUI;
using namespace Theme;

void InterfaceSettingsScreen::onStartup() {
  loadSettings();
}

void InterfaceSettingsScreen::onEntry() {
  screen_data.InterfaceSettingsScreen.brightness = CLCD::get_brightness();
  screen_data.InterfaceSettingsScreen.volume     = SoundPlayer::get_volume();
  BaseScreen::onEntry();
}

void InterfaceSettingsScreen::onRedraw(draw_mode_t what) {
  CommandProcessor cmd;

  if(what & BACKGROUND) {

    #define GRID_COLS 4
    #if defined(USE_PORTRAIT_ORIENTATION)
      #define GRID_ROWS 7
    #else
      #define GRID_ROWS 6
    #endif

    cmd.cmd(CLEAR_COLOR_RGB(background))
       .cmd(CLEAR(true,true,true))
       .tag(0)
       .font(font_medium)
       .text      (BTN_POS(1,1), BTN_SIZE(4,1), F("Interface Settings"))
    #undef EDGE_R
    #define EDGE_R 30
       .font(font_small)
       .tag(0)
       .text(BTN_POS(1,2), BTN_SIZE(2,1), F("Screen brightness:"), OPT_RIGHTX | OPT_CENTERY)
       .text(BTN_POS(1,3), BTN_SIZE(2,1), F("Sound volume:"),      OPT_RIGHTX | OPT_CENTERY)
       .text(BTN_POS(1,4), BTN_SIZE(2,1), F("Screen lock:"),       OPT_RIGHTX | OPT_CENTERY);
      #ifdef SPI_FLASH_SS
    cmd.text(BTN_POS(1,5), BTN_SIZE(2,1), F("Boot animation:"),    OPT_RIGHTX | OPT_CENTERY);
      #endif
    #undef EDGE_R
  }

  if(what & FOREGROUND) {
    #if defined(USE_PORTRAIT_ORIENTATION)
      constexpr uint8_t w = 2;
    #else
      constexpr uint8_t w = 1;
    #endif

    cmd.font(font_medium)
    #define EDGE_R 30
       .tag(2).slider(BTN_POS(3,2), BTN_SIZE(2,1), screen_data.InterfaceSettingsScreen.brightness, 128)
       .tag(3).slider(BTN_POS(3,3), BTN_SIZE(2,1), screen_data.InterfaceSettingsScreen.volume,     0xFF)
       .tag(4).toggle(BTN_POS(3,4), BTN_SIZE(w,1), F("off\xFFon"), LockScreen::is_enabled())
       #ifdef SPI_FLASH_SS
       .tag(5).toggle(BTN_POS(3,5), BTN_SIZE(w,1), F("off\xFFon"), UIData::animations_enabled())
       #endif
    #undef EDGE_R
    #define EDGE_R 0
    #if defined(USE_PORTRAIT_ORIENTATION)
      .tag(6).button (BTN_POS(1,6), BTN_SIZE(4,1), F("Customize Sounds"))
      .style(LIGHT_BTN)
      .tag(1).button (BTN_POS(1,7), BTN_SIZE(4,1), F("Back"));
    #else
      .tag(6).button (BTN_POS(1,6), BTN_SIZE(2,1), F("Customize Sounds"))
      .style(LIGHT_BTN)
      .tag(1).button (BTN_POS(3,6), BTN_SIZE(2,1), F("Back"));
    #endif
  }
}

bool InterfaceSettingsScreen::onTouchEnd(uint8_t tag) {
  switch(tag) {
    case 1: GOTO_PREVIOUS(); break;
    case 4:
      if(!LockScreen::is_enabled())
        LockScreen::enable();
      else
        LockScreen::disable();
      break;
    case 5: UIData::enable_animations(!UIData::animations_enabled());; break;
    case 6: GOTO_SCREEN(InterfaceSoundsScreen); break;
    default:
      return false;
  }
  return true;
}

bool InterfaceSettingsScreen::onTouchStart(uint8_t tag) {
  #undef EDGE_R
  #define EDGE_R 30
  CommandProcessor cmd;
  switch(tag) {
    case 2: cmd.track_linear(BTN_POS(3,3), BTN_SIZE(2,1), 2).execute(); break;
    case 3: cmd.track_linear(BTN_POS(3,4), BTN_SIZE(2,1), 3).execute(); break;
    default: break;
  }
  #undef EDGE_R
  #define EDGE_R 0
  #undef GRID_COLS
  #undef GRID_ROWS
  return true;
}

void InterfaceSettingsScreen::onIdle() {
  if(refresh_timer.elapsed(TOUCH_UPDATE_INTERVAL)) {
    refresh_timer.start();

    uint16_t value;
    CommandProcessor cmd;
    switch(cmd.track_tag(value)) {
      case 2:
        screen_data.InterfaceSettingsScreen.brightness = float(value) * 128 / 0xFFFF;
        CLCD::set_brightness(screen_data.InterfaceSettingsScreen.brightness);
        break;
      case 3:
        screen_data.InterfaceSettingsScreen.volume = value >> 8;
        SoundPlayer::set_volume(screen_data.InterfaceSettingsScreen.volume);
        break;
      default:
        return;
    }
    onRefresh();
  }
  BaseScreen::onIdle();
}

void InterfaceSettingsScreen::defaultSettings() {
  LockScreen::passcode = 0;
  SoundPlayer::set_volume(255);
  CLCD::set_brightness(255);
  UIData::reset_persistent_data();
  InterfaceSoundsScreen::defaultSettings();
  // TODO: This really should be moved to the EEPROM
  #if ENABLED(BACKLASH_GCODE)
    constexpr float backlash[XYZ] = BACKLASH_DISTANCE_MM;
    setAxisBacklash_mm(backlash[X_AXIS], X);
    setAxisBacklash_mm(backlash[Y_AXIS], Y);
    setAxisBacklash_mm(backlash[Z_AXIS], Z);
    setBacklashCorrection_percent(BACKLASH_CORRECTION);
    #ifdef BACKLASH_SMOOTHING_MM
      setBacklashSmoothing_mm(BACKLASH_SMOOTHING_MM);
    #endif
  #endif
  #if ENABLED(FILAMENT_RUNOUT_SENSOR) && defined(FILAMENT_RUNOUT_DISTANCE_MM)
    setFilamentRunoutDistance_mm(FILAMENT_RUNOUT_DISTANCE_MM);
    setFilamentRunoutEnabled(true);
  #endif
}

void InterfaceSettingsScreen::saveSettings() {
  persistent_data_t      data;
  data.magic_word        = data.MAGIC_WORD;
  data.version           = 0;
  data.sound_volume      = SoundPlayer::get_volume();
  data.screen_brightness = CLCD::get_brightness();
  data.passcode          = LockScreen::passcode;
  data.bit_flags         = UIData::get_persistent_data();
  data.touch_transform_a = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_A);
  data.touch_transform_b = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_B);
  data.touch_transform_c = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_C);
  data.touch_transform_d = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_D);
  data.touch_transform_e = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_E);
  data.touch_transform_f = CLCD::mem_read_32(REG_TOUCH_TRANSFORM_F);
  #if ENABLED(LULZBOT_BACKUP_EEPROM_INFORMATION)
    #if ENABLED(PRINTCOUNTER)
      // Keep a backup of the print counter information in SPI EEPROM
      // since the emulated EEPROM on the Due HAL does not survive
      // a reflash.
      printStatistics* stats   = print_job_timer.getStatsPtr();
      data.total_prints        = stats->totalPrints;
      data.finished_prints     = stats->finishedPrints;
      data.total_print_time    = stats->printTime;
      data.longest_print       = stats->longestPrint;
      data.total_filament_used = stats->filamentUsed;
    #endif
    #if EXTRUDERS > 1
      data.nozzle_offsets_mm[X_AXIS] = getNozzleOffset_mm(X, E1);
      data.nozzle_offsets_mm[Y_AXIS] = getNozzleOffset_mm(Y, E1);
      data.nozzle_offsets_mm[Z_AXIS] = getNozzleOffset_mm(Z, E1);
    #endif
    data.nozzle_z_offset           = getZOffset_mm();
  #endif
  // TODO: This really should be moved to the EEPROM
  #if ENABLED(BACKLASH_GCODE)
    data.backlash_distance_mm[X_AXIS] = getAxisBacklash_mm(X);
    data.backlash_distance_mm[Y_AXIS] = getAxisBacklash_mm(Y);
    data.backlash_distance_mm[Z_AXIS] = getAxisBacklash_mm(Z);
    data.backlash_correction          = getBacklashCorrection_percent();
    #ifdef BACKLASH_SMOOTHING_MM
      data.backlash_smoothing_mm      = getBacklashSmoothing_mm();
    #endif
  #endif
  #if ENABLED(FILAMENT_RUNOUT_SENSOR) && defined(FILAMENT_RUNOUT_DISTANCE_MM)
    data.runout_sensor_mm             = getFilamentRunoutDistance_mm();
    data.runout_sensor_enabled        = getFilamentRunoutEnabled();
  #endif
  for(uint8_t i = 0; i < InterfaceSoundsScreen::NUM_EVENTS; i++)
    data.event_sounds[i] = InterfaceSoundsScreen::event_sounds[i];
  UIFlashStorage::write_config_data(&data, sizeof(data));
}

void InterfaceSettingsScreen::loadSettings() {
  persistent_data_t data;
  UIFlashStorage::read_config_data(&data, sizeof(data));
  if(data.magic_word == data.MAGIC_WORD && data.version == 0) {
    SoundPlayer::set_volume(data.sound_volume);
    CLCD::set_brightness(data.screen_brightness);
    LockScreen::passcode = data.passcode;
    UIData::set_persistent_data(data.bit_flags);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_A, data.touch_transform_a);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_B, data.touch_transform_b);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_C, data.touch_transform_c);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_D, data.touch_transform_d);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_E, data.touch_transform_e);
    CLCD::mem_write_32(REG_TOUCH_TRANSFORM_F, data.touch_transform_f);
    #if ENABLED(LULZBOT_BACKUP_EEPROM_INFORMATION)
      #if ENABLED(PRINTCOUNTER)
        printStatistics* stats = print_job_timer.getStatsPtr();
        stats->totalPrints     = max(stats->totalPrints,    data.total_prints);
        stats->finishedPrints  = max(stats->finishedPrints, data.finished_prints);
        stats->printTime       = max(stats->printTime,      data.total_print_time);
        stats->longestPrint    = max(stats->longestPrint,   data.longest_print);
        stats->filamentUsed    = max(stats->filamentUsed,   data.total_filament_used);
      #endif
      #if EXTRUDERS > 1
        setNozzleOffset_mm(data.nozzle_offsets_mm[X_AXIS], X, E1);
        setNozzleOffset_mm(data.nozzle_offsets_mm[Y_AXIS], Y, E1);
        setNozzleOffset_mm(data.nozzle_offsets_mm[Z_AXIS], Z, E1);
      #endif
      setZOffset_mm(data.nozzle_z_offset);
    #endif
    // TODO: This really should be moved to the EEPROM
    #if ENABLED(BACKLASH_GCODE)
      setAxisBacklash_mm(data.backlash_distance_mm[X_AXIS], X);
      setAxisBacklash_mm(data.backlash_distance_mm[Y_AXIS], Y);
      setAxisBacklash_mm(data.backlash_distance_mm[Z_AXIS], Z);
      setBacklashCorrection_percent(data.backlash_correction);
      #ifdef BACKLASH_SMOOTHING_MM
        setBacklashSmoothing_mm(data.backlash_smoothing_mm);
      #endif
    #endif
    #if ENABLED(FILAMENT_RUNOUT_SENSOR) && defined(FILAMENT_RUNOUT_DISTANCE_MM)
      setFilamentRunoutDistance_mm(data.runout_sensor_mm);
      setFilamentRunoutEnabled(data.runout_sensor_enabled);
    #endif
    for(uint8_t i = 0; i < InterfaceSoundsScreen::NUM_EVENTS; i++)
      InterfaceSoundsScreen::event_sounds[i] = data.event_sounds[i];
  }
}

#endif // EXTENSIBLE_UI