## Tests


Tests that need to be realized

Compare PID controller and LQR controller
In this project, we explore and compare the performance of two different control strategies: PID (Proportional-Integral-Derivative) controller and LQR (Linear Quadratic Regulator) controller.
<br>
![image](https://github.com/B-Paweekorn/Reaction-wheel-inverted-pendulum/assets/122732439/714b7756-ad1f-469a-944b-afa281b2bc5b)

***Linear quadratic regulator***
<br>

![CodeCogsEqn](https://github.com/B-Paweekorn/Reaction-wheel-inverted-pendulum/assets/122732439/8ce60949-f467-4ec1-8616-7e2be18518ba)
| Error (deg)    | settling time (s) | Power (Watt) |
| ---      | ---       | --- |
| 5 |  0.66     | 0.6|
| 6 |  0.73|     1.01   |
| 7 |  0.85|1.75|
| 8 |  1.07|        3.5|
| 9 |  can't stabilize | can't stabilize |

Max Disturbance : **9.32 Nm**

***PID :*** Kp = 500 (choose form root locus)
| Error (deg)    | settling time (s) | Power (Watt) |
| ---      | ---       | --- |
| 5 |  20.43| 10.06|
| 6 |  21.02| 12.17 |
| 7 |  21.37| 13.95 |
| 8 |  21.59| 15.96 |
| 9 |  can't stabilize | can't stabilize |

Max Disturbance : **8.05 Nm**

***PID :*** Kp = 215800 (choose form root locus)
![image](https://github.com/B-Paweekorn/Reaction-wheel-inverted-pendulum/assets/122732439/2ecb7281-f6db-4a19-8914-fedf1de1eda6)
***Notice*** that when choose unstable pole the system still stable because now it have **hardware limit** so the character of controller same like Fuzzy logic control to see unstable you need to unlock hardware limitation by set ` MotorLimit ` to **False** in ` param.py `

**Conclusion**

|    | Stabilize boundary |
| ---      | ---       | 
| **LQR** |  Can stabilize in every position  |
| **PID** |  Can stabilize only in small boundary| 

**PID Controller**
  The PID controller is a widely used feedback control system that relies on three components: Proportional, Integral, and Derivative. Here's a brief overview of each component:
  
  - Proportional (P): Reacts to the current error.
  - Integral (I): Reacts to the accumulation of past errors.
  - Derivative (D): Predicts future errors based on the rate of change.

  **Advantages of PID:**
  - Simplicity and ease of implementation.
  - Effectiveness in a wide range of systems.
  - Intuitive tuning parameters for performance optimization.
  
  **Considerations:**
  - Tuning may be required for optimal performance in different systems.
  - Limited capability to handle complex or nonlinear systems.
<br>

  **LQR Controller**
  The LQR controller is designed based on the principles of optimal control theory. It minimizes a cost function that combines both state and control input, making it suitable for linear, time-invariant systems.

**Advantages of LQR:**
- Optimal control solution for linear systems.
- Ability to handle systems with multiple inputs and outputs.
- Incorporates a mathematical model for optimal performance.

**Considerations:**
- Requires a good understanding of the system dynamics for effective modeling.
- Limited applicability to strictly linear systems.
