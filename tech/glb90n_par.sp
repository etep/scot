*THE CAPS IS IN fF UNLESS MENTIONED OTHERWISE;
*TIME IS IN ns UNLESS MENTIONED OTHERWISE;
*WIDTH IS IN um UNLESS MENTIONED OTHERWISE;
*CURRENT IS IN uA UNLESS MENTIONED OTHERWISE;
* THE As are statistical constants for the variance of the I with size


.GLBPARAM
CovN = 0.465;
CovP = 0.395;
CparN_r_edge =  0.025;
CparP_r_edge = 0.019;
CparN_r_width = 0.49;
CparP_r_width = 0.38;
CparN_f_edge = 0.02;
CparP_f_edge = 0.023;
CparN_f_width= 0.42;
CparP_f_width = 0.42;
idr_rise = 230;
idr_fall = 490;
*the Cgates include the overlap capacitances on both sides.
CgateP_rise = 1;
CgateP_fall = 1.1;
CgateN_rise = 1.2;
CgateN_fall = 1.1;
*as can be clearly seen, the gate caps are mostly overlap caps.
* the Vths below are used for slope effect on delay (1+ 0.5*Vth/Vdd) factor.
Vthn = 0.35;
Vthp = 0.4;
Vdd = 1;
Vdd_energy = Vdd;
CenergyN_edge = 0.023;
CenergyP_edge = 0.021;
CenergyN_width = 0.46;
CenergyP_width = 0.41;
CgateN_energy = 1.4;
Cgate_energy_n = 1.4;
CgateP_energy = 1.25;
Cgate_energy_p = 1.25;
min_act_factor = 0.001;
def_act_factor = 0.25;
def_duty_factor = 0.5;
min_slope_effect = 0.001;
min_res = 0.001;
.ENDS
