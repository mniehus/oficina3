void RPM_FSM_update() {
//takes encoder counts provided by ISR
//and computes RPM (free run or triggered mode)  
//user can change configs (lines 6-13) 

  const byte         ENCODER_PIN          = 2;        // must be interrupt-capable
  const byte         PULSES_PER_REV       = 2;        // 2 blades -> 2 pulses per revolution
  const byte         ISR_EDGE_SELECT      = 0;        // 0:RISING, 1:FALLING, 2:CHANGE

  const bool         TRIGGERED  = true;                // true: measure between PWM updates; false: free run
  const unsigned int FREE_RUN_PERIOD_MS   = 100;      // used only in free run, if not triggered

  const unsigned int MIN_DT_MS            = 1;        // guard minimum interval to avoid div-by-zero

  const byte INIT            = 0;
  const byte IDLE            = 1;   // waiting for next interval trigger (time or PWM)
  const byte START_INTERVAL  = 2;   // mark interval start (count & time)
  const byte END_INTERVAL    = 3;   // mark interval end (count & time)
  const byte COMPUTE         = 4;   // compute RPM, publish

  static byte state = INIT;

  // book-keeping
  static unsigned long startCount = 0;
  static unsigned long lastMs    = 0;
  static unsigned long endCount   = 0;
  static unsigned long endMs      = 0;

  // Triggered on global variable
  static unsigned long lastTrigger = 0;    

  // For free-run timing
  static unsigned long tickMs      = 0;

  static float         rpmPrev      = 0.0;
  bool                 measureNow   = false;

  unsigned long nowMs = millis();

  // ------------------ State machine ------------------------
  switch (state) {

    case INIT: 
      // Configure encoder pin & interrupt
      pinMode(ENCODER_PIN, INPUT);
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
      lastTrigger = sig_trigger;
      startCount  = sig_encoder;
      lastMs     = nowMs;
      tickMs      = nowMs;
      sig_rpm     = 0.0;
      rpmPrev     = 0.0;
      measureNow   = false;

      // Transition
      state = START_INTERVAL;
      
      break;
    
    case START_INTERVAL: 
      // Activities: mark interval start (count & time)
      startCount = sig_encoder;
      lastMs = nowMs;
      // Transitions 
      if (TRIGGERED) lastTrigger = sig_trigger; 
      else tickMs = nowMs; // Free-run: close after FREE_RUN_PERIOD_MS
      state = END_INTERVAL;
    break;
    
    case END_INTERVAL:

      if (TRIGGERED && sig_trigger != lastTrigger) {
          measureNow = true;
          lastTrigger = sig_trigger; // latch closing edge
        }
       else if (nowMs - lastMs >= FREE_RUN_PERIOD_MS) {
          measureNow = true;
        }

      if (measureNow) {
      endCount = sig_encoder;  
      endMs = nowMs;
      // Transition
      state = COMPUTE;
      }
    break;

  case COMPUTE: 
      // Activities: compute RPM
      unsigned long dt_ms = endMs - lastMs;
      if (dt_ms < MIN_DT_MS || PULSES_PER_REV == 0) {
        // Too short / invalid; keep previous RPM or set zero
        sig_rpm = rpmPrev;
      } else {
        float dt_s   = (float)dt_ms / 1000.0;
        float revs   = (float)(endCount - startCount) / (float)PULSES_PER_REV;
        float rpmNew = (revs / dt_s) * 60.0;

        sig_rpm = rpmNew;
        //sig_rpm=millis();
        rpmPrev = sig_rpm;
      }

      // Transition: go wait for the next interval trigger
      state = START_INTERVAL;
      break;
    
  } // switch
}
