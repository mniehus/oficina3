void PID_FSM_update() {
    /*
  PID_FSM_update(): PID controller as a 3-state FSM (INIT, WAIT, MEASURE).
  - Uses the Arduino PID_v1 library.
  - Modes: TRIGGERED (compute on external trigger change) or FREE-RUN (fixed period).
// Uses globals (declare them in main file ):
// unsigned long sig_trigger;      // increments on each external trigger
// float        sig_pid_setpoint; // desired setpoint (same scale as measurement)
// float        sig_pid_measure;  // process variable (measurement)
// float        sig_pid_output;   // controller output
*/
  // -------- Config --------
  const bool         TRIGGERED = true;  // true: compute on external trigger; false: periodic
  const unsigned int SAMPLE_MS = 50;    // free-run period & PID sample time

  // PID tunings 
  const double KP = 2.0, KI = 5.0, KD = 1.0;

  // Output clamp (could be omitted)
  const float OUT_MIN = 0.0;
  const float OUT_MAX = 255.0;

  // -------- States --------
  const byte INIT    = 0;
  const byte WAIT    = 1;
  const byte MEASURE = 2;

  // -------- Locals --------
  static byte state = INIT;

  // PID instance bound directly to GLOBAL signals
  static PID pid(&sig_pid_measure, &sig_pid_output, &sig_pid_setpoint, KP, KI, KD, DIRECT);

  static unsigned long lastTrigger = 0;
  static unsigned long lastMs      = 0;

  unsigned long nowMs = millis();

  switch (state) {

    case INIT:
      pid.SetOutputLimits(OUT_MIN, OUT_MAX);
      pid.SetSampleTime(SAMPLE_MS);
      pid.SetMode(AUTOMATIC);

      lastTrigger = sig_trigger;
      lastMs      = nowMs;
      state       = WAIT;
      break;

    case WAIT:
      if (TRIGGERED) {
        if (sig_trigger != lastTrigger) {
          state = MEASURE;        // compute on external event
        }
      } else {
        if (nowMs - lastMs >= SAMPLE_MS) {
          state = MEASURE;        // periodic compute
        }
      }
      break;

    case MEASURE:
      // Measurement and setpoint are already in globals.
      pid.Compute();              // writes sig_pid_output (clamped to OUT_MIN..OUT_MAX)

      lastMs = nowMs;
      if (TRIGGERED) lastTrigger = sig_trigger;
      state = WAIT;
      break;
  }//switch
}