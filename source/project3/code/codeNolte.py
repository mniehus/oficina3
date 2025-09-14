#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PenInverted.py
Created on Friday Sept 11 06:03:32 2020
@author: nolte
D. D. Nolte, Introduction to Modern Dynamics: Chaos, Networks, Space and Time, 2nd ed. (Oxford,2019)
"""
 
import numpy as np
from scipy import integrate
from matplotlib import pyplot as plt
 
plt.close('all')
 
print(' ')
print('PenInverted.py')
 
F = 133.5          # 30 to 140  (133.5)
delt = 0.000       # 0.000 to 0.01
w = 20          # 20
def flow_deriv(x_y_z,tspan):
    x, y, z = x_y_z
    a = y
    b = -(1 + F*np.cos(z))*np.sin(x) - delt*y
    c = w
    return[a,b,c]
                 
T = 2*np.pi/w
 
x0 = np.pi+0.3
v0 = 0.00
z0 = 0
 
x_y_z = [x0, v0, z0]
 
# Solve for the trajectories
t = np.linspace(0, 2000, 200000)
x_t = integrate.odeint(flow_deriv, x_y_z, t)
siztmp = np.shape(x_t)
siz = siztmp[0]
 
#y1 = np.mod(x_t[:,0]-np.pi,2*np.pi)-np.pi
y1 = x_t[:,0]
y2 = x_t[:,1]
y3 = x_t[:,2]    
 
plt.figure(1)
lines = plt.plot(t[0:2000],x_t[0:2000,0]/np.pi)
plt.setp(lines, linewidth=0.5)
plt.show()
plt.title('Angular Position')
 
plt.figure(2)
lines = plt.plot(t[0:1000],y2[0:1000])
plt.setp(lines, linewidth=0.5)
plt.show()
plt.title('Speed')
 
repnum = 5000
px = np.zeros(shape=(2*repnum,))
xvar = np.zeros(shape=(2*repnum,))
cnt = -1
testwt = np.mod(t,T)-0.5*T;
last = testwt[1]
for loop in range(2,siz-1):
    if (last < 0)and(testwt[loop] > 0):
        cnt = cnt+1
        del1 = -testwt[loop-1]/(testwt[loop] - testwt[loop-1])
        px[cnt] = (y2[loop]-y2[loop-1])*del1 + y2[loop-1]
        xvar[cnt] = (y1[loop]-y1[loop-1])*del1 + y1[loop-1]
        last = testwt[loop]
    else:
        last = testwt[loop]
  
plt.figure(3)
lines = plt.plot(xvar[0:5000],px[0:5000],'ko',ms=1)
plt.show()
plt.title('First Return Map')
 
plt.figure(4)
lines = plt.plot(x_t[0:1000,0]/np.pi,y2[0:1000])
plt.setp(lines, linewidth=0.5)
plt.show()
plt.title('Phase Space')