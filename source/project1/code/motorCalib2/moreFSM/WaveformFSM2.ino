void WaveformFSM2_update() {

  const byte WAVEFORM_SINE     = 0;
  const byte WAVEFORM_TRIANGLE = 1;
  const byte WAVEFORM_SQUARE   = 2;

  const byte          WAVEFORM_SELECT      = 1; // choose one
  const unsigned int  SAMPLE_PERIOD_MS     = 10;            // time between samples
  const unsigned long MODULATION_PERIOD_MS = 2000*10*2;    // full cycle period (user must set > 0)
  const byte          AMPLITUDE            = 200;           // 0..255
  const bool          RUN_SINGLE_CYCLE     = true;          // true: one cycle then hold at 0

  // -------- States --------
  const byte INIT      = 0;
  const byte WAIT_TICK = 1;
  const byte COMPUTE   = 2;
  const byte DONE      = 3;

  // -------- Locals --------
  static byte          state    = INIT;
  static unsigned long lastTick = 0;
  static unsigned long t0       = 0;

  unsigned long nowMs = millis();

  unsigned long elapsed = nowMs - t0;
  float y = 0.0;

  switch (state) {

    case INIT: 
      sig_waveform = 0;
      lastTick = nowMs;
      t0 = nowMs;
      state = WAIT_TICK;
      break;
    

    case WAIT_TICK: 
      if (nowMs - lastTick >= SAMPLE_PERIOD_MS) {
        state = COMPUTE;
      }
      break;
    

    case COMPUTE: 

      float phase; // Phase in [0..1) 

      // Stop after exactly one cycle if requested
      if (RUN_SINGLE_CYCLE && (elapsed >= MODULATION_PERIOD_MS)) {
        sig_waveform = 0;   // ensure final value is zero
        state = DONE;
        break;
      }

      if (!RUN_SINGLE_CYCLE) elapsed = elapsed % MODULATION_PERIOD_MS;
      phase = (float)elapsed / (float)MODULATION_PERIOD_MS;

      // Normalized waveform y in [0..1]
      if (WAVEFORM_SELECT == WAVEFORM_SINE) {
        y = 0.5 * (1.0 + sin(6.28 * phase));                 
      } else if (WAVEFORM_SELECT == WAVEFORM_TRIANGLE) {
        y = (phase < 0.5) ? (phase * 2.0) : (2.0 - 2.0 * phase);
      } else if (WAVEFORM_SELECT == WAVEFORM_SQUARE) {
        y = (phase < 0.5) ? 0.0 : 1.0;
      } else {
        y = 0.0;
      }
      sig_waveform = y*AMPLITUDE; 
      //sig_waveform_value = constrain(sig_waveform_value, 0, 255); 

      lastTick = nowMs;
      state = WAIT_TICK;
      break;

    case DONE: 
      sig_waveform = 0;   
      break;

  }
}
