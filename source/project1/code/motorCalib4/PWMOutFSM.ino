void PWMOutputFSM_update() {
  //updates hardware pwm, and emits a (software) trigger 
  //user can change configs (lines 5-8) 

  const byte         PWM_PIN             = 9;      // must be PWM-capable 
  const unsigned int UPDATE_PERIOD_MS    = 2000;   // refresh rate 
  const byte         MAX_DUTY            = 255;    // upper clamp for safety (0..255)
  const bool         TRIGGER          = true ;  //create (software) trigger signal

  const byte INIT      = 0;
  const byte WAIT      = 1;
  const byte UPDATE    = 2;

  static byte state = INIT;
  static unsigned long lastMs;

  const unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      pinMode(PWM_PIN, OUTPUT);

      lastMs = nowMs;

      // Transition
      state = WAIT;
      break;
    }

    case WAIT: {
      // Activities: none (idle until next refresh)

      // Transition
      if (nowMs - lastMs >= UPDATE_PERIOD_MS) {
        state = UPDATE;
      }
      break;
    }

    case UPDATE: {
      // Activities: take desired duty, clamp/shape, write to pin
      byte duty = sig_waveform;        // desired (0..255) from waveform FSM

      // Safety clamp
      if (duty > MAX_DUTY) duty = MAX_DUTY;

      analogWrite(PWM_PIN, duty);

      if(TRIGGER)sig_trigger++; 

      sig_pwm = duty;

      lastMs = nowMs;

      // Transition
      state = WAIT;
      break;
    }

  } // switch
}
