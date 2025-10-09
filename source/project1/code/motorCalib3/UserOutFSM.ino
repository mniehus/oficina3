void UserOut_FSM_update() {
  // visual (LED) and acoustic (BUZZER) user output
  // emits short pulses in free run or triggered  mode 
  //user can change configs (lines 6-11) 

  const byte PIN_LED = 5;        // any digital pin
  const byte PIN_BUZZER = 8;     // any digital pin 

  const bool         TRIGGERED  = true;                // true: triggered; false: free run
  const unsigned int FREE_RUN_PERIOD_MS   = 1000;      // used only in free run, if not triggered
  const unsigned int PULSE_WIDTH_MS = 200;           // duration of LED / BUZZER

  const byte INIT      = 0;
  const byte WAIT      = 1;
  const byte ACTIVE    = 2;

  static byte          state = INIT;
  static unsigned long lastMs = 0;
  static unsigned long lastTrigger = 0;

  static bool toggle = false;

  unsigned long nowMs = millis();

  switch (state) {

    case INIT:
      pinMode(PIN_LED, OUTPUT);
      digitalWrite(PIN_LED, LOW);
      pinMode(PIN_BUZZER, OUTPUT);
      digitalWrite(PIN_BUZZER, LOW);

      if(TRIGGERED) lastTrigger = 0;
      else lastMs = 0;
      state = WAIT;
      break;

    case WAIT: 
       if (TRIGGERED && sig_trigger != lastTrigger) {
          toggle = true;
          lastTrigger = sig_trigger; 
        }
       else if (nowMs - lastMs >= FREE_RUN_PERIOD_MS) {
          toggle = true;
          //lastMs = nowMs;
        }
       if (toggle){
          digitalWrite(PIN_LED, HIGH);
          digitalWrite(PIN_BUZZER, HIGH);
          toggle=false;
          lastMs=nowMs;
          state = ACTIVE; 
       }
      break;

    case ACTIVE: 
      if (nowMs - lastMs >= PULSE_WIDTH_MS) {
          digitalWrite(PIN_LED, LOW);
          digitalWrite(PIN_BUZZER, LOW);
          state = WAIT; 
       }
      break;

  } // switch
}
