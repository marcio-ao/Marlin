/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "../../Marlin.h"

#if ENABLED(CALIBRATION_GCODE)

#include "../gcode.h"
#include "../../lcd/ultralcd.h"
#include "../../module/motion.h"
#include "../../module/planner.h"
#include "../../module/tool_change.h"
#include "../../module/endstops.h"

//#define CALIBRATION_MEASUREMENT_DEBUG

/**
 * G425 backs away from the cube by various distances depending
 * on the user's confidence level:
 *
 *   UNKNOWN   - No real notion on where the cube is on the bed
 *   UNCERTAIN - Measurement may be uncertain due to backlash
 *   CERTAIN   - Measurement obtained with backlash compensation
 */

#ifndef CALIBRATION_MEASUREMENT_UNKNOWN
  #define CALIBRATION_MEASUREMENT_UNKNOWN   5.0 // mm
#endif
#ifndef CALIBRATION_MEASUREMENT_UNCERTAIN
  #define CALIBRATION_MEASUREMENT_UNCERTAIN 1.0 // mm
#endif
#ifndef CALIBRATION_MEASUREMENT_CERTAIN
  #define CALIBRATION_MEASUREMENT_CERTAIN   0.5 // mm
#endif

#define HAS_X_CENTER (ENABLED(CALIBRATION_CUBE_MEASURE_LEFT)  && ENABLED(CALIBRATION_CUBE_MEASURE_RIGHT))
#define HAS_Y_CENTER (ENABLED(CALIBRATION_CUBE_MEASURE_FRONT) && ENABLED(CALIBRATION_CUBE_MEASURE_BACK))

#if ENABLED(BACKLASH_GCODE)
  extern float backlash_distance_mm[], backlash_correction, backlash_smoothing_mm;
#endif

enum side_t : uint8_t { TOP, RIGHT, FRONT, LEFT, BACK, NUM_SIDES };

struct measurements_t {
  static const float dimensions[XYZ];
  static constexpr float true_center[XYZ] = CALIBRATION_CUBE_CENTER;
  float cube_center[XYZ] = CALIBRATION_CUBE_CENTER;
  float cube_side[NUM_SIDES];

  float backlash[NUM_SIDES];
  float pos_error[XYZ];

  float nozzle_outer_dimension[2] = {CALIBRATION_NOZZLE_OUTER_DIAMETER, CALIBRATION_NOZZLE_OUTER_DIAMETER};
};

const float measurements_t::dimensions[]  = CALIBRATION_CUBE_DIMENSIONS;

/**
 * A class to save and change the endstop state,
 * then restore it when it goes out of scope.
 */
class TemporaryEndstopsState {
  private:
    bool saved_endstops_global_enabled, saved_soft_endstops_enabled;

  public:
    TemporaryEndstopsState(bool enable) {
      saved_endstops_global_enabled = endstops.global_enabled();
      endstops.enable_globally(enable);

      saved_soft_endstops_enabled = soft_endstops_enabled;
      soft_endstops_enabled = enable;
    }

    ~TemporaryEndstopsState() {
      soft_endstops_enabled = saved_soft_endstops_enabled;
      endstops.enable_globally(saved_endstops_global_enabled);
    }
};

/**
 * A class to save and change the backlash compensation state,
 * then restore it when it goes out of scope.
 */
class TemporaryBacklashCompensation {
  public:
    TemporaryBacklashCompensation(const bool enable) {
      #if ENABLED(BACKLASH_GCODE)
        REMEMBER(backlash_correction);
        #ifdef BACKLASH_SMOOTHING_MM
          REMEMBER(backlash_smoothing_mm);
          backlash_smoothing_mm = 0;
        #endif
        backlash_correction = enable;
      #endif
    }
};

/**
 * Move to a particular location. Up to three individual axes
 * and their destinations can be specified, in any order.
 */
