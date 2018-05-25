/*****************
 * ui_sounds.cpp *
 *****************/

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

#include "ui.h"

#if defined(EXTENSIBLE_UI)

#include "ftdi_eve_constants.h"
#include "ftdi_eve_functions.h"

#include "ui_sounds.h"

/******************* TINY INTERVAL CLASS ***********************/

bool tiny_interval_t::elapsed() {
  uint8_t now = tiny_interval(millis());
  if(now > end) {
    return true;
  } else {
    return false;
  }
}

void tiny_interval_t::wait_for(uint32_t ms) {
  uint32_t now = millis();
  end = tiny_interval(now + ms);
  if(tiny_interval(now + ms*2) < end) {
    // Avoid special case where timer
    // might get wedged and stop firing.
    end = 0;
  }
}

/******************* SOUND HELPER CLASS ************************/

namespace FTDI {
  SoundPlayer sound; // Global sound player object

  const PROGMEM SoundPlayer::sound_t SoundPlayer::silence[] = {
    {SILENCE, END_SONG, 0}
  };

  void SoundPlayer::set_volume(uint8_t vol) {
    CLCD::mem_write_8(REG_VOL_SOUND, vol);
  }

  void SoundPlayer::play(effect_t effect, note_t note) {
    CLCD::mem_write_16(REG_SOUND, (note << 8) | effect);
    CLCD::mem_write_8( REG_PLAY,  1);

    #if defined(UI_FRAMEWORK_DEBUG)
      #if defined (SERIAL_PROTOCOLLNPAIR)
        SERIAL_PROTOCOLPAIR("Playing note ", note);
        SERIAL_PROTOCOLLNPAIR(", instrument ", effect);
      #endif
    #endif
  }

  note_t SoundPlayer::frequency_to_midi_note(const uint16_t frequency_hz) {
    const float f0 = 440;
    return note_t(NOTE_A4 + (log(frequency_hz)-log(f0))*12/log(2) + 0.5);
  }

  // Plays a tone of a given frequency and duration. Since the FTDI FT810 only
  // supports MIDI notes, we round down to the nearest note.

  void SoundPlayer::play_tone(const uint16_t frequency_hz, const uint16_t duration_ms) {
    play(ORGAN, frequency_to_midi_note(frequency_hz));

    // Schedule silence to squelch the note after the duration expires.
    sequence = silence;
    next = tiny_interval_t::tiny_interval(millis() + duration_ms);
  }

  void SoundPlayer::play(const sound_t* seq) {
    sequence = seq;
    // Delaying the start of the sound seems to prevent glitches. Not sure why...
    next     = tiny_interval_t::tiny_interval(millis()+250);
  }

  bool SoundPlayer::is_sound_playing() {
    return CLCD::mem_read_8( REG_PLAY ) & 0x1;
  }

  void SoundPlayer::onIdle() {
    if(!sequence) return;

    const uint8_t tiny_millis = tiny_interval_t::tiny_interval(millis());
    const bool ready_for_next_note = (next == WAIT) ? !is_sound_playing() : (tiny_millis > next);

    if(ready_for_next_note) {
      const effect_t fx = effect_t(pgm_read_byte_near(&sequence->effect));
      const note_t   nt =   note_t(pgm_read_byte_near(&sequence->note));
      const uint16_t ms = uint32_t(pgm_read_byte_near(&sequence->sixteenths)) * 1000 / 16;

      if(ms == 0 && fx == SILENCE && nt == 0) {
        sequence = 0;
        play(SILENCE, REST);
      } else {
        #if defined(UI_FRAMEWORK_DEBUG)
          #if defined (SERIAL_PROTOCOLLNPAIR)
            SERIAL_PROTOCOLLNPAIR("Scheduling note in ", ms);
          #endif
        #endif
        next =   (ms == WAIT) ? 0       : (tiny_interval_t::tiny_interval(millis() + ms));
        play(fx, (nt == 0)    ? NOTE_C4 : nt);
        sequence++;
      }
    }
  }
} // namespace FTDI

#endif // EXTENSIBLE_UI