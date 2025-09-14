%   MotoShield Lag Compensator Design based on Transfer Function.
%
%   This code is part of the AutomationShield hardware and software
%   ecosystem. Visit http://www.automationshield.com for more
%   details. This code is licensed under a Creative Commons
%   Attribution-NonCommercial 4.0 International License.
% 
%   Created by Ján Boldocký.

startScript;
%%
load MotoShield_GreyboxModel_TF.mat
margin(model) %
s=tf('s');
C=0.001*(s+15)/(s+0.05);
control_sys=feedback(0.01*model,1)
opt = stepDataOptions('InputOffset',0,'StepAmplitude',1);
Y=step(control_sys,opt);
Cd=c2d(C,0.02)
dcontrol_sys = feedback(Cd*zmodel,1);
step(dcontrol_sys);
figure
margin(C)
%margin(control_sys)