inline void move_to(
  const AxisEnum a1, const float p1,
  const AxisEnum a2, const float p2,
  const AxisEnum a3, const float p3
) {
  set_destination_from_current();

  // Note: The order of p1, p2, p3 may not correspond to X, Y, Z
  if (a1 != NO_AXIS) destination[a1] = p1;
  if (a2 != NO_AXIS) destination[a2] = p2;
  if (a3 != NO_AXIS) destination[a3] = p3;

  // Make sure coordinates are within bounds
  destination[X_AXIS] = MAX(MIN(destination[X_AXIS], X_MAX_POS), X_MIN_POS);
  destination[Y_AXIS] = MAX(MIN(destination[Y_AXIS], Y_MAX_POS), Y_MIN_POS);
  destination[Z_AXIS] = MAX(MIN(destination[Z_AXIS], Z_MAX_POS), Z_MIN_POS);

  // Move to position
  do_blocking_move_to(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], MMM_TO_MMS(CALIBRATION_FEEDRATE_TRAVEL));
}

/**
 * Move to the exact center above the calibration cube
 *
 *   m                  in     - Measurement record
 *   uncertainty        in     - How far away from the cube top to park
 */
inline void park_above_cube(measurements_t &m, const float uncertainty) {
  /* Move to safe distance above cube */
  move_to(Z_AXIS, m.cube_center[Z_AXIS] + m.dimensions[Z_AXIS] / 2 + uncertainty);

  /* Move to center of cube in XY */
  move_to(X_AXIS, m.cube_center[X_AXIS], Y_AXIS, m.cube_center[Y_AXIS]);
}

#if HOTENDS > 1

  inline void set_nozzle(measurements_t &m, const uint8_t extruder) {
    if (extruder != active_extruder) {
      park_above_cube(m, CALIBRATION_MEASUREMENT_UNKNOWN);
      tool_change(extruder);
    }
  }

  inline void reset_nozzle_offsets() {
    constexpr float tmp[XYZ][HOTENDS] = { HOTEND_OFFSET_X, HOTEND_OFFSET_Y, HOTEND_OFFSET_Z };
    LOOP_XYZ(i) HOTEND_LOOP() hotend_offset[i][e] = tmp[i][e];
  }

  inline void normalize_hotend_offsets() {
    for (uint8_t e = 1; e < HOTENDS; e++) {
      hotend_offset[X_AXIS][e] -= hotend_offset[X_AXIS][0];
      hotend_offset[Y_AXIS][e] -= hotend_offset[Y_AXIS][0];
      hotend_offset[Z_AXIS][e] -= hotend_offset[Z_AXIS][0];
    }
    hotend_offset[X_AXIS][0] = 0;
    hotend_offset[Y_AXIS][0] = 0;
    hotend_offset[Z_AXIS][0] = 0;
  }

  //
  // This function requires normalize_hotend_offsets() to be called
  //
  inline void report_hotend_offsets() {
    for (uint8_t e = 1; e < HOTENDS; e++) {
      SERIAL_ECHOPAIR("T", int(e));
      SERIAL_ECHOLNPGM(" Hotend Offset:");
      SERIAL_ECHOLNPAIR("  X: ", hotend_offset[X_AXIS][e]);
      SERIAL_ECHOLNPAIR("  Y: ", hotend_offset[Y_AXIS][e]);
      SERIAL_ECHOLNPAIR("  Z: ", hotend_offset[Z_AXIS][e]);
      SERIAL_EOL();
    }
  }

#endif // HOTENDS > 1

/**
 * Move along axis until the probe is triggered. Move toolhead to its starting
 * point and return the measured value.
 *
 *   axis               in     - Axis along which the measurement will take place
 *   dir                in     - Direction along that axis (-1 or 1)
 *   stop_state         in     - Move until probe pin becomes this value
 *   backlash_ptr       in/out - When not NULL, measure and record axis backlash
 *   uncertainty        in     - If uncertainty is CALIBRATION_MEASUREMENT_UNKNOWN, do a fast probe.
 */
