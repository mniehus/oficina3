# State-Machines applied to DC Motor Calibration 

This document is an **intro + guide** to the project of the DC Motor Calibration Automated Test Setup. 

The project automates a **speed–voltage calibration** for a small DC propulsion unit. An Arduino generates a configurable **PWM (=pulse width modulation) ramp TTL signal** that drives an H-bridge motor driver, which powers a **3.7 V drone DC motor** with a **two-blade propeller**. The motor sits in a simple rig with an **optical slot sensor** acting as an incremental encoder: each blade crossing produces a pulse used to infer **rotational speed RPM (=rotations per minute)**. Test parameters—ramp maximum, duration, step/cadence—are set in software so the same bench hardware can run many repeatable sweeps.

During a run, the PWM ramp and the RPM measurements are **synchronized**: PWM updates define clean timing windows over which pulses are counted and converted to RPM (or, alternatively, a fixed sampling period can be used). After each measurement, the system streams **tab-separated values (TSV)** over the USB serial link—time, commanded waveform value, applied PWM, and RPM—so the user can copy-paste straight into Excel (or any spreadsheet) for plotting and further analysis.

The **hardware part** is simple. A **PC** connects over **USB** to an **Arduino board** (e.g., Arduino Uno). One **PWM-capable digital pin** on the Arduino feeds the **H-bridge DC motor driver’s enable/PWM input**. 

The H-bridge board is powered by a **bench DC supply** (set to the motor’s rated voltage/current) and drives a **3.7 V mini drone DC motor** fitted with a **two-blade propeller**. Verify that voltage–current operating points during the calibration ramp are not limited by the DC supply (current limit/sag), especially at high speed/high load. 

An **optical slot sensor** (IR LED + phototransistor) is mounted in a small mechanical setup so each blade interrupts the beam once per revolution half-turn, producing two clean pulses per revolution. Use the slot sensor board’s LED indicators to align the blades for a clean RPM signal, and prefer the sensor’s digital output over the analog output. The sensor output connects to an **Arduino interrupt pin** (e.g., D2) for accurate pulse counting. 

Logic grounds are commoned: Arduino GND ↔ driver logic GND ↔ sensor GND. The motor power supply is separated from the logic power supply, which stays tied at the driver. Power the motor from the bench supply and the Arduino from USB to reduce coupling; the H-bridge’s internal diodes handle flyback.

For real time observation, use two scope test points: the **PWM signal** (Arduino → driver enable) and the **RPM pulse signal** (sensor → Arduino interrupt). View duty and pulse timing on an oscilloscope while the Arduino simultaneously uses them (drive and measure). 

For user input, a **pushbutton**, a **potentiometer (knob)**, and the **USB serial interface** can be considered for future use. Also, for user output, **three RGB LEDs** for visual feedback and a **piezo buzzer** for acoustic signalling/sound feedback can be used. These are included for completeness here; however, their use in the current project is optional, but they can be handy to implement simple user interfaces here or in other projects.

The **software part** is organized into six small independent tasks, through modules that are coded as  **finite state machines (FSMs)**. 

This system is an **experimental test ground for academic training**. While the automated DC motor calibration could be implemented more simply, the goal is to introduce and consolidate a **reusable, modular FSM framework** that is scalable and easy to adapt to future projects.

It’s meant to teach students how FSMs work, how to implement and test them, to understand why we use FSMs, and show that the code is reusable and scalablable, that it stays clear and tidy as the system grows (no spaghetti!), and how the modules are integrated into a system, and how they can interact via a small set of shared globals.

---

## Why State Machines?

**The problem:** embedded systems juggle multiple tasks—waveform generation, PWM output, RPM measurement, logging, user I/O—often with different cadences and hardware constraints. Naive loops with delays quickly become unmanageable.

**State machines** solve this by:

* **Modularity:** each feature lives in its own file/function with its own states, timers, and config; no spaghetti delays.
* **Testability:** each FSM can be exercised in isolation (e.g., force states or inject inputs) and verified.
* **Determinism:** all FSMs are **non-blocking** and run every loop; timing is explicit (period checks), not implicit (`delay()`).
* **Extensibility:** add a state or transition without rewriting unrelated parts.
* **Low cross coupling:** modules “talk” only through a few **shared global signals** (bytes, floats, counters), not through deep call chains.
* **Usability:** the behavior of each module is easy to explain with a simple **UML state chart**.

