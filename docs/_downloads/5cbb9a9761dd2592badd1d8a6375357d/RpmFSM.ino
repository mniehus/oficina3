
void RPM_FSM_update() {

  /*
  RPM_FSM_update()
  ----------------------------------------------------------------------------
  TASK
    Compute motor RPM from an optical encoder with a simple ISR pulse counter.
    Supports two modes:
      (A) Free-run sampling with fixed period.
      (B) PWM-coupled sampling: count pulses exactly between consecutive
          PWM update events (constant duty window).

  INPUTS (reads)
    - volatile unsigned long sig_encoder_pulses  // ISR-managed pulse counter
    - unsigned long sig_pwm_update_counter       // incremented by PWM FSM on each output write

  OUTPUTS (writes)
    - float sig_rpm                              // latest RPM estimate

  HOW TO USE
    1) Declare the shared globals and the ISR above.
    2) Call RPM_FSM_update() in loop().
    3) For PWM-coupled mode, add the one-liner in the PWM FSM's UPDATE case.

  HOW TO TEST
    - Free-run: Set MODE_COUPLED_TO_PWM=false. Spin motor; watch sig_rpm.
    - PWM-coupled: Set MODE_COUPLED_TO_PWM=true. Ensure PWM FSM increments
      sig_pwm_update_counter on each analogWrite. You should see clean stepwise
      RPM aligned to PWM intervals.

  HOW TO EXPAND
    - Add moving-average/IIR filtering of sig_rpm.
    - Add timeout detection to force RPM=0 if no pulses for T ms.
    - Add optional “min duty deadband” awareness to treat tiny speeds as zero.

  NOTE
    - Uses Arduino types: bool, char, byte, int, long, float, unsigned long.
    - Two measurement strategies are available:
        * DELTA method (non-destructive): uses count differences.
        * RESET method (destructive): zeroes the ISR counter at the window start.
*/

  // ------------------ Local configuration constants -----------------------
  // Encoder wiring & semantics
  const byte         ENCODER_PIN          = 2;        // must be interrupt-capable
  const bool         ENCODER_PULLUP       = true;     // use INPUT_PULLUP if true
  const byte         PULSES_PER_REV       = 2;        // 2 blades -> 2 pulses per revolution
  const byte         ISR_EDGE_SELECT      = 0;        // 0:RISING, 1:FALLING, 2:CHANGE

  // Sampling mode
  const bool         MODE_COUPLED_TO_PWM  = true;     // true: measure between PWM updates
  const unsigned int FREE_RUN_PERIOD_MS   = 100;      // used only if not coupled

  // Counter handling
  const bool         USE_RESET_METHOD     = false;    // true: reset counter each window; false: delta
  const unsigned int MIN_DT_MS            = 5;        // guard minimum window to avoid div-by-zero

  // Optional smoothing (disabled by default)
  const bool         APPLY_IIR_FILTER     = false;
  const float        IIR_ALPHA            = 0.3f;     // y = alpha*y_new + (1-alpha)*y_prev

  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT            = 0;
  const byte IDLE            = 1;   // waiting for next window trigger (time or PWM)
  const byte START_WINDOW    = 2;   // mark window start (count & time)
  const byte END_WINDOW      = 3;   // mark window end (count & time)
  const byte COMPUTE         = 4;   // compute RPM, publish

  // ------------------ Local static variables ------------------------------
  static byte          state = INIT;

  // Window book-keeping
  static unsigned long startCount = 0;
  static unsigned long startMs    = 0;
  static unsigned long endCount   = 0;
  static unsigned long endMs      = 0;

  // For PWM-coupled triggering
  static unsigned long pwmEdgeSeen = 0;     // last observed sig_pwm_update_counter

  // For free-run timing
  static unsigned long tickMs      = 0;

  // For IIR
  static float         rpmPrev      = 0.0f;

  // ------------------ Helper functions---------------------
  auto snapshotPulses = []() -> unsigned long {
    noInterrupts();
    unsigned long c = sig_encoder_pulses;
    interrupts();
    return c;
  };

  auto resetPulses = []() {
    noInterrupts();
    sig_encoder_pulses = 0;
    interrupts();
  };

  // ------------------ Time base ----------------------------
  unsigned long nowMs = millis();

  // ------------------ State machine ------------------------
  switch (state) {

    case INIT: {
      // Configure encoder pin & interrupt
      pinMode(ENCODER_PIN, ENCODER_PULLUP ? INPUT_PULLUP : INPUT);
      // Detach first to be safe, then attach with selected edge
      detachInterrupt(digitalPinToInterrupt(ENCODER_PIN));
      if (ISR_EDGE_SELECT == 0) {
        attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderISR, RISING);
      } else if (ISR_EDGE_SELECT == 1) {
        attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderISR, FALLING);
      } else {
        attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderISR, CHANGE);
      }

      // Initialize counters/time
      pwmEdgeSeen = sig_pwm_update_counter;
      startCount  = snapshotPulses();
      startMs     = nowMs;
      tickMs      = nowMs;
      sig_rpm     = 0.0f;
      rpmPrev     = 0.0f;

      // Transition
      state = MODE_COUPLED_TO_PWM ? IDLE : IDLE;
      break;
    }

    case IDLE: {
      // Activities: none

      // Transitions
      if (MODE_COUPLED_TO_PWM) {
        // Wait for next PWM update to START the window
        if (sig_pwm_update_counter != pwmEdgeSeen) {
          state = START_WINDOW;
        }
      } else {
        // Free-run: start a new window every FREE_RUN_PERIOD_MS
        if ((unsigned long)(nowMs - tickMs) >= (unsigned long)FREE_RUN_PERIOD_MS) {
          state = START_WINDOW;
        }
      }
      break;
    }

    case START_WINDOW: {
      // Activities: mark window start (count & time)
      if (USE_RESET_METHOD) {
        resetPulses();
        startCount = 0;
      } else {
        startCount = snapshotPulses();
      }
      startMs = nowMs;

      // Transitions to END_WINDOW criteria
      if (MODE_COUPLED_TO_PWM) {
        // Consume exactly one PWM edge and wait for the next one to close the window
        pwmEdgeSeen = sig_pwm_update_counter; // latch the edge we used to start
        state = END_WINDOW;
      } else {
        // Free-run: close after FREE_RUN_PERIOD_MS
        tickMs = nowMs;
        state = END_WINDOW;
      }
      break;
    }

    case END_WINDOW: {
      // Activities: check if the window should close now
      bool shouldClose = false;

      if (MODE_COUPLED_TO_PWM) {
        // Close on the next PWM edge
        if (sig_pwm_update_counter != pwmEdgeSeen) {
          shouldClose = true;
          pwmEdgeSeen = sig_pwm_update_counter; // latch closing edge
        }
      } else {
        // Close when period elapsed
        if ((unsigned long)(nowMs - startMs) >= (unsigned long)FREE_RUN_PERIOD_MS) {
          shouldClose = true;
        }
      }

      if (!shouldClose) {
        // Stay in END_WINDOW until the condition is met
        break;
      }

      // Capture end snapshot
      if (USE_RESET_METHOD) {
        endCount = snapshotPulses();  // pulses since reset at START_WINDOW
        resetPulses();                // prep for next window
      } else {
        endCount = snapshotPulses();
      }
      endMs = nowMs;

      // Transition
      state = COMPUTE;
      break;
    }

    case COMPUTE: {
      // Activities: compute RPM
      unsigned long dt_ms = (unsigned long)(endMs - startMs);
      if (dt_ms < (unsigned long)MIN_DT_MS || PULSES_PER_REV == 0) {
        // Too short / invalid; keep previous RPM or set zero
        // Here we choose to keep previous filtered value (rpmPrev)
        sig_rpm = rpmPrev;
      } else {
        unsigned long pulseDelta;
        if (USE_RESET_METHOD) {
          pulseDelta = endCount;                  // since reset
        } else {
          pulseDelta = (unsigned long)(endCount - startCount); // wrap-safe unsigned math
        }

        float dt_s   = (float)dt_ms / 1000.0f;
        float revs   = (float)pulseDelta / (float)PULSES_PER_REV;
        float rpmNew = (revs / dt_s) * 60.0f;

        if (APPLY_IIR_FILTER) {
          sig_rpm = IIR_ALPHA * rpmNew + (1.0f - IIR_ALPHA) * rpmPrev;
        } else {
          sig_rpm = rpmNew;
        }
        rpmPrev = sig_rpm;
      }

      // Transition: go wait for the next window trigger
      state = IDLE;
      break;
    }

    default: {
      state = INIT;
      break;
    }
  } // switch
}