inline float measure(const AxisEnum axis, const int dir, const bool stop_state, float * const backlash_ptr, const float uncertainty) {
  const bool fast = uncertainty == CALIBRATION_MEASUREMENT_UNKNOWN;

  // Save position
  set_destination_from_current();
  const float start_pos = destination[axis];
  const float measured_pos = measuring_movement(axis, dir, stop_state, fast);
  // Measure backlash
  if (backlash_ptr && !fast) {
    const float release_pos = measuring_movement(axis, -dir, !stop_state, fast);
    *backlash_ptr = ABS(release_pos - measured_pos);
  }
  // Return to starting position
  destination[axis] = start_pos;
  do_blocking_move_to(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], MMM_TO_MMS(CALIBRATION_FEEDRATE_TRAVEL));
  return measured_pos;
}

/**
 * Probe one side of the calibration cube
 *
 *   m                  in/out - Measurement record, m.cube_center and m.cube_side will be updated.
 *   uncertainty        in     - How far away from the cube to begin probing
 *   side               in     - Side of probe where probe will occur
 *   probe_top_at_edge  in     - When probing sides, probe top of cube nearest edge
 *                               to find out height of edge
 */
inline void probe_cube_side(measurements_t &m, const float uncertainty, const side_t side, const bool probe_top_at_edge=false) {
  AxisEnum axis;
  float dir;

  park_above_cube(m, uncertainty);

  switch(side) {
    case TOP: {
      const float measurement = measure(Z_AXIS, -1, true, &m.backlash[TOP], uncertainty);
      m.cube_center[Z_AXIS] = measurement - m.dimensions[Z_AXIS] / 2;
      m.cube_side[TOP] = measurement;
      return;
    }
    case RIGHT: axis = X_AXIS; dir = -1; break;
    case FRONT: axis = Y_AXIS; dir =  1; break;
    case LEFT:  axis = X_AXIS; dir =  1; break;
    case BACK:  axis = Y_AXIS; dir = -1; break;
    default:
      return;
  }

  if (probe_top_at_edge) {
    // Probe top nearest the side we are probing
    move_to(axis, m.cube_center[axis] + (-dir) * (m.dimensions[axis] / 2 - m.nozzle_outer_dimension[axis]));
    m.cube_side[TOP] = measure(Z_AXIS, -1, true, &m.backlash[TOP], uncertainty);
    m.cube_center[Z_AXIS] = m.cube_side[TOP] - m.dimensions[Z_AXIS] / 2;
  }

  // Move to safe distance to the side of the cube
  move_to(axis, m.cube_center[axis] + (-dir) * (m.dimensions[axis] / 2 + m.nozzle_outer_dimension[axis] / 2 + uncertainty));

  // Plunge below the side of the cube and measure
  move_to(Z_AXIS, m.cube_side[TOP] - CALIBRATION_NOZZLE_TIP_HEIGHT * 0.7);
  const float measurement = measure(axis, dir, true, &m.backlash[side], uncertainty);
  m.cube_center[axis] = measurement + dir * (m.dimensions[axis] / 2 + m.nozzle_outer_dimension[axis] / 2);
  m.cube_side[side] = measurement;
}

/**
 * Probe all sides of the calibration cube
 *
 *   m                  in/out - Measurement record: center, backlash and error values be updated.
 *   uncertainty        in     - How far away from the cube to begin probing
 */
