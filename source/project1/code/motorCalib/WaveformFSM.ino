// ---------------------- Waveform Generator FSM -----------------------------
void WaveformFSM_update(){
  /*
  WaveformFSM_update()
  ---------------------------------------------------------------------------
  TASK
    Time-discrete waveform generator that produces a value (0..255) into the
    shared global signal 'sig_waveform_value', at a configurable sample period.
    Supports: SINE, SQUARE, SAWTOOTH (rising), and SINGLE_RAMP (0→A once).

  INPUT PARAMETERS
    None (by design). All configuration is defined as local 'const' inside
    this function for simplicity and educational purposes.

  OUTPUT PARAMETERS
    - Writes to the global variable:
        byte sig_waveform_value;   // 0..255 latest waveform sample

  HOW TO USE
    1) Make sure 'sig_waveform_value' is declared globally (see below).
    2) Call WaveformFSM_update() once every loop() iteration (non-blocking).
    3) Other state machines can read 'sig_waveform_value' to drive PWM,
       log to Serial, etc.

  HOW TO TEST (quick manual test)
    - Temporarily add a Serial.println(sig_waveform_value) at ~10–50 Hz in loop
      OR map it to analogWrite on a spare LED/PWM pin to observe changes.
    - Try different WAVEFORM_SELECT, AMPLITUDE, SAMPLE_PERIOD_MS, and
      MODULATION_PERIOD_MS constants inside the FSM.
    - For SINGLE_RAMP, set SINGLE_RAMP_REPEAT to false to run exactly once.

  HOW TO EXPAND
    - Add TRIANGLE, EXP, or custom tables by extending the COMPUTE case.
    - Add an 'OFFSET' constant to shift the waveform (e.g., center around 127).
    - Expose a tiny setter to switch waveforms at runtime (keeping signals as
      globals only if you want external control), or add a Serial command parser.
    - Add a 'PHASE_DEG' constant to apply a fixed phase offset to SINE/SQUARE.

  NOTE
    Uses Arduino-idiomatic types: bool, char, byte, int, long, float, unsigned long.
*/
  // ------------------ Local configuration constants -----------------------
  // Wave types
  const byte WF_SINE         = 0;
  const byte WF_SQUARE       = 1;
  const byte WF_SAWTOOTH     = 2;  // rising saw
  const byte WF_RAMP_SINGLE  = 3;  // 0→A once, then hold (or repeat if enabled)

  // Select active waveform here
  const byte WAVEFORM_SELECT = WF_SAWTOOTH;

  // Sampling & modulation
  const unsigned int  SAMPLE_PERIOD_MS      = 50;     // time between output samples
  const unsigned long MODULATION_PERIOD_MS  = 2000UL; // full wave period (except single ramp)
  const byte          AMPLITUDE             = 220;    // peak value (0..255). Output is in [0..AMPLITUDE]

  // SINGLE_RAMP behavior
  const bool          SINGLE_RAMP_REPEAT    = false;  // true: restart ramp after reaching AMPLITUDE

  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT        = 0;
  const byte WAIT_TICK   = 1;
  const byte COMPUTE     = 2;
  const byte HOLD        = 3;   // used when single-ramp completes and not repeating

  // ------------------ Local static state variables -------------------------
  static byte          state = INIT;
  static unsigned long lastTickMs = 0;       // scheduler tick
  static unsigned long periodT0Ms = 0;       // start time for periodic phase (sine/square/saw)
  static unsigned long rampT0Ms   = 0;       // start time for single ramp

  // ------------------ Time base -------------------------
  const unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      sig_waveform_value = 0;
      lastTickMs = nowMs;
      periodT0Ms = nowMs;
      rampT0Ms   = nowMs;

      // Transition
      state = WAIT_TICK;
      break;
    }

    case WAIT_TICK: {
      // Activities: none (idle until next sample is due)

      // Transition: wait for sample period to elapse
      if ((unsigned long)(nowMs - lastTickMs) >= (unsigned long)SAMPLE_PERIOD_MS) {
        state = COMPUTE;
      }
      break;
    }

    case COMPUTE: {
      // Activities: compute one sample into sig_waveform_value
      // Common helper: normalized phase in [0,1) for periodic waves
      float phase = 0.0f;
      if (MODULATION_PERIOD_MS > 0UL) {
        unsigned long elapsed = (unsigned long)(nowMs - periodT0Ms);
        unsigned long mod     = elapsed % MODULATION_PERIOD_MS;
        phase = (float)mod / (float)MODULATION_PERIOD_MS;  // [0..1)
      }

      byte outVal = 0;

      if (WAVEFORM_SELECT == 0 /*WF_SINE*/) {
        // y = 0.5*(1+sin(2π·phase)) * AMPLITUDE
        // Keep in [0..AMPLITUDE]
        //const float TWO_PI = 6.28318530718f;  //was already defined in my Arduino Uno R4 
        float y = 0.5f * (1.0f + sin(TWO_PI * phase));
        float yScaled = y * (float)AMPLITUDE;
        if (yScaled < 0.0f) yScaled = 0.0f;
        if (yScaled > 255.0f) yScaled = 255.0f;
        outVal = (byte)(yScaled + 0.5f);
      }
      else if (WAVEFORM_SELECT == 1 /*WF_SQUARE*/) {
        // 50% duty: low 0 for phase<0.5, high AMPLITUDE otherwise
        outVal = (phase < 0.5f) ? (byte)0 : AMPLITUDE;
      }
      else if (WAVEFORM_SELECT == 2 /*WF_SAWTOOTH*/) {
        // Rising saw: y = phase * AMPLITUDE
        float yScaled = phase * (float)AMPLITUDE;
        if (yScaled < 0.0f) yScaled = 0.0f;
        if (yScaled > 255.0f) yScaled = 255.0f;
        outVal = (byte)(yScaled + 0.5f);
      }
      else if (WAVEFORM_SELECT == 3 /*WF_RAMP_SINGLE*/) {
        // Single ramp 0→AMPLITUDE across MODULATION_PERIOD_MS, then hold or repeat
        unsigned long elapsed = (unsigned long)(nowMs - rampT0Ms);
        float ratio = 0.0f;
        if (MODULATION_PERIOD_MS > 0UL) {
          if (elapsed >= MODULATION_PERIOD_MS) {
            ratio = 1.0f;  // completed
          } else {
            ratio = (float)elapsed / (float)MODULATION_PERIOD_MS; // [0..1)
          }
        } else {
          ratio = 1.0f; // degenerate: instant jump to AMPLITUDE
        }
        float yScaled = ratio * (float)AMPLITUDE;
        if (yScaled < 0.0f) yScaled = 0.0f;
        if (yScaled > 255.0f) yScaled = 255.0f;
        outVal = (byte)(yScaled + 0.5f);
      }
      else {
        // Fallback safety: constant 0 if unknown selection
        outVal = 0;
      }

      sig_waveform_value = outVal;  // <-- single shared signal

      // Activities: update tick base
      lastTickMs = nowMs;

      // Transitions
      if (WAVEFORM_SELECT == 3 /*WF_RAMP_SINGLE*/) {
        // If ramp completed, either hold or restart depending on SINGLE_RAMP_REPEAT
        if (MODULATION_PERIOD_MS == 0UL) {
          // zero-period means instant completion
          if (SINGLE_RAMP_REPEAT) {
            rampT0Ms = nowMs;    // restart immediately
            state = WAIT_TICK;
          } else {
            state = HOLD;        // freeze at AMPLITUDE
          }
        } else {
          unsigned long elapsed = (unsigned long)(nowMs - rampT0Ms);
          if (elapsed >= MODULATION_PERIOD_MS) {
            if (SINGLE_RAMP_REPEAT) {
              rampT0Ms = nowMs;  // restart
              state = WAIT_TICK;
            } else {
              state = HOLD;      // freeze
            }
          } else {
            state = WAIT_TICK;   // keep ramping
          }
        }
      } else {
        // Periodic waves always continue sampling
        state = WAIT_TICK;
      }

      break;
    }

    case HOLD: {
      // Activities: keep the last value (typically AMPLITUDE for single ramp)
      // Transition: none (steady). For manual restart, set 'state = INIT' from outside or edit behavior.
      break;
    }

    default: {
      // Safety net: re-init
      state = INIT;
      break;
    }
  } // switch
}