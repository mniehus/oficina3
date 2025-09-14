import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import solve_ivp

# Physical constants
g = 9.81       # gravity (m/s^2)
L = 1.0        # length of pendulum (m)
b = 0.5        # damping coefficient (kg/s)
m = 1.0        # mass (kg)

# Time span
t_span = (0, 10)
t_eval = np.linspace(*t_span, 1000)

# Initial conditions: starts inverted (theta=pi), no initial velocity
theta0 = np.pi
omega0 = 0

# Damped pendulum ODE system
def damped_pendulum(t, y):
    theta, omega = y
    dtheta_dt = omega
    domega_dt = -(b/m)*omega - (g/L)*np.sin(theta)
    return [dtheta_dt, domega_dt]

# Solve ODE
sol = solve_ivp(damped_pendulum, t_span, [theta0, omega0], t_eval=t_eval)

theta = sol.y[0]
omega = sol.y[1]
alpha = np.gradient(omega, t_eval)  # numerical derivative for angular acceleration

# Plotting
fig, axs = plt.subplots(3, 1, sharex=True, figsize=(10, 8))
fig.suptitle("Damped Pendulum from Inverted Position")

axs[0].plot(t_eval, theta, label='Angle (rad)', color='blue')
axs[0].set_ylabel("Angle (rad)")
axs[0].grid(True)

axs[1].plot(t_eval, omega, label='Angular Velocity (rad/s)', color='orange')
axs[1].set_ylabel("Angular Velocity (rad/s)")
axs[1].grid(True)

axs[2].plot(t_eval, alpha, label='Angular Acceleration (rad/s²)', color='green')
axs[2].set_xlabel("Time (s)")
axs[2].set_ylabel("Angular Acceleration (rad/s²)")
axs[2].grid(True)

plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.show()
