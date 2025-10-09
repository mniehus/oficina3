void DataLogger_FSM_update() {
  //sends values in TSV format to serial port
  //user can change configs (lines 5-7) and the prints in state LOG (lines 45-55) 

  const unsigned long BAUD_RATE          = 115200;
  const bool          TRIGGERED          = false;   // true: triggered; false: free-run
  const unsigned long LOG_PERIOD_MS      = 20;     // in free-run mode only

  const byte INIT = 0;
  const byte WAIT = 1;
  const byte LOG  = 2;

  static byte state = INIT;
  static unsigned long lastMs;         // for free-run (independent)
  static unsigned long lastTrigger;    // for trigger mode (dependent)

  // ------------------ Time base -------------------------
  unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: 
      // Activities
      Serial.begin(BAUD_RATE);

      lastMs    = nowMs;
      lastTrigger  = sig_trigger;

      // Transition
      state = WAIT;
      break;

    case WAIT: 
      // Transition conditions
      if (TRIGGERED && sig_trigger != lastTrigger) {
          state = LOG;
        }
      else if (nowMs - lastMs >= LOG_PERIOD_MS ) {
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
        lastMs = nowMs;
      }

      // Transition
      state = WAIT;
      break;
  } // switch
}//fsm
