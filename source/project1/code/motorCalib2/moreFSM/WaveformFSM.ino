
void WaveformFSM_update(){
  
  // Wave types
  const byte WF_SINE         = 0;
  const byte WF_SQUARE       = 1;
  const byte WF_SAWTOOTH     = 2;  // rising saw
  const byte WF_RAMP_SINGLE  = 3;  // 0→A once, then hold (or repeat if enabled)

  // Select active waveform here
  const byte WAVEFORM_SELECT = WF_SAWTOOTH;

  const unsigned int  SAMPLE_PERIOD_MS      = 50;     // time between output samples
  const unsigned long MODULATION_PERIOD_MS  = 2000;   // full wave period
  const byte          AMPLITUDE             = 220;    // peak value (0..255). Output is in [0..AMPLITUDE]
  const bool          SINGLE_RAMP_REPEAT    = false;  // true: restart ramp after reaching AMPLITUDE

  const byte INIT        = 0;
  const byte WAIT_TICK   = 1;
  const byte COMPUTE     = 2;
  const byte HOLD        = 3;   // used when single-ramp completes and not repeating

  static byte          state = INIT;
  static unsigned long lastTickMs = 0;       // scheduler tick
  static unsigned long periodT0Ms = 0;       // start time for periodic phase (sine/square/saw)
  static unsigned long rampT0Ms   = 0;       // start time for single ramp

  static byte outVal = 0;
  static float phase = 0.0;  // [0..1)

  const unsigned long nowMs = millis();

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
      if (nowMs - lastTickMs >= SAMPLE_PERIOD_MS) {
        state = COMPUTE;
      }
      break;
    }

    case COMPUTE: {
      // Activities: compute one sample into sig_waveform_value

      if (MODULATION_PERIOD_MS > 0) {
        unsigned long elapsed = nowMs - periodT0Ms;
        unsigned long mod     = elapsed % MODULATION_PERIOD_MS;
        phase = mod / MODULATION_PERIOD_MS;  
      }

      if (WAVEFORM_SELECT == 0 /*WF_SINE*/) {
        // y = 0.5*(1+sin(2π·phase)) * AMPLITUDE
        outVal = 0.5 * (1.0 + sin(6.28 * phase))* AMPLITUDE;
      }
      else if (WAVEFORM_SELECT == 1 /*WF_SQUARE*/) {
        // 50% duty: low 0 for phase<0.5, high AMPLITUDE otherwise
        outVal = phase < 0.5 ? 0 : AMPLITUDE;
      }
      else if (WAVEFORM_SELECT == 2 /*WF_SAWTOOTH*/) {
        // Rising saw: y = phase * AMPLITUDE
        outVal = phase * AMPLITUDE;
      }
      else if (WAVEFORM_SELECT == 3 /*WF_RAMP_SINGLE*/) {
        // Single ramp 0→AMPLITUDE across MODULATION_PERIOD_MS, then hold or repeat
        unsigned long elapsed = nowMs - rampT0Ms;
        float ratio = 0.0f;
        if (MODULATION_PERIOD_MS > 0) {
          if (elapsed >= MODULATION_PERIOD_MS) {
            ratio = 1.0f;  // completed
          } else {
            ratio = (float)elapsed / (float)MODULATION_PERIOD_MS; // [0..1)
          }
        } else {
          ratio = 1.0f; // degenerate: instant jump to AMPLITUDE
        }
        outVal = ratio *AMPLITUDE;
      }
      else {
        outVal = 0;
      }
      sig_waveform_value = constrain(outval,0,255);  // <-- single shared signal

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