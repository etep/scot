* Note:  The A's are statistical constants for the variance of I with size.
* Unless mentioned otherwise:
* Capacitance: [fF/um]
*        Time: [ns]
*       Width: [um]
*     Current: [uA]
.GLBPARAM
CovN = 0.0832;
CovP = 0.1872;
CparN_r_edge  = 1.7250;
CparP_r_edge  = 0.8625;
CparN_r_width = 0.0031;
CparP_r_width = 0.0031;
CparN_f_edge  = 1.8328;
CparP_f_edge  = 0.8625;
CparN_f_width = 0.0066;
CparP_f_width = 0.0066;
idr_rise = 1000;
idr_fall = 1000;
*the Cgates include the overlap capacitances on both sides.
CgateP_rise = 0.7480;
CgateP_fall = 1.8919;
CgateN_rise = 0.4451;
CgateN_fall = 1.8919;
*as can be clearly seen, the gate caps are mostly overlap caps.
* the Vths below are used for slope effect on delay (1+ 0.5*Vth/Vdd) factor.
Vthn = 0.3900;
Vthp = 0.4000;
Vdd = 1.0;
Vdd_energy = Vdd;
CenergyN_edge    = 0.8934;
CenergyP_edge    = 2.0101;
CenergyN_width   = 0.0925;
CenergyP_width   = 0.2082;
CgateN_energy    = 0.0001;
Cgate_energy_n   = 0.0001;
CgateP_energy    = 0.0001;
Cgate_energy_p   = 0.0001;
min_act_factor   = 0.0010;
def_act_factor   = 0.2500;
def_duty_factor  = 0.5000;
min_slope_effect = 0.0010;
min_res = 0.0010;
.ENDS
