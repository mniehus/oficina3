
void DataLogger_FSM_update(bool includeTimeMs, bool includeWaveform, bool includePWM, bool includeRPM) {
  /*
  DataLogger_FSM_update(includeTimeMs, includeWaveform, includePWM, includeRPM)
  ----------------------------------------------------------------------------
  TASK
    Emit TSV logs with user-selectable columns, operating in one of two modes:
      (A) Free-run periodic logging (fixed period).
      (B) PWM-synchronized logging (exactly once per PWM output update window).

  INPUT PARAMETERS
    bool includeTimeMs   - include current millis() in the first column
    bool includeWaveform - include sig_waveform_value
    bool includePWM      - include sig_pwm_applied
    bool includeRPM      - include sig_rpm

  OUTPUT
    - Writes a single TSV line per logging event with the selected fields.

  HOW TO USE
    1) Call DataLogger_FSM_update(...) each loop().
    2) Choose mode with the local constant MODE_COUPLED_TO_PWM (true/false).
    3) Ensure:
         - PWMOutputFSM_update() bumps sig_pwm_update_counter on each analogWrite.
         - RPM_FSM_update() runs before this logger when in coupled mode,
           so sig_rpm corresponds to the window that just ended.

  HOW TO TEST
    - Start with (true, true, true, true) and watch the Serial output.
    - Toggle the booleans at runtime; header line updates automatically.

  HOW TO EXPAND
    - Add more columns (e.g., duty target), each with its own boolean gate.
    - Add units row or CSV mode by changing the delimiter.
    - Add a separate timestamp origin (t0) if desired.

  NOTE
    - Arduino-idiomatic types used throughout.
*/
  // ------------------ Local configuration constants -----------------------
  const unsigned long BAUD_RATE          = 115200;
  const bool          MODE_COUPLED_TO_PWM= true;    // true: sync to PWM edges; false: free-run
  const unsigned long LOG_PERIOD_MS      = 100;     // used only in free-run mode

  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT = 0;
  const byte IDLE = 1;
  const byte LOG  = 2;

  // ------------------ Local static variables ------------------------------
  static byte          state = INIT;
  static unsigned long lastTickMs = 0;         // free-run cadence
  static unsigned long lastSeenPwmEdge = 0;    // edge detector for sync mode

  // Persist last field selection to reprint header if layout changes
  static bool lastIncludeTimeMs   = false;
  static bool lastIncludeWaveform = false;
  static bool lastIncludePWM      = false;
  static bool lastIncludeRPM      = false;

  // ------------------ Time base -------------------------
  unsigned long nowMs = millis();

  // ------------------ Helper: header printer -------------------------------
  auto printHeader = [&](bool t, bool w, bool p, bool r) {
    if (!(t || w || p || r)) return;  // nothing selected, no header
    bool first = true;
    if (t) { Serial.print(F("time_ms")); first = false; }
    if (w) { if (!first) Serial.print('\t'); Serial.print(F("waveform_value")); first = false; }
    if (p) { if (!first) Serial.print('\t'); Serial.print(F("pwm_applied"));    first = false; }
    if (r) { if (!first) Serial.print('\t'); Serial.print(F("rpm")); }
    Serial.println();
  };

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      Serial.begin(BAUD_RATE);
      printHeader(includeTimeMs, includeWaveform, includePWM, includeRPM);

      lastIncludeTimeMs   = includeTimeMs;
      lastIncludeWaveform = includeWaveform;
      lastIncludePWM      = includePWM;
      lastIncludeRPM      = includeRPM;

      lastTickMs       = nowMs;
      lastSeenPwmEdge  = sig_pwm_update_counter;

      // Transition
      state = IDLE;
      break;
    }

    case IDLE: {
      // Activities: if selection changed, print a refreshed header
      if (includeTimeMs   != lastIncludeTimeMs   ||
          includeWaveform != lastIncludeWaveform ||
          includePWM      != lastIncludePWM      ||
          includeRPM      != lastIncludeRPM) {

        printHeader(includeTimeMs, includeWaveform, includePWM, includeRPM);

        lastIncludeTimeMs   = includeTimeMs;
        lastIncludeWaveform = includeWaveform;
        lastIncludePWM      = includePWM;
        lastIncludeRPM      = includeRPM;
      }

      // Transition conditions per mode
      if (MODE_COUPLED_TO_PWM) {
        if (sig_pwm_update_counter != lastSeenPwmEdge) {
          state = LOG;
        }
      } else {
        if ((unsigned long)(nowMs - lastTickMs) >= LOG_PERIOD_MS) {
          state = LOG;
        }
      }
      break;
    }

    case LOG: {
      // Activities: emit one TSV line (if any field enabled)
      if (includeTimeMs || includeWaveform || includePWM || includeRPM) {
        bool first = true;
        unsigned long tNow = millis(); // timestamp closer to print moment

        if (includeTimeMs)   { Serial.print(tNow); first = false; }
        if (includeWaveform) { if (!first) Serial.print('\t'); Serial.print(sig_waveform_value); first = false; }
        if (includePWM)      { if (!first) Serial.print('\t'); Serial.print(sig_pwm_applied);    first = false; }
        if (includeRPM)      { if (!first) Serial.print('\t'); Serial.println(sig_rpm, 2);} else {Serial.println(sig_rpm, 2); }
        if (!includeRPM)     { if (includeTimeMs || includeWaveform || includePWM) Serial.println(); }
      }

      // Book-keeping per mode
      if (MODE_COUPLED_TO_PWM) {
        // Consume this PWM edge so we wait for the next one
        lastSeenPwmEdge = sig_pwm_update_counter;
      } else {
        lastTickMs = nowMs;
      }

      // Transition
      state = IDLE;
      break;
    }

    default: {
      state = INIT;
      break;
    }
  } // switch
}
