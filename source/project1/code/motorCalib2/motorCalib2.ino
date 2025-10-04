 //user can select in the loop combinations of state machines (lines 17-20) 

byte sig_waveform = 42;   // waveform sample
byte sig_pwm = 0;  // value written to PWM pin
float sig_rpm = 0.0;   // rpm measurement
unsigned long sig_trigger=0;   // trigger counter 
volatile unsigned long sig_encoder = 0; // Encoder counter
void encoderISR() { //interrupt service routine
  sig_encoder++;
}

void setup() {
  // all FSM self-initialize on first call.
}

void loop() {
  WaveformFSM3_update();      // updates sig_waveform_value (in free run)
  //PWMOutputFSM_update();    // writes hardware PWM (in free run) and can create trigger
  //RPM_FSM_update();         // uses encoder and interrupts compute sig_rpm (free run or triggered)
  DataLogger_FSM_update();    //data logger to serial (free run or triggered)
}
