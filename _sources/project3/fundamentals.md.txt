## Fundamentals

This section introduces the fundamental concepts and equations behind the Reaction Wheel Inverted Pendulum. It combines physics, electromechanics, and control theory.

---

### Dynamics

| Quantity      | Linear                         | Angular                         |
|---------------|--------------------------------|----------------------------------|
| Position      | $x$                            | $\theta$                         |
| Velocity      | $v = \frac{dx}{dt}$            | $\omega = \frac{d\theta}{dt}$    |
| Acceleration  | $a = \frac{dv}{dt}$            | $\alpha = \frac{d\omega}{dt}$    |

---

**Newton’s Laws for Linear Motion**

$$ 
F = m a
$$ 

- $F$: Force (N)  
- $m$: Mass (kg)  
- $a$: Acceleration (m/s²)

**Newton’s Laws for Rotational Motion**

$$ 
\tau = I \alpha
$$ 

- $\tau$: Torque (Nm)  
- $I$: Moment of inertia (kg·m²)  
- $\alpha$: Angular acceleration (rad/s²)

To rotate a body with **friction torque** $\tau_f$:

$$ 
\tau_{\text{net}} = I \alpha + \tau_f
$$ 

A spinning disk induces a **reaction torque**:

$$ 
\tau_{\text{reaction}} = -I \alpha
$$ 

Note the correspondence of linear vs angular motion parameters. 


### Moment of Inertia

For a **solid disk** of mass $m$ and radius $r$, rotating about its center:

 $$ 
I = \frac{1}{2} m r^2
 $$ 

For a **solid wheel** (annular disk) of mass $m$, inner radius $R_i$ and outer radius $R_a$:

 $$ 
I = \frac{1}{2} m (R_i^2 + R_a^2)
 $$ 

For a **point mass** $M$ at a distance $L$ from the axis of rotation (e.g., for a pendulum):

 $$ 
I = M L^2
 $$ 

For a **general rotationally symmetric object** with radial mass density $\rho(r)$ (in kg/m):

 $$ 
I = \int_{R_i}^{R_a} \rho(r) \, r^2 \cdot 2\pi r \, dr = 2\pi \int_{R_i}^{R_a} \rho(r) \, r^3 \, dr
 $$ 

For a **composite object** made of multiple components with known moments of inertia $I_1, I_2, \dots, I_n$:

 $$ 
I_{\text{total}} = I_1 + I_2 + \dots + I_n = \sum_{i=1}^n I_i
 $$ 


