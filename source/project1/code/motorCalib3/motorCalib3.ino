 #include <Wire.h> 
 #include <AS5600.h>
 #include <PID_v1.h>
 
 //user select state machines in the loop  

//global signals (published or subscribed by FSMs)
byte sig_waveform = 42;     // waveform sample
byte sig_pwm = 0;           // value written to PWM pin
float sig_rpm = 0.0;        // rpm measurement
float sig_angle_deg = 0.0;  // angle measurement
unsigned long sig_trigger=0;// trigger counter 
volatile unsigned long sig_encoder = 0; // encoder counter
void encoderISR() { //interrupt service routine
  sig_encoder++;
}

//pid controller i/o (to be configured in loop)
double sig_pid_setpoint=0.0; // setpoint 
double sig_pid_measure=0.0;  // measurement
double sig_pid_output=0.0;   // controller output

void setup() {
  // all FSM self-initialize on first call
}

void loop() {
  WaveformFSM3_update();      // updates sig_waveform_value (in free run)
  //PWMOutputFSM_update();    // writes hardware PWM (in free run) and can create trigger
  //RPM_FSM_update();         // updates sig_rpm (free run or triggered) from encoder and interrupts 
  DataLogger_FSM_update();    // data logger to serial (free run or triggered)
  //UserOut_FSM_update();     // updates visual (LED) and/or acoustic (BUZZER) feedback 
  //Angle_FSM_update();       // updates angle measurement via AS5600 sensor

  //sig_pid_setpoint=sig_waveform; // update setpoint for PID
  //sig_pid_measure=sig_angle_deg;  // update measurement for PID
  //PID_FSM_update();              // update PID controller
  //sig_pwm=sig_pid_output;       // update controller output from PID
}
