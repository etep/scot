#ifndef _H_ModelPrefix
#define _H_ModelPrefix

#include <string>

using namespace std;

static string CovN = "CovN";
static string CovP = "CovP";
static string CparN_r_edge = "CparN_r_edge";
static string CparP_r_edge = "CparP_r_edge";
static string CparN_r_width = "CparN_r_width";
static string CparP_r_width = "CparP_r_width";
static string CparN_f_edge = "CparN_f_edge";
static string CparP_f_edge = "CparP_f_edge";
static string CparN_f_width = "CparN_f_width";
static string CparP_f_width = "CparP_f_width";

static string CenergyN_edge = "CenergyN_edge";
static string CenergyP_edge = "CenergyP_edge";
static string CenergyN_width = "CenergyN_width";
static string CenergyP_width = "CenergyP_width";

static string idr_rise = "idr_rise";
static string idr_fall = "idr_fall";

static string CgateP_rise = "CgateP_rise";
static string CgateP_fall = "CgateP_fall";
static string CgateN_rise = "CgateN_rise";
static string CgateN_fall = "CgateN_fall";

static string Cgate_energy_n = "Cgate_energy_n";
static string Cgate_energy_p = "Cgate_energy_p";

static string Vthn = "Vthn";
static string Vthp = "Vthp";
static string VoltageSource = "Vdd"; // clash with class Vdd


static string fallToRiseStd = "fr_std_";
static string fallToRiseNor = "fr_mean_";

static string riseToFallStd = "rf_std_";
static string riseToFallNor = "rf_mean_";

static string transistorPrefix = "trans_";

static string width = "W";
static string intCapVal = "c";
static string multiplier = "M";
static string effectiveWidth = "Weff"; // Weff = M * width
static string txGateWidthPrefix = "WT";

static string minActFactor = "min_act_factor";
static string defActFactor = "def_act_factor";
static string defDutyFactor = "def_duty_factor";
static string modelNotFound = "NOT_FOUND";

static string pLeakNom = "p_leak_nom";
static string pLeakStat = "p_leak_stat";
static string nLeakNom = "n_leak_nom";
static string nLeakStat = "n_leak_stat";

#endif
