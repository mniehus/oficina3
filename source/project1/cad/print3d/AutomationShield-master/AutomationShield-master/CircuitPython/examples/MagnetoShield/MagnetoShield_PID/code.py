"""
  MagnetoShield PID example

  This example implements a PID feedback control for the
  magnet levitation using a CircuitPython and a compatible
  board.

  This example imports the sampling and PID control
  modules from the AutomationShield CircuitPython library.
  You may select whether the reference is given by the
  potentiometer or you want to test a predetermined
  reference trajectory. Save the code to your board,
  including the other necessary modules and post-plot
  the results in Mu (too fast otherwise) or use an
  external serial terminal program

  Tested with the following boards
  - Adafruit Metro M4 Express (1),(4)
  - Adafruit Metro M0 Express (2),(3), (4)
  - Adafruit Metro M4 Grand Central (1),(4)

  (1) Runs as expected. Mu cannot keep up with data output, use external means to log.
  (2) Runs only without serial output.
  (3) Some degradation visible, increase sampling if possible
  (4) Strict real-time not achievable

  If you have found any use of this code, please cite our work in your
  academic publications, such as thesis, conference articles or journal
  papers. A list of publications connected to the AutomationShield
  project is available at:
  https://github.com/gergelytakacs/AutomationShield/wiki/Publications

  This code is part of the AutomationShield hardware and software
  ecosystem. Visit http://www.automationshield.com for more
  details. This code is licensed under a Creative Commons
  Attribution-NonCommercial 4.0 International License.

  Created by:       Gergely Takács
  Created on:       12.10.2020.
  Last updated by:  Gergely Takács
  Last update:      26.10.2020.
"""

import MagnetoShield                            # Imports the MagnetoShield module for hardware functionality
import Sampling                                 # Imports the Sampling module for pseudo-real time sampling
import PIDAbs                                   # Imports the PIDAbs module for the absolute PID algorithm
import time                                     # Imports the time module for delays

MANUAL = False                                  # Reference by pot (True) or automatically (False)?
DATA_OUTPUT = True                              # Only experiment (False) or with data dumped to serial (True)
PLOTTING_POST = False                           # Does not supply data while the experiment is running, only does it after it is finished
                                                # this helps Mu Plotter not to be flooded. You will only see Y and U plotted. As an alternative
                                                # use an external serial program like CoolTerm.

# Sampling rate and PID Tuning
Ts = int(5000)                                  # [ms] Sampling in microseconds, lower limit unknown for the M4 Express

# PID Tuning
KP = 3.5                                        # PID Kp (proportional constant)
TI = 0.6                                        # PID Ti (integral time constant)
TD = 0.025                                      # PID Td (derivative time constant)

R = [14.0, 13.0, 14.0, 15.0, 14.0]              # [mm] Desired reference trajectory (pre-set)
T = int(1000)                                   # [steps] Experiment section length
r = R[0]                                        # Initial reference

# Fallback for slower processors, since sampling cannot be kept up with desired speed
def fallbackSettings():                                 # Used later, comment out the function call if not needed.
    import microcontroller                              # Imports the microcontroller module so that CPU speed can be determined
    global Ts, KP, TI, TD, R, T, DATA_OUTPUT            # Makes these global to be settable
    if (microcontroller.cpu.frequency/1000000 == 48):   # For the Adafruit Metro M0 @48 MHz override defaults
        Ts = int(6000)                                  # [ms] Sampling in microseconds, lower limit unknown for the M0 Express
        KP = 2.0                                        # PID Kp (proportional constant)
        TI = 0.4                                        # PID Ti (integral time constant)
        TD = 0.02                                       # PID Td (derivative time constant)
        R = [14.0, 14.0]                                # [mm] Desired reference trajectory (pre-set)
        T = int(2500)                                   # [steps] Experiment section length
        DATA_OUTPUT = False                             # Disable logging output

if PLOTTING_POST:                               # If the plotter of Mu is used, this speed will flood it, so plot it later.
    Ylog = []                                   # Empty list to store output results
    Ulog = []                                   # Empty list to store input results

k = int(1)                                      # Sample index
i = int(1)                                      # Experiment section counter

# Initialize and calibrate board
MagnetoShield.begin()                           # Lock I2C bus
MagnetoShield.calibration()                     # Calibrate device
fallbackSettings()                              # These are only active when CPU speed is 48 MHz. Comment if you want to use settings as above

# Set the PID settings
PIDAbs.Settings.setKp(KP)                       # Proportional
PIDAbs.Settings.setTi(TI)                       # Integral
PIDAbs.Settings.setTd(TD)                       # Derivative
PIDAbs.Settings.setTs(Ts)                       # Sampling (use Ts in microseconds)

Sampling.begin(Ts)                              # Initialize sampling subsystem (based on time.monotonic_ns())

# Algorithm step - every step that is necessary for control
def step():
    global r, R, i, k                           # Access these global variables
    if MANUAL:                                  # If reference from potentiometer
        r = AutomationShield.mapFloat(MagnetoShield.referenceRead(), 0.0, 100.0, 12.0, 17.0)
    else:                                       # if pre-set experiment
        if (k > (len(R) * T) - 1):              # if the experiment is overs
            Sampling.Settings.realTimeViolation = False  # Not a real-time violation
            MagnetoShield.actuatorWrite(0.0)    # then turn off magnet
            if DATA_OUTPUT:                     # if outputs are requested
                if PLOTTING_POST:                   # In case plotting in post is enabled
            	    for j in enumerate(Ylog):             # for every element in the log vector of outputs
                        print((Ylog[j[0]],Ulog[j[0]],))   # Print to serial
                        time.sleep(0.03)            # Wait a bit so that Mu plotter can catch up
            while True:                         # then stop
                pass                            # and do nothing
        else:                                   # if the experiment is not yet over
            if (k % (T*i) == 0):                # else for each section
                    r = R[i]                    # set reference
                    i += 1                      # and increase section counter for next

    y = MagnetoShield.sensorRead()              # [mm] sensor read routine
    u = PIDAbs.compute(-(r-y), 0.0, 10.0, -10.0, 10.0) #Compute constrained absolute-form PID
    MagnetoShield.actuatorWrite(u)              # [V] write input to actuator
    if DATA_OUTPUT:
        if PLOTTING_POST:                       # If we are plotting after the experiment
            Ylog.append(y)                      # append output y to output vector
            Ulog.append(u)                      # append input u to input vector
        else:                                   # otherwise we are plotting "real time"
            print((r, y, u))                    # send data to output and
    k += 1                                      # Increment time-step k

# Main loop launches a single step at each enable time
while True:                                     # Infinite loop
    Sampling.stepEnable()                       # Routine to enable the algorithm step, changes the flag Sampling.enable
    if Sampling.Settings.enable:                # If time comes
        step()                                  # Algorithm step
        Sampling.Settings.enable = False        # Then disable