---

## Project layout 

This project follows a **modular state-machine architecture**: every essential task runs **non-blocking** in its own finite state machine (FSM), coded in a **separate, reusable file**. Each FSM keeps its own local configuration (as `const` inside the function), uses internal state to handle timing (no `delay()`), and communicates only through a few shared global signals. The **Arduino controller file** in the same folder configures the interrupt(s), declares the shared globals, and assembles the system by calling all FSMs in `loop()`.


**Controller file**

```cpp
MotorCalib.ino                  // controller, globals, interrupt config, empty setup(), loop() calling all FSMs
```
**FSM Modules**
```cpp
WaveformFSM.ino                 // WaveformFSM_update()
PWMOutputFSM.ino                // PWMOutputFSM_update()
RPM_FSM.ino                     // RPM_FSM_update()
DataLogger_FSM.ino           // UnifiedLogger_FSM_update(...)
UserInput_FSM.ino               // UserInput_FSM_update()
UserOutput_FSM.ino              // UserOutput_FSM_update()
```

* **Waveform FSM**: Generates a sample stream (sine / square / sawtooth / single ramp) at a configured sample period and modulation period; writes `sig_waveform_value`.
* **PWM Output FSM**: Periodically applies `sig_waveform_value` to the PWM pin with clamp, optional minimum, and **slew limiter**; writes `sig_pwm_applied` and increments `sig_pwm_update_counter` each update.
* **RPM FSM**: Counts optical encoder pulses (from ISR) and computes RPM either on a **fixed period** or **synchronized** to PWM windows using `sig_pwm_update_counter`; writes `sig_rpm`.
* **Data Logger FSM**: One logger with two modes—**free-run** (fixed cadence) or **PWM-synchronized**—printing selected columns (time, waveform, PWM, RPM) as TSV.
* **User Input FSM**: Debounces a pushbutton, samples a potentiometer, and parses simple serial **key=value** commands; writes `sig_input_*`.
* **User Output FSM**: Drives RGB LEDs and a piezo via `tone()` (continuous or timed) or digital toggling; consumes `sig_led_*` and `sig_buzzer_*`.


**How do the parts come together?**

1. **Waveform → PWM:** The Waveform FSM produces a **desired duty** (`sig_waveform_value`).
2. **PWM applies (with safety):** PWM FSM clamps, optionally lifts small values, and **slew-limits** before writing the pin. It also increments `sig_pwm_update_counter` on each update.
3. **RPM sampling:** RPM FSM measures pulses either on a fixed period or **in sync with PWM windows** (edge-to-edge), then writes `sig_rpm`.
4. **Logging:** Data Logger prints TSV lines either periodically or per PWM window, with selectable columns for **time / waveform / PWM / RPM**.
5. **User input/output:** The User Input FSM updates `sig_input_*` from the **button, pot, and serial**. `sig_led_*` and the buzzer signals are also globally declared, and are driven by User Output FSM. Any state machine can access them. 
6. `MotorCalib.ino` is the “composer”: it declares the **shared global variables**, configures and attaches the **encoder ISR**, leaves `setup()` empty (FSMs self-init on first call), and in `loop()` calls the FSMs in order, e.g.
  `Waveform → PWM Output → RPM → Datalogger → User Input → User Output`.
  7. Each FSM is **non-blocking** and advances via timers and `switch(state)`; they read/write only the documented globals, so files remain independent and reusable across projects.


## Tips for Labs & Testing

* **Single-module tests:** Comment out others and run one FSM at a time; use the Logger in free-run mode to watch behavior.
* **Traceability:** Each FSM’s header in code documents details **task, inputs (signals), outputs (signals), usage, tests, and how to extend**— explore and study the information in each module.
* **Implement the use case:** Set Waveform to **SINGLE\_RAMP** (no repeat) to capture a full PWM→RPM calibration sweep.