inline void probe_cube(measurements_t &m, const float uncertainty) {
  TemporaryEndstopsState s(false);

  #ifdef CALIBRATION_CUBE_PROBE_AT_TOP_EDGES
    constexpr bool probe_top_at_edge = true;
  #else
    /* Probing at the exact center only works if the center is flat. Probing on a washer
     * or bolt will require probing the top near the side edges, away from the center.
     */
    constexpr bool probe_top_at_edge = false;
    probe_cube_side(m, uncertainty, TOP);
  #endif

  #ifdef CALIBRATION_CUBE_MEASURE_RIGHT
    probe_cube_side(m, uncertainty, RIGHT, probe_top_at_edge);
  #endif
  #ifdef CALIBRATION_CUBE_MEASURE_FRONT
    probe_cube_side(m, uncertainty, FRONT, probe_top_at_edge);
  #endif
  #ifdef CALIBRATION_CUBE_MEASURE_LEFT
    probe_cube_side(m, uncertainty, LEFT,  probe_top_at_edge);
  #endif
  #ifdef CALIBRATION_CUBE_MEASURE_BACK
    probe_cube_side(m, uncertainty, BACK,  probe_top_at_edge);
  #endif

  /* Compute the measured center of the cube. */
  #if HAS_X_CENTER
    m.cube_center[X_AXIS] = (m.cube_side[LEFT] + m.cube_side[RIGHT]) / 2;
  #endif
  #if HAS_Y_CENTER
    m.cube_center[Y_AXIS] = (m.cube_side[FRONT] + m.cube_side[BACK]) / 2;
  #endif

  /* Compute the outside diameter of the nozzle at the height
   * at which it makes contact with the cube */
  #if HAS_X_CENTER
    m.nozzle_outer_dimension[X_AXIS] = m.cube_side[RIGHT] - m.cube_side[LEFT] - m.dimensions[X_AXIS];
  #endif
  #if HAS_Y_CENTER
    m.nozzle_outer_dimension[Y_AXIS] = m.cube_side[BACK]  - m.cube_side[FRONT] - m.dimensions[Y_AXIS];
  #endif

  park_above_cube(m, uncertainty);

  /* The positional error is the difference between the known cube
   * location and the measured cube location */
  m.pos_error[X_AXIS] =
  #if HAS_X_CENTER
    m.true_center[X_AXIS] - m.cube_center[X_AXIS];
  #else
    0;
  #endif
  m.pos_error[Y_AXIS] =
  #if HAS_Y_CENTER
    m.true_center[Y_AXIS] - m.cube_center[Y_AXIS];
  #else
    0;
  #endif
  m.pos_error[Z_AXIS] = m.true_center[Z_AXIS] - m.cube_center[Z_AXIS];
}

