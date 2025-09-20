
void UserInput_FSM_update() {
  /*
  UserInput_FSM_update()
  ----------------------------------------------------------------------------
  TASK
    Read a debounced pushbutton, an analog potentiometer, and parse user
    serial commands of the form:  <key> [=|:] <value>  terminated by newline.
    Updates shared global variables

  INPUT PARAMETERS
    None (all configuration as local consts).

  OUTPUT PARAMETERS (globals written)
    sig_input_button   (byte 0/1)
    sig_input_pot      (int 0..1023)
    sig_input_key      (char)
    sig_input_value    (long)
    sig_input_has_cmd  (bool set true when a new pair is parsed)

  HOW TO USE
    - Call once per loop(). Non-blocking.
    - Send lines like:  A=123\n   or   p:45\n   or   x -7\n
      (spaces optional; '=' or ':' optional; case preserved in key).

  HOW TO TEST
    - Wire a button to PIN_BUTTON using internal pullup (active LOW).
    - Turn the pot on PIN_POT (A0..A7 depending on board).
    - Open Serial Monitor @ BAUD and send "R=1" to turn on red LED if you
      later map commands to outputs.
*/
  // ------------------ Local configuration constants -----------------------
  const byte         PIN_BUTTON        = 3;        // digital pin for pushbutton
  const bool         BUTTON_PULLUP     = true;     // use INPUT_PULLUP if true
  const bool         BUTTON_ACTIVE_LOW = true;     // with pullup, pressed reads LOW
  const unsigned int BUTTON_DEBOUNCE_MS= 25;       // debounce time

  const byte         PIN_POT           = A0;       // analog pin for potentiometer
  const unsigned int POT_SAMPLE_MS     = 25;       // how often to sample the pot

  const unsigned long SERIAL_BAUD      = 115200;   // Serial console baud

  // ------------------ Local state constants (UPPERCASE) --------------------
  const byte INIT        = 0;
  const byte WAIT_TICK   = 1;
  const byte SCAN_INPUTS = 2;
  const byte PARSE_LINE  = 3;

  // ------------------ Local static variables ------------------------------
  static byte          state = INIT;

  // Button debounce bookkeeping
  static byte          btn_stable = 0;        // stable physical level (0/1 before normalization)
  static byte          btn_raw_prev = 0;
  static unsigned long btn_last_change_ms = 0;

  // Pot sampling
  static unsigned long pot_last_ms = 0;

  // Serial receiver buffer
  static char          rxbuf[32];
  static byte          rxidx = 0;

  // Timing
  unsigned long nowMs = millis();

  // ------------------ Helper functions-----------------------------------------
  auto readButtonNormalized = [&](void) -> byte {
    int raw = digitalRead(PIN_BUTTON);
    byte level = (raw != 0) ? 1 : 0;            // digitalRead -> 0/1 logic level
    // Normalize pressed=1 regardless of wiring polarity
    if (BUTTON_ACTIVE_LOW) level = (level == 0) ? 1 : 0;
    return level;
  };

  // Attempts to parse "<key><sep?><value>" from rxbuf[0..rxidx-1]
  auto tryParseLine = [&](char* s, byte n, char &outKey, long &outVal) -> bool {
    // Find first non-space as key (letter or any non-digit char)
    byte i = 0;
    while (i < n && (s[i] == ' ' || s[i] == '\t')) i++;
    if (i >= n) return false;
    outKey = s[i++];

    // Skip optional spaces and separators (= or :)
    while (i < n && (s[i]==' ' || s[i]=='\t')) i++;
    if (i < n && (s[i]=='=' || s[i]==':')) i++;
    while (i < n && (s[i]==' ' || s[i]=='\t')) i++;

    // Parse optional sign
    int sign = 1;
    if (i < n && (s[i]=='+' || s[i]=='-')) { if (s[i]=='-') sign = -1; i++; }

    // Accumulate decimal digits
    bool haveDigit = false;
    long val = 0;
    while (i < n && s[i] >= '0' && s[i] <= '9') {
      haveDigit = true;
      val = val * 10 + (long)(s[i]-'0');
      i++;
    }
    if (!haveDigit) return false;

    outVal = (long)sign * val;
    return true;
  };

  // ------------------ State machine ---------------------------------------
  switch (state) {

    case INIT: {
      // Configure pins
      pinMode(PIN_BUTTON, BUTTON_PULLUP ? INPUT_PULLUP : INPUT);
      pinMode(PIN_POT, INPUT);

      // Init button state
      btn_raw_prev = digitalRead(PIN_BUTTON) ? 1 : 0;
      btn_stable   = btn_raw_prev;
      btn_last_change_ms = nowMs;
      sig_input_button = BUTTON_ACTIVE_LOW ? (btn_stable ? 0 : 1) : btn_stable;

      // Init pot
      sig_input_pot = analogRead(PIN_POT);
      pot_last_ms = nowMs;

      // Serial
      Serial.begin(SERIAL_BAUD);
      rxidx = 0;

      // Next
      state = WAIT_TICK;
      break;
    }

    case WAIT_TICK: {
      // Button debounce check
      byte raw_now = digitalRead(PIN_BUTTON) ? 1 : 0;
      if (raw_now != btn_raw_prev) {
        btn_raw_prev = raw_now;
        btn_last_change_ms = nowMs;         // restart debounce
      } else {
        // stable long enough?
        if ((unsigned long)(nowMs - btn_last_change_ms) >= BUTTON_DEBOUNCE_MS && raw_now != btn_stable) {
          btn_stable = raw_now;
          // publish normalized button state
          sig_input_button = BUTTON_ACTIVE_LOW ? (btn_stable ? 0 : 1) : btn_stable;
        }
      }

      // Pot sampling
      if ((unsigned long)(nowMs - pot_last_ms) >= POT_SAMPLE_MS) {
        pot_last_ms = nowMs;
        sig_input_pot = analogRead(PIN_POT);
      }

      // Serial: accumulate characters if available
      if (Serial.available() > 0) {
        state = SCAN_INPUTS;
      }
      break;
    }

    case SCAN_INPUTS: {
      while (Serial.available() > 0) {
        char c = (char)Serial.read();

        // Line terminators trigger parsing
        if (c == '\n' || c == '\r' || c == ';') {
          if (rxidx > 0) {
            state = PARSE_LINE;
            break;
          } else {
            // ignore empty lines
            continue;
          }
        }

        // Keep within buffer
        if (rxidx < (sizeof(rxbuf)-1)) {
          rxbuf[rxidx++] = c;
        } else {
          // overflow: reset buffer
          rxidx = 0;
        }
      }

      if (state != PARSE_LINE) {
        // No complete line yet
        state = WAIT_TICK;
      }
      break;
    }

    case PARSE_LINE: {
      // Null-terminate and parse
      rxbuf[rxidx] = '\0';
      char key = 0;
      long val = 0;
      if (tryParseLine(rxbuf, rxidx, key, val)) {
        sig_input_key    = key;
        sig_input_value  = val;
        sig_input_has_cmd= true;   // latch; other modules may clear after handling
      }
      // Reset buffer for next line
      rxidx = 0;

      // Next
      state = WAIT_TICK;
      break;
    }

    default: {
      state = INIT;
      break;
    }
  } // switch
}