🔗 [Wikipedia: Moment of Inertia](https://en.wikipedia.org/wiki/Moment_of_inertia)


**Measuring Moment of Inertia in the Lab**

The **moment of inertia** of an object can be determined experimentally using various methods. One simple and effective method is the **torsional pendulum** or **rotational acceleration method**.

**Method 1: Torsional Oscillation**

1. **Suspend** the object from a thin wire or torsion spring, allowing it to rotate freely about a vertical axis.
2. **Measure the period** $T$ of small-angle oscillations.
3. **Determine the torsion constant** $k$ of the wire or system by calibrating with a known inertia or using a reference object.
4. Use the formula:
5. 
    $$ 
   I = \frac{T^2 k}{4\pi^2}
    $$ 

   where:
   - $I$ is the moment of inertia,
   - $T$ is the oscillation period,
   - $k$ is the torsional stiffness.

**Method 2: Angular Acceleration**

1. Apply a **known torque** $\tau$ (e.g., via a hanging mass and pulley).
2. Measure the resulting **angular acceleration** $\alpha$ (using a rotary encoder or sensor).
3. Use Newton's second law for rotation:
   
    $$ 
   I = \frac{\tau}{\alpha}
    $$ 

   - $\tau = r F$, where $F$ is the force applied at radius $r$.

---

### Power conversion

In a rotating system, energy and power are governed by the following key relations.

The **rotational kinetic energy** of a spinning body is:

 $$ 
E_{\text{rot}} = \frac{1}{2} I \omega^2
 $$ 

where:
- $I$ is the moment of inertia (in kg·m²),
- $\omega$ is the angular velocity (in rad/s).

The **mechanical power** delivered to or from a rotating body is:

 $$ 
P_{\text{mech}} = \tau \cdot \omega
 $$ 

where:
- $\tau$ is the torque (in N·m),
- $\omega$ is the angular velocity (in rad/s).

In electromechanical systems (e.g. DC motor + disk), the mechanical power must come from electrical input power. If $\eta$ is the efficiency of conversion:

$$ 
P_{\text{elec}} = \frac{P_{\text{mech}}}{\eta}
$$ 

and

$$ 
P_{\text{elec}} = V \cdot I
$$ 

where:
- $V$ is the supply voltage (in volts),
- $I$ is the current (in amperes),
- $\eta$ is the efficiency (e.g. 0.8 for 80%).

**Numerical Example**

Let’s consider a **solid disk** with:
- mass $m = 1$ kg,
- radius $r = 0.05$ m (10 cm diameter),
- angular velocity $\omega = 2\pi$ rad/s (i.e., 1 turn/s),
- angular acceleration $\alpha = 2\pi$ rad/s²,
- efficiency $\eta = 0.8$.

1. Moment of Inertia of the Disk:

 $$ 
I = \frac{1}{2} m r^2 = \frac{1}{2} \cdot 1 \cdot (0.05)^2 = 0.00125 \ kg·m^2
 $$ 


2. Kinetic Energy of Rotation:

 $$ 
E_{\text{rot}} = \frac{1}{2} I \omega^2 = \frac{1}{2} \cdot 0.00125 \cdot (2\pi)^2 \approx 0.049 \ \text{J}
 $$ 


3. Required Torque:
Assuming angular acceleration $\alpha = 2\pi$ rad/s²:

 $$ 
\tau = I \cdot \alpha = 0.00125 \cdot 2\pi \approx 0.00785 \ N·m
 $$ 


4. Mechanical Power:

 $$ 
P_{\text{mech}} = \tau \cdot \omega = 0.00785 \cdot 2\pi \approx 0.049 \ \text{W}
 $$ 


5. Electrical Power Required:
With 80% efficiency:

 $$ 
P_{\text{elec}} = \frac{P_{\text{mech}}}{\eta} = \frac{0.049}{0.8} \approx 0.061 \ \text{W}
 $$ 


>  This example highlights the relevance of inertia and torque in system design. Note also that the power levels required to drive mechanical motion are typically beyond what basic microcontrollers can provide, and as such it is required to use a transistor based driver circuit between the Arduino and the Dc motors. 


###  DC Motor Basics

DC motors convert electrical energy into mechanical torque and rotational motion. The fundamental equations are:

- Torque generated: 
-  
   $$ 
  \tau = k_t \cdot I
   $$ 

- Voltage across the motor:  
- 
   $$ 
  V = k_e \cdot \omega + I \cdot R
   $$ 


Where:
- $\tau$: torque (N·m)  
- $I$: current (A)  
- $V$: voltage (V)  
- $\omega$: angular velocity (rad/s)  
- $k_t$: torque constant (N·m/A)  
- $k_e$: back-EMF constant (V·s/rad)  
- $R$: motor coil resistance (Ω)

**Typical Parameters**

To correctly simulate and use a DC motor in a control system, you must know or estimate the following key parameters:

| Symbol      | Quantity                 | Units        | Typical Range (Lab Motors) |
|-------------|--------------------------|--------------|----------------------------|
| $\tau$      | Torque                   | N·m          | 0.01 – 0.2 N·m             |
| $I$         | Current                  | A            | 0.2 – 2 A                  |
| $V$         | Voltage                  | V            | 3 – 24 V                   |
| $\omega$    | Angular speed            | rad/s        | 100 – 2000 rad/s           |
| $k_t$       | Torque constant          | N·m/A        | 0.01 – 0.1 N·m/A           |
| $k_e$       | Back-EMF constant        | V·s/rad      | 0.01 – 0.1 V·s/rad         |
| $R$         | Coil resistance          | Ω            | 0.5 – 10 Ω                 |

---

Most of these values are provided in the motor's datasheet:

- **$k_t$ and $k_e$**: Often given directly. For brushed motors, these are numerically equal ($k_t = k_e$) if using SI units.
- **Stall torque** ($\tau_{\text{stall}}$): Maximum torque at zero speed.
- **No-load speed** ($\omega_{\text{free}}$): Speed at zero torque.
- **Stall current** ($I_{\text{stall}}$): Current at zero speed and maximum torque.
- **Resistance**: $R = V_{\text{stall}} / I_{\text{stall}}$

From these, you can compute:
- $k_t = \tau_{\text{stall}} / I_{\text{stall}}$
- $k_e = V_{\text{free}} / \omega_{\text{free}}$
  
**Trade-Offs in Motor Selection**

The performance of a DC motor is a balance between torque and speed. Key considerations:
- Higher torque → requires more current.
- Higher speed → increases back-EMF and voltage needs.
- Higher current → increases resistive losses ($I^2 R$) and risk of overheating.
- Motor constants ($k_t$, $k_e$) are typically fixed for a given model.

>  Always respect **maximum ratings**: exceeding current, voltage, or power may permanently damage the motor.

**Gears**

Gears can be used to **adapt motor output** to desired speed and torque levels.

**Basic Gear Equations**

- Gear ratio:  
- 
   $$ 
  R = \frac{N_{\text{out}}}{N_{\text{in}}}
   $$ 

  where $N$ is the number of teeth or diameter.

- Output torque:  
- 
   $$ 
  \tau_{\text{out}} = R \cdot \tau_{\text{in}}
   $$ 


- Output speed: 
-  
   $$ 
  \omega_{\text{out}} = \frac{\omega_{\text{in}}}{R}
   $$ 


**Why Use Gears?**

- **Increase torque** at the cost of lower speed.
- **Reduce load** on the motor to avoid overheating.
- **Match the dynamics** of the controlled system (e.g. pendulum).

**Pulleys (with belts)** provide similar mechanical advantages as gears.They are quieter than gears, and the distance between shafts is more flexible, so systems with pulleys and belts they are more simple to design and align. At the same time, they are less precise (risk of belt slippage), and limited to moderate torque applications.

**Example: Gear Matching**

Suppose your pendulum needs:
- Torque of $0.1$ N·m
- Speed around $30$ rad/s

But your available motor produces:
- Torque of $0.02$ N·m
- Speed of $150$ rad/s

Then use a gear ratio of:

 $$ 
R = \frac{0.1}{0.02} = 5
 $$ 


This brings output torque up to $0.1$ N·m, and reduces speed to:

 $$ 
\omega_{\text{out}} = \frac{150}{5} = 30 \ \text{rad/s}
 $$ 




---

### Sensor basics

**Magnetic Position Sensor (AS5600)**

- Contactless rotary sensor using a magnetic field.
- Measures absolute angle with high resolution.
- Outputs via I2C or PWM.

 [AS5600 Datasheet](https://ams.com/as5600)

---


### Controller

A control system aims to keep the pendulum balanced using feedback.

Main Elements

- **Plant**: The physical system to be controlled (e.g., pendulum)
- **Controller**: Algorithm producing control signals
- **Sensor**: Measures the system state
- **Actuator**: Applies control input (motor)
- **Feedback**: Uses sensor data to adjust behavior

Feedback Control

Continuously reduces error by adjusting inputs based on outputs.

- **Bang-bang**: All-or-nothing control; fast but unstable.
- **PID**: Combines:
  
 $$ 
  u(t) = K_p e(t) + K_i \int e(t) \, dt + K_d \frac{de(t)}{dt}
 $$   

- **LQR**: Uses optimal gains to minimize cost function:
- 
   $$ 
  u = -Kx
   $$ 

  where $x$ is the system state and $K$ is the gain matrix.


### Arduino System Integration

Arduino microcontrollers are widely used for real-time control systems due to their simplicity, low cost, and large ecosystem of libraries and tutorials. In this project, the Arduino is responsible for:

- Reading sensor data (e.g., from the AS5600 magnetic rotary sensor)
- Running the control algorithm (Bang-Bang, PID, or LQR)
- Sending the command to the motor driver to actuate the reaction wheel

**Typical Setup Components**

- **Microcontroller**: Arduino Uno / Nano / Mega  
- **Sensor**: AS5600 magnetic rotary position sensor (via I2C)  
- **Actuator**: Brushed DC motor + motor driver (e.g., L298N or TB6612FNG)  
- **Power supply**: Appropriate for motor and controller (check ratings!)  
- **Other**: Wires, resistors, capacitors, breadboards or PCBs  

### Programming

Migrating from Python Simulation to Arduino Deployment

In your development process, you will first simulate the control system (e.g., pendulum dynamics and reaction wheel behavior) using Python. This allows you to:

- Explore different parameters (e.g., control gains, system constants)
- Test various controllers (Bang-Bang, PID, LQR)
- Validate system stability in idealized and perturbed conditions

Once the control strategy is selected and tuned in Python, **you will port the core control logic to C/C++ in the Arduino IDE**:

1. **Isolate the control algorithm** from your Python code (e.g., a function that outputs control action given angle and velocity).
2. **Rewrite this logic in Arduino-compatible C++**, respecting memory and computation constraints.
3. Replace Python’s simulated sensor and motor interfaces with real-time reads from the AS5600 and motor driver control (e.g., `analogWrite()` or `digitalWrite()`).
4. Use Arduino’s `loop()` and `millis()` to structure periodic execution of your control loop (e.g., 100 Hz update rate).
5. Calibrate your sensors and actuators to reflect real-world behavior.

>  Real hardware behaves differently than simulations — expect to tune your controller gains again!

By mirroring your software architecture between simulation and deployment, you ensure a smoother transition and better reuse of your logic.