---

## Annex 1: Full reference code base
The full reference code base is available for download for course participants on the courses moodle platform, pls check.

## Annex 2: Calibration Controller

The **calibration controller**  Arduino file `motorCalib.ino` is the project’s orchestration point. It **owns the shared signals** (simple global variables), **defines the encoder ISR**, and **runs all finite-state machines (FSMs) once per loop** in a fixed, non-blocking order. Each FSM keeps its own local configuration and internal timing; the controller only wires them together through a few globals.

**Tasks**

* **Shared globals:** declare the small set of cross-module variables (e.g., `sig_waveform_value`, `sig_pwm_applied`, `sig_pwm_update_counter`, `sig_encoder_pulses`, `sig_rpm`, user I/O and output intents). 
* **Interrupt ownership:** define the **ISR** (increments `volatile unsigned long sig_encoder_pulses`) and the **encoder pin**. The **attach/detach** is performed by the RPM FSM during its `INIT` state (single source of truth for mode/edge), while the controller remains the canonical place for the ISR symbol and counter.
* **Execution model:** cooperative, **non-blocking** scheduling—every FSM is called once per `loop()`. Each FSM advances via timers/guards (no `delay()`), so tasks interleave predictably.
* **Call order (establishes data flow):**
  `Waveform → PWM Output → RPM → Datalogger → User Input → User Output`.
  This ordering ensures:

  * PWM reads the latest waveform sample.
  * RPM can **couple** its measurement window to PWM updates via `sig_pwm_update_counter`.
  * The Datalogger prints values computed **this** cycle.
  * User input/output are applied every cycle without blocking others.

**Timing & determinism**

* All timing is **explicit** (period checks inside each FSM). Loop frequency can vary; behavior does not.
* The only asynchronous source is the **encoder ISR**; access its counter with `volatile` and brief critical sections (already handled inside the RPM FSM).

**Code:**   [motorCalib.ino](code/motorCalib/motorCalib.ino)


## Annex 3: Global signals

These are the only cross-module variables. Each FSM **reads** and/or **writes** them.

| Signal                        | Type                     | Producer(s)  | Consumer(s)        | Meaning                                        |
| ----------------------------- | ------------------------ | ------------ | ------------------ | ---------------------------------------------- |
| `sig_waveform_value`          | `byte`                   | Waveform FSM | PWM Output, Logger | Desired duty sample (0..255)                   |
| `sig_pwm_applied`             | `byte`                   | PWM Output   | Logger             | Actually written PWM (0..255)                  |
| `sig_pwm_update_counter`      | `unsigned long`          | PWM Output   | RPM FSM, Logger    | Increments each time PWM is updated            |
| `sig_encoder_pulses`          | `volatile unsigned long` | **ISR**      | RPM FSM            | Rising-edge pulse counter from slot sensor     |
| `sig_rpm`                     | `float`                  | RPM FSM      | Logger             | Latest RPM estimate                            |
| `sig_input_button`            | `byte`                   | User Input   | Any                | Debounced button (0/1)                         |
| `sig_input_pot`               | `int`                    | User Input   | Any                | Potentiometer ADC (0..1023)                    |
| `sig_input_key`               | `char`                   | User Input   | Any                | Last serial key                                |
| `sig_input_value`             | `long`                   | User Input   | Any                | Last serial value                              |
| `sig_input_has_cmd`           | `bool`                   | User Input   | Any                | Latched “new command available”                |
| `sig_led_r/g/b`               | `byte`                   | Any          | User Output        | RGB LED on/off commands                        |
| `sig_buzzer_tone_enable`      | `bool`                   | Any          | User Output        | Use `tone()` if true                           |
| `sig_buzzer_tone_freq_hz`     | `unsigned int`           | Any          | User Output        | Tone frequency                                 |
| `sig_buzzer_tone_ms`          | `unsigned int`           | Any          | User Output        | 0=continuous, else one-shot duration           |
| `sig_buzzer_toggle_enable`    | `bool`                   | Any          | User Output        | Use digital toggling if true and tone disabled |
| `sig_buzzer_toggle_period_ms` | `unsigned int`           | Any          | User Output        | Half-period for toggling                       |

