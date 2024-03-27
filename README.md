# Simulation of a reservoir with arduino + state design pattern

![Screenshot 2024-03-27 031502](https://github.com/MohamedAmr982/ReservoirSimulator/assets/16720762/8aebc957-b645-405e-a84f-65d1fdd39d03)

The Charging and discharging, are controlled by setting the position of the servo angle to control the action (charging, discharging,
idle and the rate). The operator has two potentiometers to change the rates, one for charging, the other for
discharging. The operator has a push button to switch from charging, discharging and idle. There is an RGB led to
indicate current action. The system checks the limits (fully charged, completely discharged), and switch to idle,
and activate a buzzer alarm.

The system obeys the following state diagram:

![Screenshot 2024-03-27 033009](https://github.com/MohamedAmr982/ReservoirSimulator/assets/16720762/868d3a23-1328-4045-8492-3d30715d43da)

The state machine was implemented using state design pattern in C:
![Reservoir (1)](https://github.com/MohamedAmr982/ReservoirSimulator/assets/16720762/25498082-f155-498e-b03c-8300133e89a8)

Where 'Context' is a struct with all important variables in the system, which delegates state-specific logic (variable updating according to the current state, transition between states based on conditions, etc.) to the 'State' struct. The State by default has pointers to default behavior functions.

There are three concrete states in the system (idle, charging, and discharging), each has functions that override the default state behavoir. Each state has its own 'constructor' (idle(), charging(), and discharging()) which are passed a state struct. To implement the 'overriding', the constructor function just replaces the default function pointer with a pointer to the concrete function of the state.
