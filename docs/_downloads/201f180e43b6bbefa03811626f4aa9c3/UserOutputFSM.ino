

void UserOutput_FSM_update() {
  /*
  UserOutput_FSM_update()
  ----------------------------------------------------------------------------
  TASK
    Drive three LEDs (R,G,B) on digital pins and a piezo buzzer that can be
    actuated either with tone() (frequency/duration) or by simple digital
    toggling (square wave) — all based on shared *global* command variables.

  INPUT PARAMETERS
    None (all configuration as local consts).

  OUTPUT PARAMETERS
    - Writes to LED pins via digitalWrite.
    - Starts/stops tone() on the buzzer pin or toggles it digitally.

  HOW TO USE
    - Other modules set:
        sig_led_r/g/b               (0/1)
        sig_buzzer_tone_enable      (bool)
        sig_buzzer_tone_freq_hz     (unsigned int)
        sig_buzzer_tone_ms          (unsigned int; 0 = continuous)
        sig_buzzer_toggle_enable    (bool)
        sig_buzzer_toggle_period_ms (unsigned int)
    - Call once per loop().

  HOW TO TEST
    - From the Serial input, send:  R=1  G=0  B=1  (as you prefer)
    - For buzzer: send commands to set freq and enable tone, or enable toggling.
*/
  // ------------------ Local configuration constants -----------------------
  const byte PIN_LED_R = 5;        // any digital pin
  const byte PIN_LED_G = 6;
  const byte PIN_LED_B = 7;

  const bool LED_ACTIVE_HIGH = true; // set false if wired to Vcc and sink to turn on

  const byte PIN_BUZZER = 8;       // choose a pin that supports tone() on your board

  const unsigned int UPDATE_PERIOD_MS = 10; // refresh cadence for outputs

  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT      = 0;
  const byte WAIT_TICK = 1;
  const byte APPLY     = 2;

  // ------------------ Local static variables ------------------------------
  static byte          state = INIT;
  static unsigned long lastTickMs = 0;

  // Buzzer bookkeeping
  static bool          tone_running = false;       // whether tone() is currently active (continuous)
  static unsigned long tone_end_ms  = 0;           // for one-shot tone duration tracking

  static byte          toggle_level = 0;           // current digital level when toggling
  static unsigned long toggle_last_ms = 0;

  // ------------------ Time base -------------------------
  unsigned long nowMs = millis();

  // ------------------ Helper Functions ---------------------------------------------
  auto writeLed = [&](byte pin, byte on) {
    byte level = LED_ACTIVE_HIGH ? (on ? HIGH : LOW) : (on ? LOW : HIGH);
    digitalWrite(pin, level);
  };

  // ------------------ State machine ---------------------------------------
  switch (state) {

    case INIT: {
      // LED pins
      pinMode(PIN_LED_R, OUTPUT);
      pinMode(PIN_LED_G, OUTPUT);
      pinMode(PIN_LED_B, OUTPUT);
      writeLed(PIN_LED_R, 0);
      writeLed(PIN_LED_G, 0);
      writeLed(PIN_LED_B, 0);

      // Buzzer pin
      pinMode(PIN_BUZZER, OUTPUT);
      digitalWrite(PIN_BUZZER, LOW);
      tone_running = false;
      tone_end_ms  = 0;
      toggle_level = 0;
      toggle_last_ms = nowMs;

      lastTickMs = nowMs;
      state = WAIT_TICK;
      break;
    }

    case WAIT_TICK: {
      if ((unsigned long)(nowMs - lastTickMs) >= UPDATE_PERIOD_MS) {
        state = APPLY;
      }
      break;
    }

    case APPLY: {
      // ---- LEDs ----
      writeLed(PIN_LED_R, sig_led_r ? 1 : 0);
      writeLed(PIN_LED_G, sig_led_g ? 1 : 0);
      writeLed(PIN_LED_B, sig_led_b ? 1 : 0);

      // ---- Buzzer priority: tone() overrides toggling ----
      if (sig_buzzer_tone_enable) {
        // Start/refresh tone if needed
        if (sig_buzzer_tone_ms > 0) {
          // One-shot tone: each call restarts duration, so only call when starting or changing
          if (!tone_running || nowMs >= tone_end_ms) {
            tone(PIN_BUZZER, sig_buzzer_tone_freq_hz, sig_buzzer_tone_ms);
            tone_running = false;                      // one-shots are not considered "running"
            tone_end_ms  = nowMs + (unsigned long)sig_buzzer_tone_ms;
          }
        } else {
          // Continuous tone until disabled
          if (!tone_running) {
            tone(PIN_BUZZER, sig_buzzer_tone_freq_hz);
            tone_running = true;
          } else {
            // If frequency changed meaningfully, restart
            // (tone() with same pin/freq is harmless but we avoid spamming)
            static unsigned int last_freq = 0;
            if (sig_buzzer_tone_freq_hz != last_freq) {
              tone(PIN_BUZZER, sig_buzzer_tone_freq_hz);
            }
            last_freq = sig_buzzer_tone_freq_hz;
          }
        }
      } else {
        // Ensure tone is off before toggling
        if (tone_running || tone_end_ms != 0) {
          noTone(PIN_BUZZER);
          tone_running = false;
          tone_end_ms  = 0;
        }

        // Optional digital toggling (square wave by software)
        if (sig_buzzer_toggle_enable && sig_buzzer_toggle_period_ms > 0) {
          if ((unsigned long)(nowMs - toggle_last_ms) >= sig_buzzer_toggle_period_ms) {
            toggle_last_ms = nowMs;
            toggle_level = toggle_level ? 0 : 1;
            digitalWrite(PIN_BUZZER, toggle_level ? HIGH : LOW);
          }
        } else {
          // Idle: keep buzzer low
          digitalWrite(PIN_BUZZER, LOW);
          toggle_level = 0;
        }
      }

      lastTickMs = nowMs;
      state = WAIT_TICK;
      break;
    }

    default: {
      state = INIT;
      break;
    }
  } // switch
}
