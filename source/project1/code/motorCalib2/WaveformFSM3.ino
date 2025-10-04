void WaveformFSM3_update() {

 //user can change configs (lines 11-19) 

  // -------- Wave types --------
  const byte WAVEFORM_SINE              = 0;
  const byte WAVEFORM_TRIANGLE          = 1;
  const byte WAVEFORM_SQUARE            = 2;
  const byte WAVEFORM_STAIRS_UP_DOWN     = 3;   

  // -------- Config --------
  const byte          WAVEFORM_SELECT      = WAVEFORM_STAIRS_UP_DOWN;    // choose one
  const unsigned int  SAMPLE_PERIOD_MS     = 50;               // time between samples
  const unsigned long MODULATION_PERIOD_MS = 2000;             // full cycle period (>0)
  const byte          AMPLITUDE            = 200;               // 0..255
  const bool          RUN_SINGLE_CYCLE     = false;             // true: one cycle then 0
  const byte          STEP_PERCENT         = 10;                 // step size as % of AMPLITUDE 

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

    case COMPUTE: {
      unsigned long elapsed = nowMs - t0;

      // Single-cycle end
      if (RUN_SINGLE_CYCLE && (elapsed >= MODULATION_PERIOD_MS)) {
        sig_waveform = 0;
        state = DONE;
        break;
      }

      // Phase in [0..1] (single) or [0..1) (continuous)
      if (!RUN_SINGLE_CYCLE) elapsed = elapsed % MODULATION_PERIOD_MS;
      float phase = (float)elapsed / (float)MODULATION_PERIOD_MS;

      // Normalized y in [0..1]
      float y = 0.0;

      if (WAVEFORM_SELECT == WAVEFORM_SINE) {
        y = 0.5 * (1.0 + sin(6.2831853 * phase));
      } else if (WAVEFORM_SELECT == WAVEFORM_TRIANGLE) {
        y = (phase < 0.5) ? (phase * 2.0) : (2.0 - 2.0 * phase);
      } else if (WAVEFORM_SELECT == WAVEFORM_SQUARE) {
        y = (phase < 0.5) ? 0.0 : 1.0;
      } else if (WAVEFORM_SELECT == WAVEFORM_STAIRS_UP_DOWN) {
        // Triangle, then quantize to steps of STEP_PERCENT% of amplitude
        float tri = (phase < 0.5) ? (phase * 2.0) : (2.0 - 2.0 * phase);   // 0..1 triangle
        float stepSize = (float)STEP_PERCENT / 100.0;                       // 0..1
        int   k = (int)(tri / stepSize);                                    // which step
        y = (float)k * stepSize;                                            // quantized (staircase)
        if (y > 1.0) y = 1.0;
      } else {
        y = 0.0;
      }

      // Compute -> constrain -> publish (common for all waves)
      int outVal = (int)(y * (float)AMPLITUDE + 0.5);
      sig_waveform = (byte)constrain(outVal, 0, 255);

      lastTick = nowMs;
      state = WAIT_TICK;
      break;
    }

    case DONE:
      sig_waveform = 0;   // hold at zero after single cycle
      break;
  }
}