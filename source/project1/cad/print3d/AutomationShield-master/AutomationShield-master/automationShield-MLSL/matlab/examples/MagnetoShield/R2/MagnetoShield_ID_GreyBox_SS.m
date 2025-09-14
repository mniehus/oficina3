%   MagnetoShield grey-box linearized state space system
%   identification example
% 
%   This example takes a measurement from the 
%   MagnetoShield device, where the permanent magnet is
%   levitatated around the 15 mm setpoint in closed-loop
%   using PID feedback. The experiment contains the 
%   input voltage, and two outputs: position and current.
%   The identification assumes that the dynamics can be 
%   represented using third order state-space model, 
%   that is based on the second order mechanical 
%    equation and the first order electrical
%   equation. Most literature assumes that the 
%   inductance L of the solenoid is fixed 
%   (fixedInductance=1), here you also may choose to
%   model it as a distance-dependent variable 
%   (fixedInductance=0). The resulting model will be in
%   "delta" form, that is all variables relate to 
%   linearization point.

%   This code is part of the AutomationShield hardware and software
%   ecosystem. Visit http://www.automationshield.com for more
%   details. This code is licensed under a Creative Commons
%   Attribution-NonCommercial 4.0 International License.
% 
%   Created by Gergely Takács. 
%   Last update: 15.02.2018.
  
startScript;                                    % Clears screen and variables, except allows CI testing

fixedInductance=0;                              % Fixed or distance dependent inductance?
                                                % to mathematical model
load ID_PID_4000us.mat                          % Load data file
Ts=0.004;                                       % [s] Sampling
y=result(:,1)/1000;                             % [m] Output in meters
u=result(:,2);                                  % [V] Input is closed loop + probe signal
i=result(:,3)/1000;                             % [A] Current

%% System identification data object
data = iddata([y i],u,Ts,'Name','Magnetic Levitation');    % Data file
data.InputName = 'Solenoid';                    % Input name: Solenoid voltage
data.InputUnit =  'V';                          % Input unit
data.OutputName{1} = 'Position';                % Output 1 name
data.OutputUnit{1} = 'm';                       % Output 1 unit
data.OutputName{2} = 'Current';                 % Output 2 name
data.OutputUnit{2} = 'A';                       % Output 2 unit
data.Tstart = 0;                                % Starting time
data.TimeUnit = 's';                            % Time unit
data = data(100:end);                           % Discard the time when magnet is on ground, pick close to linearization point
data=detrend(data);                             % Discard offset                               
dataf = fft(data);                              % Frequency domain 

%% Parameters
m = 0.76E-3;                                    % [kg] Magnet mass
R = 198.3;                                      % [Ohm] Solenoid resistance
L = 0.239;                                      % [H] Solenoid inductance
u0=mean(u);                                     % [V] Input linearization around setpoint
y0=mean(y);                                     % [m] Output (position) linearization around setpoint
Km=2E-6;                                        % Magnetic constant (rhough estimate)
    
% Initial parameters for linearized model
h0=data.y(1,1);                                 % Initial position estimate
dh0=(data.y(2,1)-data.y(1,1))/Ts;               % Initial velocity estimate
i0=data.y(1,2);                                 % Initial current estimate
alpha=2*(Km/m)*u0^2/y0^3;                       % Linearized parameter guess
beta=2*(Km/m)*u0/y0^2;                          % Linearized parameter guess
gamma=2*Km*mean(i)/(L*y0^2);                    % Linearized parameter guess
delta=R/L;                                      % Parameter guess
epsilon=1/L;                                    % Parameter guess

%% Construct model
if fixedInductance==1                           % Magnet inductance L fixed
    A=[0       1    0;
       alpha   0   -beta
       0       0   -delta];

elseif  fixedInductance==0                      % Magnet inductance L(y) distance dependent 

    A=[0       1        0;                      % Dybamic matrix initial guess
       alpha   0       -beta
       0       gamma   -delta];     
end

B=[0; 0; epsilon];                              % Input matrix
C=[1 0 0;                                       % Output matrix
   0 0 1];                                      % Distance and current measured
D=[0; 0];                                       % No feed-through                                            
K = zeros(3,2);                                 % Disturbance
x0=[h0; dh0; i0];                               % Initial condition
disp('Initial guess:')
sys=idss(A,B,C,D,K,x0,0)                        % Construct state-space representation


% Mark the free parameters
if fixedInductance==1                           % Magnet inductance L fixed
    sys.Structure.A.Free=      [0     0    0;   % Free and fixed variables
                                1     0    1;
                                0     0    1];
elseif fixedInductance==0                       % Magnet inductance L(y) distance dependent
    sys.Structure.A.Free=      [0     0    0;   % Free and fixed variables
                                1     0    1;
                                0     1    1];
end
                      
sys.Structure.B.Free=  [0  0 1]';               % Free and fixed variables
sys.Structure.C.Free=  false;                   % No free parameters

sys.DisturbanceModel = 'estimate';              % Estimate disturbance model
sys.InitialState = 'estimate';                  % Estimate initial states

%% Set estimation options
Options = ssestOptions;                         % State-space estimation options
Options.Display = 'on';                         % Show progress
Options.Focus = 'simulation';                   % Identification focus
Options.EnforceStability = 0;                   % Unstable model
Options.InitialState = 'estimate';              % Estimate initial condition

%% Estimate and list parameters
disp('Identified model:')
model = ssest(dataf,sys,Options)                % Launch estimation procedure
compare(dataf,model)                            % Compare data to model

disp('---Parameter Comparison---')
Ld=['L (measured): ',num2str(L),' [H],   L (model): ',num2str(1/model.b(3),3),' H'];
disp(Ld)
Rd=['R (measured): ',num2str(R),' [Ohm], R (model): ',num2str(-model.a(3,3)/model.b(3),3),' [Ohm]'];
disp(Rd)
if fixedInductance==0 
   
Ke=mean([model.a(2,1)*2*m*y0^3/u0^2 -(model.a(2,3)*2*m*y0^2)/u0]);
Kstd=std([model.a(2,1)*2*m*y0^3/u0^2 -(model.a(2,3)*2*m*y0^2)/u0]);
Kd=['K (guess): ',num2str(Km), ',    K (model,mean): ',num2str(Ke,1),' +/- ',num2str(Kstd,1)];
disp(Kd)
disp(['K (range): ',num2str([model.a(2,1)*2*m*y0^3/u0^2 -(model.a(2,3)*2*m*y0^2)/u0])])
end