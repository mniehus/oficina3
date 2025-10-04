void PWMOutputFSM_update() {

  //user can change configs (lines 5-7) 

  const byte         PWM_PIN             = 9;      // PWM-capable pin 
  const unsigned int UPDATE_PERIOD_MS    = 2000;     // refresh rate 
  const byte         MAX_DUTY            = 255;    // upper clamp for safety (0..255)

  const byte INIT      = 0;
  const byte WAIT_TICK = 1;
  const byte UPDATE    = 2;
  static byte state = INIT;

  static unsigned long lastTickMs;

  const unsigned long nowMs = millis();

  // ------------------ State machine --------------------
  switch (state) {

    case INIT: {
      // Activities
      pinMode(PWM_PIN, OUTPUT);

      lastTickMs = nowMs;

      // Transition
      state = WAIT_TICK;
      break;
    }

    case WAIT_TICK: {
      // Activities: none (idle until next refresh)

      // Transition
      if (nowMs - lastTickMs >= UPDATE_PERIOD_MS) {
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

      sig_trigger++;   

      sig_pwm = duty;

      lastTickMs = nowMs;

      // Transition
      state = WAIT_TICK;
      break;
    }

  } // switch
}
