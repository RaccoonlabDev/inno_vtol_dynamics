# Quadcopter dynamics

PX4 is based on FRD notation and has the following motors enumeration:

<img src="https://dev.px4.io/master/assets/airframes/types/QuadRotorX.svg" alt="drawing" width="150">

The dynamics library is based on FLU notation and has the following motors enumeration:

|||||
|-|-|-|-|
| 0 | front left  | positive moment around | +0.08, +0.08
| 1 | tail left   | negative moment around | -0.08, +0.08
| 2 | tail right  | positive moment around | -0.08, -0.08
| 3 | front right | negative moment around | +0.08, -0.08