---

## Annex 4: FSM modules explained

Below a concise UML chart for each FSM (states, transitions, key events/activities). These match the code structure.

### Waveform Generator FSM 

**Task:** “Tick at a steady pace and compute the next point of the chosen wave.”

```{mermaid}  
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : after setup
  WAIT_TICK --> COMPUTE : every sample period
  COMPUTE --> WAIT_TICK : periodic waves (sine/square/saw) or ramp still running
  COMPUTE --> HOLD : single ramp finished AND not repeating
  HOLD --> WAIT_TICK : if restarted


```

**Events:** timer elapsed.
**Activities:** compute waveform from `phase = (now - t0) % MOD_PERIOD`.

**Code:** 

[void WaveformFSM_update()](code/motorCalib/WaveformFSM.ino)

---

### PWM Output FSM

**Task:** “At a fixed pace, update PWM for the motor; and limit how fast it changes.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : after setup
  WAIT_TICK --> UPDATE : every PWM update period
  UPDATE --> WAIT_TICK : after writing PWM (with clamps & slew)

```

**Events:** timer elapsed.
**Activities:** enforce `MAX_DUTY`, optional min, **slew limit** in logical domain; write PWM & signals.

**Code:** 

[PWMOutputFSM_update()](code/motorCalib/PWMOutputFSM.ino)

---

### RPM FSM 

**Task:** “Open a measurement window, count pulses, compute RPM; windows can be timed or aligned with PWM updates.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> IDLE : after setup
  IDLE --> START_WINDOW : MODE=coupled AND PWM updated
  IDLE --> START_WINDOW : MODE=free-run AND timer elapsed
  START_WINDOW --> END_WINDOW : window started
  END_WINDOW --> COMPUTE : MODE=coupled AND next PWM update
  END_WINDOW --> COMPUTE : MODE=free-run AND window time elapsed
  COMPUTE --> IDLE : after RPM computed

```

**Events:** PWM edge via `sig_pwm_update_counter` (coupled) or periodic timer (free-run).
**Activities:** snapshot or reset pulse counter; compute RPM (guard for very short `dt`).

**Code:** 

[void RPM_FSM_update()](code/motorCalib/RpmFSM.ino)

---

### Data Logger FSM 

**Task:**  “Either log on a timer, or once per PWM window—only the columns you enabled.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> IDLE : after setup
  IDLE --> LOG : MODE=coupled AND PWM updated
  IDLE --> LOG : MODE=free-run AND log period elapsed
  LOG --> IDLE : after one line printed

```

**Events:** PWM edge or fixed timer; 
**Activities:** print selected fields; reprint header if layout changes.

**Code:**
[void DataLogger_FSM_update()](code/motorCalib/DataLoggerFSM.ino)



---

### User Input FSM

**Task:** “Continuously read button and knob; when a full serial line arrives, parse key/value.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : after setup
  WAIT_TICK --> SCAN_INPUTS : serial data available
  SCAN_INPUTS --> PARSE_LINE : end-of-line reached
  SCAN_INPUTS --> WAIT_TICK : no complete line yet
  PARSE_LINE --> WAIT_TICK : after key/value latched

```

**Events:** debounce timeout, sample timer, serial EOL.
**Activities:** normalize active-low button to 0/1; capture ADC; parse `char=value`.

**Code:** 

[void UserInput_FSM_update()](code/motorCalib/UserInputFSM.ino)

---

### User Output FSM

**Task:** “At a steady pace, drive LEDs; make sound via tone() or simple on/off toggling.”

```{mermaid}
stateDiagram-v2
  [*] --> INIT
  INIT --> WAIT_TICK : after setup
  WAIT_TICK --> APPLY : every output update period
  APPLY --> WAIT_TICK : after LEDs/buzzer refreshed

```

**Events:** timer elapsed.
**Activities:** `tone()` continuous/one-shot, or software toggle; active-high/low LED handling.

**Code:** 

[void UserOutput_FSM_update()](code/motorCalib/UserOutputFSM.ino)


