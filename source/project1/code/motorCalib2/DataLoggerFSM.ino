void DataLogger_FSM_update() {

  //user can change configs (lines 5-7) and the prints in state LOG (lines 45-55) 

  const unsigned long BAUD_RATE          = 115200;
  const bool          TRIGGERED          = false;    // true: triggered; false: free-run
  const unsigned long LOG_PERIOD_MS      = 20;     // used only in free-run mode

  const byte INIT = 0;
  const byte IDLE = 1;
  const byte LOG  = 2;
  static byte state = INIT;

  static unsigned long lastTickMs;         // free-run cadence
  static unsigned long lastTrigger;    // edge detector for trigger mode

  // ------------------ Time base -------------------------
  unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      Serial.begin(BAUD_RATE);

      lastTickMs    = nowMs;
      lastTrigger  = sig_trigger;

      // Transition
      state = IDLE;
      break;
    }

    case IDLE: 
      // Transition conditions
      if (TRIGGERED && sig_trigger != lastTrigger) {
          state = LOG;
        }
      else if (nowMs - lastTickMs >= LOG_PERIOD_MS ) {
        state = LOG;
        }
    break;

    case LOG:
      // Activities: print TSV lines (tab separated values) 
      //unsigned long tNow = millis(); // timestamp closer to print moment
      //Serial.print(tNow); 
      //Serial.print('\t');
      Serial.print(sig_waveform); 
      //Serial.print('\t');
      //Serial.print(sig_pwm); 
      //Serial.print('\t'); 
      //Serial.print(sig_rpm);

      Serial.println();//line break

      // Book-keeping
      if (TRIGGERED) {
        lastTrigger = sig_trigger;
      } else {
        lastTickMs = nowMs;
      }

      // Transition
      state = IDLE;
      break;
  } // switch
}//fsm