inline void report_measured_faces(const measurements_t &m) {
  SERIAL_ECHOLNPGM("Cube Sides:");
  SERIAL_ECHOLNPAIR("  Top: ", m.cube_side[TOP]);
  #if ENABLED(CALIBRATION_CUBE_MEASURE_LEFT)
    SERIAL_ECHOLNPAIR("  Left: ", m.cube_side[LEFT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_RIGHT)
    SERIAL_ECHOLNPAIR("  Right: ", m.cube_side[RIGHT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_FRONT)
    SERIAL_ECHOLNPAIR("  Front: ", m.cube_side[FRONT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_BACK)
    SERIAL_ECHOLNPAIR("  Back: ", m.cube_side[BACK]);
  #endif
  SERIAL_EOL();
}

inline void report_measured_center(const measurements_t &m) {
  SERIAL_ECHOLNPGM("Cube Center:");
  #if HAS_X_CENTER
    SERIAL_ECHOLNPAIR(" X", m.cube_center[X_AXIS]);
  #endif
  #if HAS_Y_CENTER
    SERIAL_ECHOLNPAIR(" Y", m.cube_center[Y_AXIS]);
  #endif
  SERIAL_ECHOLNPAIR(" Z", m.cube_center[Z_AXIS]);
  SERIAL_EOL();
}

inline void report_measured_backlash(const measurements_t &m) {
  SERIAL_ECHOLNPGM("Backlash:");
  #if ENABLED(CALIBRATION_CUBE_MEASURE_LEFT)
    SERIAL_ECHOLNPAIR("  Left: ", m.backlash[LEFT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_RIGHT)
    SERIAL_ECHOLNPAIR("  Right: ", m.backlash[RIGHT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_FRONT)
    SERIAL_ECHOLNPAIR("  Front: ", m.backlash[FRONT]);
  #endif
  #if ENABLED(CALIBRATION_CUBE_MEASURE_BACK)
    SERIAL_ECHOLNPAIR("  Back: ", m.backlash[BACK]);
  #endif
  SERIAL_ECHOLNPAIR("  Top: ", m.backlash[TOP]);
  SERIAL_EOL();
}

inline void report_measured_positional_error(const measurements_t &m) {
  SERIAL_CHAR('T');
  SERIAL_ECHO(int(active_extruder));
  SERIAL_ECHOLNPGM(" Positional Error:");
  #if HAS_X_CENTER
    SERIAL_ECHOLNPAIR(" X", m.pos_error[X_AXIS]);
  #endif
  #if HAS_Y_CENTER
    SERIAL_ECHOLNPAIR(" Y", m.pos_error[Y_AXIS]);
  #endif
  SERIAL_ECHOLNPAIR(" Z", m.pos_error[Z_AXIS]);
  SERIAL_EOL();
}

/**
 * Probe around the calibration cube to measure backlash
 *
 *   m              in/out - Measurement record, updated with new readings
 *   uncertainty    in     - How far away from the cube to begin probing
 */
inline void calibrate_backlash(measurements_t &m, const float uncertainty) {
  // Backlash compensation should be off while measuring backlash
  TemporaryBacklashCompensation s(false);

  ui.set_status_P(PSTR("Measuring backlash"));
  probe_cube(m, uncertainty);

  #if ENABLED(BACKLASH_GCODE)
    #if HAS_X_CENTER
      backlash_distance_mm[X_AXIS] = (m.backlash[LEFT] + m.backlash[RIGHT]) / 2;
    #elif ENABLED(CALIBRATION_CUBE_MEASURE_LEFT)
      backlash_distance_mm[X_AXIS] = m.backlash[LEFT];
    #elif ENABLED(CALIBRATION_CUBE_MEASURE_RIGHT)
      backlash_distance_mm[X_AXIS] = m.backlash[RIGHT];
    #endif

    #if HAS_Y_CENTER
      backlash_distance_mm[Y_AXIS] = (m.backlash[FRONT] + m.backlash[BACK]) / 2;
    #elif ENABLED(CALIBRATION_CUBE_MEASURE_FRONT)
      backlash_distance_mm[Y_AXIS] = m.backlash[FRONT];
    #elif ENABLED(CALIBRATION_CUBE_MEASURE_BACK)
      backlash_distance_mm[Y_AXIS] = m.backlash[BACK];
    #endif

    backlash_distance_mm[Z_AXIS] = m.backlash[TOP];
  #endif

  #if ENABLED(BACKLASH_GCODE)
    // Turn on backlash compensation and move in all
    // directions to take up any backlash
    TemporaryBacklashCompensation s(true);
    move_to(
      X_AXIS, current_position[X_AXIS] + 3,
      Y_AXIS, current_position[Y_AXIS] + 3,
      Z_AXIS, current_position[Z_AXIS] + 3
    );
    move_to(
      X_AXIS, current_position[X_AXIS] - 3,
      Y_AXIS, current_position[Y_AXIS] - 3,
      Z_AXIS, current_position[Z_AXIS] - 3
    );
  #endif
}

inline void update_measurements(measurements_t &m, const AxisEnum axis) {
  current_position[axis] += m.pos_error[axis];
  m.cube_center[axis] = m.true_center[axis];
  m.pos_error[axis] = 0;
}

/**
 * Probe around the calibration cube. Adjust the position and toolhead offset
 * using the deviation from the known position of the calibration cube.
 *
 *   m              in/out - Measurement record, updated with new readings
 *   uncertainty    in     - How far away from the cube to begin probing
 *   extruder       in     - What extruder to probe
 *
 * Prerequisites:
 *    - Call calibrate_backlash() beforehand for best accuracy
 */
inline void calibrate_toolhead(measurements_t &m, const float uncertainty, const uint8_t extruder) {
  TemporaryBacklashCompensation s(true);

  const bool fast = uncertainty == CALIBRATION_MEASUREMENT_UNKNOWN;
  ui.set_status_P(fast ? PSTR("Finding calibration cube") : PSTR("Centering nozzle"));

  #if HOTENDS > 1
    set_nozzle(m, extruder);
  #endif

  probe_cube(m, uncertainty);
  if (!fast) {
    #if ENABLED(BACKLASH_GCODE) && ENABLED(CALIBRATION_MEASUREMENT_DEBUG)
      SERIAL_ECHOPAIR("Backlash (T", int(extruder));
      SERIAL_ECHOLNPGM("):");
      report_measured_backlash(m);
    #endif
  }

  /* Adjust the hotend offset */
  #if HOTENDS > 1
    #if HAS_X_CENTER
      hotend_offset[X_AXIS][extruder] += m.pos_error[X_AXIS];
    #endif
    #if HAS_Y_CENTER
      hotend_offset[Y_AXIS][extruder] += m.pos_error[Y_AXIS];
    #endif
    hotend_offset[Z_AXIS][extruder] += m.pos_error[Z_AXIS];

    normalize_hotend_offsets();
    #ifdef CALIBRATION_MEASUREMENT_DEBUG
      report_hotend_offsets();
    #endif
  #endif

  // Correct for positional error, so the cube
  // is at the known actual spot
  planner.synchronize();
  #if HAS_X_CENTER
    update_measurements(m, X_AXIS);
  #endif
  #if HAS_Y_CENTER
    update_measurements(m, Y_AXIS);
  #endif
  update_measurements(m, Z_AXIS);

  sync_plan_position();
}

/**
 * Probe around the calibration cube for all toolheads, adjusting the coordinate
 * system for the first nozzle and the nozzle offset for subsequent nozzles.
 *
 *   m              in/out - Measurement record, updated with new readings
 *   uncertainty    in     - How far away from the cube to begin probing
 */
inline void calibrate_all_toolheads(measurements_t &m, const float uncertainty) {
  TemporaryBacklashCompensation s(true);

  HOTEND_LOOP() calibrate_toolhead(m, uncertainty, e);

  #if HOTENDS > 1
    normalize_hotend_offsets();
    set_nozzle(m, 0);
    #ifdef CALIBRATION_MEASUREMENT_DEBUG
      report_hotend_offsets();
    #endif
  #endif
}

inline void report_measured_nozzle_dimensions(const measurements_t &m) {
  SERIAL_ECHOLNPGM("Nozzle Tip Outer Dimensions:");
  #if HAS_X_CENTER
    SERIAL_ECHOLNPAIR(" X", m.nozzle_outer_dimension[X_AXIS]);
  #endif
  #if HAS_Y_CENTER
    SERIAL_ECHOLNPAIR(" Y", m.nozzle_outer_dimension[Y_AXIS]);
  #endif
  SERIAL_EOL();
}

inline bool read_probe_value() {
  #if ENABLED(Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN)
    return (READ(Z_MIN_PIN) != Z_MIN_ENDSTOP_INVERTING);
  #else
    return (READ(Z_MIN_PROBE_PIN != Z_MIN_PROBE_ENDSTOP_INVERTING);
  #endif
}

/**
 * Move along axis in the specified dir until the probe value becomes stop_state,
 * then return the axis value.
 *
 *   axis         in - Axis along which the measurement will take place
 *   dir          in - Direction along that axis (-1 or 1)
 *   stop_state   in - Move until probe pin becomes this value
 *   fast         in - Fast vs. precise measurement
 */
float measuring_movement(const AxisEnum axis, const int dir, const bool stop_state, const bool fast) {
  const float step  =            fast ? 0.25                      : CALIBRATION_MEASUREMENT_RESOLUTION;
  const float mms   = MMM_TO_MMS(fast ? CALIBRATION_FEEDRATE_FAST : CALIBRATION_FEEDRATE_SLOW);
  const float limit =            fast ? 50                        : 5;

  set_destination_from_current();
  for (float travel = 0; travel < limit; travel += step) {
    destination[axis] += dir * step;
    do_blocking_move_to(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], mms);
    planner.synchronize();
    if (read_probe_value() == stop_state)
      break;
  }
  return destination[axis];
}

inline void say_calibration_done() { ui.set_status_P(PSTR("Calibration done.")); }

/**
 * Perform a full auto-calibration routine:
 *
 *   1) For each nozzle, touches top and sides of cube to determine cube position and
 *      nozzle offsets. Do a fast but rough search over a wider area.
 *   2) With the first nozzle, touch top and sides of cube to determine backlash values
 *      for all axis (if BACKLASH_GCODE is enabled)
 *   3) For each nozzle, touches top and sides of cube slowly to determine precise
 *      position of cube. Adjust coordinate system and nozzle offsets so probed cube
 *      location corresponds to known cube location with a high degree of precision.
 */
inline void calibrate_all() {
  measurements_t m;

  #if HOTENDS > 1
    reset_nozzle_offsets();
  #endif

  TemporaryBacklashCompensation s(true);

  /* Do a fast and rough calibration of the toolheads */
  ui.set_status_P(PSTR("Finding cube"));
  calibrate_all_toolheads(m, CALIBRATION_MEASUREMENT_UNKNOWN);

  #if ENABLED(BACKLASH_GCODE)
    calibrate_backlash(m, CALIBRATION_MEASUREMENT_UNCERTAIN);
    #ifdef CALIBRATION_MEASUREMENT_DEBUG
      SERIAL_ECHOLNPGM("Backlash before correction:");
      report_measured_backlash(m);
    #endif
  #endif

  /* Cycle the toolheads so the servos settle into their "natural" positions */
  #if HOTENDS > 1
    HOTEND_LOOP() set_nozzle(m, e);
  #endif

  /* Do a slow and precise calibration of the toolheads */
  calibrate_all_toolheads(m, CALIBRATION_MEASUREMENT_UNCERTAIN);

  say_calibration_done();
  move_to(X_AXIS, 150); // Park nozzle away from cube
}

/**
 * G425: Perform calibration with calibration cube.
 *
 *   B           - Perform calibration of backlash only.
 *   T<extruder> - Perform calibration of toolhead only.
 *   V           - Probe cube and print position, error, backlash and hotend offset.
 *   U           - Uncertainty, how far to start probe away from the cube (mm)
 *
 *   no args     - Perform entire calibration sequence (backlash + position on all toolheads)
 */
void GcodeSuite::G425() {
  if (axis_unhomed_error()) return;

  measurements_t m;

  float uncertainty = parser.seenval('U') ? parser.value_float() : CALIBRATION_MEASUREMENT_UNCERTAIN;

  if (parser.seen('V')) {
    ui.set_status_P(PSTR("Measuring nozzle center"));
    probe_cube(m, uncertainty);
    SERIAL_EOL();
    report_measured_faces(m);
    report_measured_center(m);
    report_measured_backlash(m);
    report_measured_nozzle_dimensions(m);
    report_measured_positional_error(m);
    #if HOTENDS > 1
      normalize_hotend_offsets();
      report_hotend_offsets();
    #endif
    ui.set_status_P(PSTR("Measurements finished."));
  }
  else if (parser.seen('B')) {
    calibrate_backlash(m, uncertainty);
    say_calibration_done();
  }
  else if (parser.seen('T')) {
    calibrate_toolhead(m, uncertainty, parser.has_value() ? parser.value_int() : active_extruder);
    say_calibration_done();
  }
  else
    calibrate_all();
}

#endif // CALIBRATION_GCODE