void PWMOutputFSM_update() {

  /*
  PWMOutputFSM_update()
  ----------------------------------------------------------------------------
  TASK
    Drive a PWM-capable pin from the shared signal 'sig_waveform_value' at a
    fixed update period, with optional clamp and inversion. Writes the applied
    duty to the shared 'sig_pwm_applied'.

  INPUT PARAMETERS
    - Reads global:  byte sig_waveform_value  // 0..255 desired duty (already generated elsewhere)

  OUTPUT PARAMETERS
    - Writes global: byte sig_pwm_applied     // 0..255 actually written to the pin
    - Side effect:   analogWrite(PWM_PIN, ...)

  HOW TO USE
    1) Ensure 'sig_waveform_value' exists .
    2) Call PWMOutputFSM_update() in loop().
    3) Set the local constants below (PWM pin, update period, clamps, inversion).

  HOW TO TEST
    - Connect PWM_PIN to an LED+resistor (and/or oscilloscope and/or your H-bridge enable).
    - In loop(), also run the Waveform FSM to generate values.
    - Optionally print 'sig_pwm_applied' at ~10–50 Hz to confirm updates.

  HOW TO EXPAND
    - Add slew-rate limiting (max Δduty per update) to soften step changes.
    - Add a deadband/minimum duty to overcome static friction (already included).
    - Add a runtime “enable” boolean or an emergency stop latch.
    - Add mapping from waveform [0..255] to a narrower window [dutyMin..dutyMax].

  NOTE
    - Uses Arduino-idiomatic types: bool, char, byte, int, long, float, unsigned long.
*/

  // ------------------ Local configuration constants -----------------------
  const byte         PWM_PIN             = 9;      // PWM-capable pin to H-bridge enable
  const unsigned int UPDATE_PERIOD_MS    = 200;     // how often to refresh analogWrite
  const byte         MAX_DUTY            = 255;    // upper clamp for safety (0..255)

  // Optional minimum duty to overcome static friction (applied only when nonzero)
  const bool         APPLY_MIN_WHEN_NONZERO = false;
  const byte         MIN_DUTY            = 25;     // used only if APPLY_MIN_WHEN_NONZERO = true

  // If your hardware input is active-low (rare for enable), set true to invert PWM
  const bool         OUTPUT_INVERT       = false;  // false: write duty; true: write (255 - duty)


  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT      = 0;
  const byte WAIT_TICK = 1;
  const byte UPDATE    = 2;

  // ------------------ Local static state variables -------------------------
  static byte          state = INIT;
  static unsigned long lastTickMs = 0;

  // ------------------ Time base -------------------------
  const unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      pinMode(PWM_PIN, OUTPUT);

      // Write "zero effective power" respecting inversion
      byte zeroApplied = OUTPUT_INVERT ? (byte)255 : (byte)0;
      analogWrite(PWM_PIN, zeroApplied);
      sig_pwm_applied = zeroApplied;

      lastTickMs = nowMs;

      // Transition
      state = WAIT_TICK;
      break;
    }

    case WAIT_TICK: {
      // Activities: none (idle until next refresh)

      // Transition
      if ((unsigned long)(nowMs - lastTickMs) >= (unsigned long)UPDATE_PERIOD_MS) {
        state = UPDATE;
      }
      break;
    }

    case UPDATE: {
      // Activities: take desired duty, clamp/shape, write to pin
      byte duty = sig_waveform_value;        // desired (0..255) from waveform FSM

      // Safety clamp
      if (duty > MAX_DUTY) duty = MAX_DUTY;

      // Optional minimum to overcome static friction (only if nonzero request)
      if (APPLY_MIN_WHEN_NONZERO && duty > 0 && duty < MIN_DUTY) {
        duty = MIN_DUTY;
      }

      // Invert if needed
      byte applied = OUTPUT_INVERT ? (byte)(255 - duty) : duty;

      analogWrite(PWM_PIN, applied);

      // after analogWrite(PWM_PIN, applied);
      sig_pwm_update_counter++;   // <-- exposes PWM window edges to RPM FSM

      sig_pwm_applied = applied;

      lastTickMs = nowMs;

      // Transition
      state = WAIT_TICK;
      break;
    }

    default: {
      // Safety net
      state = INIT;
      break;
    }
  } // switch
}
