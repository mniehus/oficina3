# Resources

## Course links
- [Course communication via Moodle](https://2425moodle.isel.pt/course/view.php?id=8793)
- [Course documentation (this page)](https://mniehus.github.io/oficina3)
- [Course repository (online)](https://github.com/mniehus/oficina3)

--- 

## Collaboration Plan for COIL (project3)

This project brings together international student teams to model, simulate, implement, and control a Reaction Wheel Inverted Pendulum (RWIP). The goal is to explore both technical and collaborative challenges in control engineering through simulation and physical implementation.

---

**Week 1: Kickoff & Icebreakers**
- Meet your international teammates (via Zoom or Teams).
- Participate in structured icebreaker activities.
- Share backgrounds, expectations, and technical interests.
- Set up collaborative tools: VS Code, Python, GitHub, shared folders.
- Begin reading reference material on inverted pendulums and reaction wheels.

 _Tip: Keep a shared team logbook and schedule weekly check-ins._

---

**Week 2: System Understanding & Parameter Assignment**
- Study the inverted pendulum and reaction wheel system.
- Identify key physical and control concepts: torque, moment of inertia, stability.
- Assign unique parameter sets to each team (e.g., pendulum length, mass, motor power, wheel inertia).
- Clarify technical goals and system constraints.
- Start drafting control objectives.

---

**Week 3: Simulation & Strategy Development**
- Build Python simulations of the RWIP system.
- Explore system dynamics (e.g., swing-up vs. stabilization).
- Identify stable and unstable parameter regions.
- Test control strategies:
  - Open-loop behavior
  - Swing-up with **bang-bang control**
  - Stabilization using **PID** and **LQR** controllers
- Compare performance metrics across teams.

---

**Week 4: Physical Implementation & Integration**
- Assemble the RWIP using available materials (e.g., 3D printed parts, hardware components).
- Integrate sensors (e.g., AS5600 magnetic sensor) and actuators (brushed DC motor).
- Wire and interface the system with Arduino or compatible microcontroller.
- Characterize physical properties: friction, inertia, motor constants.
- Validate component specs vs. datasheets.

---

**Week 5: Testing, Comparison & Analysis**
- Deploy controllers on the Arduino (migrating from Python simulations).
- Test swing-up and stabilization phases in real conditions.
- Compare experimental results with simulated predictions.
- Measure performance: response time, overshoot, stability margins.
- Evaluate control effectiveness and hardware limitations.

---

**Final Session: Reflection & Presentation**
- Present each team’s approach, implementation, and results.
- Reflect on the technical, intercultural, and collaborative experiences.
- Share lessons learned and insights into system limitations.
- Discuss real-world relevance and next steps for further development.

**Optional Deliverables**
Each team should maintain:
- Simulation notebooks (.ipynb)
- Arduino code (.ino)
- CAD files (.stl, .step)
- Data logs (.csv)
- Diagrams, images, and demo videos (.png, .mp4)
- -Description of the system and control strategy  
- Simulations and analysis  
- Photographs and diagrams  
- Electrical and mechanical schematics  
- STL files for 3D parts
- Software architecture diagrams  
- Source code (Python and Arduino)
- Description of tests and results  
- Short video of system in action  
- Reflections and conclusions

Include all in your team’s final GitHub repository.

---



## Tutorials

---

## Reference implementation

---

## Repository

---

## Technical data

---

## Data Formats

| Extension | Description                        | Tool/Use                          |
|-----------|------------------------------------|-----------------------------------|
| `.py`     | Python scripts                     | VS Code, Python                   |
| `.ipynb`  | Jupyter notebooks                  | VS Code (Jupyter), JupyterLab     |
| `.ino`    | Arduino sketches                   | Arduino IDE                       |
| `.stl`    | 3D models for printing             | Fusion 360, Onshape, Cura         |
| `.csv`    | Sensor logs, tabular data          | Python, Excel, Pandas             |
| `.mp4` / `.mp3` | Video/audio demos            | OBS, DaVinci Resolve, Phones      |
| `.jpg` / `.png`| Diagrams, photos              | draw.io, Canva, Snipping Tool     |
| `.pdf`    | Final report and documentation     | Word, LaTeX, Google Docs          |

---

## Code snippets


```cpp
#include <Wire.h>
#include <AS5600.h>

// === Sensor Setup ===
AMS_5600 encoder;

// === Motor Pins ===
const int pwmPin = 9;     // PWM signal to motor
const int dirPin1 = 7;    // Motor direction
const int dirPin2 = 8;

float targetAngle = 180.0;   // Upright position in degrees
float currentAngle = 0.0;

// === PID Parameters ===
float kp = 1.5;
float ki = 0.1;
float kd = 0.05;

float error = 0;
float previousError = 0;
float integral = 0;

unsigned long lastTime = 0;
float dt = 0;

// === Motor Control Function ===
void setMotor(float pwmValue) {
  pwmValue = constrain(pwmValue, -255, 255);

  if (pwmValue > 0) {
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
    analogWrite(pwmPin, pwmValue);
  } else {
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
    analogWrite(pwmPin, -pwmValue);
  }
}

// === Get Angle in Degrees ===
float getAngleDeg() {
  int rawAngle = encoder.getRawAngle(); // 0–4095
  float angle = (rawAngle / 4095.0) * 360.0;
  return angle;
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);

  delay(500); // Allow sensor startup
  Serial.println("PID Control Initialized");
  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  dt = (currentTime - lastTime) / 1000.0;
  lastTime = currentTime;

  currentAngle = getAngleDeg();
  error = targetAngle - currentAngle;

  integral += error * dt;
  float derivative = (error - previousError) / dt;

  float output = kp * error + ki * integral + kd * derivative;
  setMotor(output);

  previousError = error;

  // Debug info
  Serial.print("Angle: ");
  Serial.print(currentAngle);
  Serial.print(" | Output: ");
  Serial.println(output);

  delay(10); // ~100 Hz loop
}

```
---

## References
- [Swing Up and Balancing of a Reaction Wheel Inverted Pendulum](http://ise.ait.ac.th/wp-content/uploads/sites/57/2020/12/Swing-Up-and-Balancing-of-a-Reaction-Wheel-Inverted-Pendulum.pdf)

- [Inverted Pendulum: State-Space Methods for Controller Design](https://ctms.engin.umich.edu/CTMS/index.php?example=InvertedPendulum&section=ControlStateSpace)

## Contacts



