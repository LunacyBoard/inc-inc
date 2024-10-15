# inc-inc
In C. In C. In RC.

rc_midi contains general midi routines
rc_smf  contains code for loading standard midi files


CYCLES_PM
---------
The CYCLES_PM value is based on RC2014 running at standard speed. It is used to calculate the actual delta time
 5000 --> 0:34  0:34
10000 --> 1:07  1:08
20000 --> 2:15  2:15

2 844 420 operations per min in wait_delta function

480 ppqn x 120 bpm = 57600 ppm  49
96  ppqn x 120 bpm = 11520 ppm  246

49 units per ppqn for 480

