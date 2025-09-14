function [dx, y] = BOBShieldSin2_ODE(t, x, u, J,R,m,M,g, varargin)
%   ODE file representing the dynamics of the FloattShield device.
%   Reference: Chaos, D. et al - Robust switched control of an air
%   levitation system with minimum sensing, ISA Transactions, 2019
%
%   This code is part of the AutomationShield hardware and software
%   ecosystem. Visit http://www.automationshield.com for more
%   details. This code is licensed under a Creative Commons
%   Attribution-NonCommercial 4.0 International License.
% 
%   Created by Martin Gulan and Gergely Tak�cs.

% States: x1 - ball position, x2 - ball speed, x3 - airspeed inside the tube
% Input:  u - fan voltage
% Output: y - ball position
%2nd order nonlinear ver 2
y = x(1);                                               % Output equation
dx(1) = x(2);                                           % State equation...
dx(2) =(M*x(1)*(u*(pi/180))^2-m*g*sin(u*(pi/180)))/(J/R^2+m); % ...

