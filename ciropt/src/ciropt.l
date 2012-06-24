%{
#include <string>
#include <vector>
#include "opt.hpp"
#include "../obj/ciropt.tab.hpp"

unsigned ciropt_line_number = 1;
extern std::string ciropt_filename;
static void HandleToken();
static void HandleNumber();
%}

%option noyywrap
%s N
%x GLB_PARAMS SUBCKT_SECTION CONNECT_SECTION PI_SECTION PO_SECTION GLB_CNSTR POWER_SECTION DUTY_SECTION TRANSMISSION_SECTION  OPTIMIZE_SECTION

number		[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
identifier	_?[A-Za-z][A-Za-z0-9_\.\[\]]*



/* sections to make rules for 
 * so that symbols specific to one remain in one.*/


  /*SINGLE_COMMENT    (";".*|"*"[^\n]*) */
SINGLE_COMMENT    ("^*"[^\n]*)


BEG_GLBPARAMS     (".glbparam"|".GLBPARAM")
END_GLBPARAMS     (".ends"|".ENDS")

BEG_SUBCKT        (".subckt"|".SUBCKT")
END_SUBCKT        (".ends"|".ENDS")

BEG_CONNECT       (".connect"|".CONNECT")
END_CONNECT       (".ends"|".ENDS")

BEG_PI            (".pi"|".PI")
END_PI            (".ends"|".ENDS")

BEG_PO            (".po"|".PO")
END_PO            (".ends"|".ENDS")

BEG_GLBCNSTR      (".glbcnstr"|".GLBCNSTR")
END_GLBCNSTR      (".ends"|".ENDS")

BEG_POWER         (".power"|".POWER")
END_POWER         (".ends"|".ENDS")

BEG_DUTY            (".duty"|".DUTY")
END_DUTY            (".ends"|".ENDS")

BEG_TRANSMISSION    (".transmission"|".TRANSMISSION")
END_TRANSMISSION    (".ends"|".ENDS")

BEG_OPTIMIZE    (".optimize"|".OPTIMIZE")
END_OPTIMIZE    (".ends"|".ENDS")

%%



"max"		{ return MAX; }
 /*"rf"		{ return RF; }
 "fr"		{ return FR; }
 "rr"		{ return RR; }
 "ff"		{ return FF; }
 */

 /*
{number}	{ ciroptlval.doubleV = atof(yytext); 
		        //cout << yytext << endl;
		        return PNUMBER; }
 
{identifier}	{
		  ciroptlval.stringV = new std::string(yytext);
			//cout << yytext << " " ;
		  return IDENTIFIER;
		}
		*/

"<"		{ return LESSTHAN; }
"="		{ return EQUALTO; }
":"		{ return COLON; }
";"		{ return DELIMITER; }
"+"		{ return PLUS; }
"-"		{ return MINUS; }
"*"		{ return TIMES; }
"/"		{ return DIVIDE; }
"^"		{ return POWER; }
"("		{ return POPEN; }
")"		{ return PCLOSE; }
","		{ return COMMA; }
"'"   { return SINGLE_I;}



.	{ return ILLEGAL_TOKEN;  }


 /* -------------------- Universal rules ----------------------------- */
<*>^[ \t]*"*".*\n	{ ciropt_line_number++; }
<*>"<"		{ return LESSTHAN; }
<*>"="		{ return EQUALTO; }
<*>":"		{ return COLON; }
<*>";"		{ return DELIMITER; }
<*>"+"		{ return PLUS; }
<*>"-"		{ return MINUS; }
<*>"*"		{ return TIMES; }
<*>"/"		{ return DIVIDE; }
<*>"^"		{ return POWER; }
<*>"("		{ return POPEN; }
<*>")"		{ return PCLOSE; }
<*>","		{ return COMMA; }
<*>"'"    { return SINGLE_I;}
<*>[\n]		{ ciropt_line_number ++; }
<*>[ \t\r]+        { }
<*>{number}		    { HandleNumber();return PNUMBER; }



 /* Rules for the GLB_PARAMS definitions */
{BEG_GLBPARAMS}             { BEGIN( GLB_PARAMS ); return GLBPARAM;}
<GLB_PARAMS>{END_GLBPARAMS}  { BEGIN( N ); return ENDS;}
<GLB_PARAMS>{identifier}		{ HandleToken();return IDENTIFIER; }


 /* Rules for the SUBCKTs definitions */
{BEG_SUBCKT}             { BEGIN( SUBCKT_SECTION ); return SUBCKT;}
<SUBCKT_SECTION>{END_SUBCKT}  { BEGIN( N ); return ENDS;}
<SUBCKT_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }



 /* Rules for the CONNECT definitions */
{BEG_CONNECT}             { BEGIN( CONNECT_SECTION ); return CONNECT;}
<CONNECT_SECTION>{END_CONNECT}  { BEGIN( N ); return ENDS;}
<CONNECT_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }

  /* Rules to be applied in the .PI section only */
{BEG_PI}             { BEGIN( PI_SECTION); return PI;}
<PI_SECTION>{END_PI}  { BEGIN( N ); return ENDS;}
<PI_SECTION>"at"		{ return AT; }
<PI_SECTION>"af"		{ return AF; }
<PI_SECTION>"s"		{ return SLOPE; }
<PI_SECTION>"name"		{ return NAME; }
<PI_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }


  /* Rules to be applied in the .PO section only */
{BEG_PO}             { BEGIN( PO_SECTION); return PO;}
<PO_SECTION>{END_PO}  { BEGIN( N ); return ENDS;}
<PO_SECTION>"name"		{ return NAME; }
<PO_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }

 /* Rules for the .GLBCNSTR Section on;y...it contains the most symbols. */
{BEG_GLBCNSTR}             { BEGIN( GLB_CNSTR );return GLBCNSTR; }
<GLB_CNSTR>{END_GLBCNSTR}  { BEGIN( N ); return ENDS;}
<GLB_CNSTR>"A"		{ return AREA; }
<GLB_CNSTR>"E"		{ return ENGY; }
<GLB_CNSTR>"P"		{ return PWR; }
<GLB_CNSTR>"D"		{ return DELAY; }
<GLB_CNSTR>"MIN_WIDTH"	{ return MIN_WIDTH; }
<GLB_CNSTR>"MAX_WIDTH"	{ return MAX_WIDTH; }
<GLB_CNSTR>"MIN_VTH_P"	{ return MIN_VTH_P; }
<GLB_CNSTR>"MAX_VTH_P"	{ return MAX_VTH_P; }
<GLB_CNSTR>"MIN_VTH_N"	{ return MIN_VTH_N; }
<GLB_CNSTR>"MAX_VTH_N"	{ return MAX_VTH_N; }
<GLB_CNSTR>"MAX_VDD"	{ return MAX_VDD; }
<GLB_CNSTR>"MIN_VDD"	{ /*printf("got here\n");*/ 
		                     return MIN_VDD; }
<GLB_CNSTR>"LogicDepthFactor"	{ return LDF; }
<GLB_CNSTR>"OnlyFormulateProblem"	{ return OFP; }
<GLB_CNSTR>"DoNotUniquify"	{ return DNU; }
<GLB_CNSTR>"DoNotIncludeLeakage"	{ return DNIL; }
<GLB_CNSTR>"NoRiseFallTiming"	{ return NRF; }
<GLB_CNSTR>"NoAllEdgeTimeConstraint"	{ return NAETC; }
<GLB_CNSTR>"GlobalVar"	{ return GLBVAR; }
<GLB_CNSTR>"SetUniformKappas"	{ return SUK; }
<GLB_CNSTR>"SetCriticalityKappas"	{ return SCK; }
<GLB_CNSTR>"UseDefActFact"	{ return UDAF; }
<GLB_CNSTR>"AntiCorr"	{ return ANTICORR; }
<GLB_CNSTR>"IntPrecharge"	{ return INTPRECHARGE; }
<GLB_CNSTR>"t_rise"	{ return TRISE; }
<GLB_CNSTR>"t_fall"	{ return TFALL; }
<GLB_CNSTR>"RF"	{ return RF; }
<GLB_CNSTR>"FR"	{ return FR; }
<GLB_CNSTR>"rf"	{ return RF; }
<GLB_CNSTR>"fr"	{ return FR; }
<GLB_CNSTR>{identifier}		{ HandleToken();return IDENTIFIER; }


  /* Rules to be applied in the .POWER section only */
{BEG_POWER}             { BEGIN( POWER_SECTION); return POWER_SEC;}
<POWER_SECTION>{END_POWER}  { BEGIN( N ); return ENDS;}
<POWER_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }

  /* Rules to be applied in the .DUTY section only */
{BEG_DUTY}             { BEGIN( DUTY_SECTION); return DUTY_SEC;}
<DUTY_SECTION>{END_DUTY}  { BEGIN( N ); return ENDS;}
<DUTY_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }


  /* Rules to be applied in the .TRANSMISSION section only */
{BEG_TRANSMISSION}             { BEGIN( TRANSMISSION_SECTION); return TRAN_GATE_SEC;}
<TRANSMISSION_SECTION>{END_TRANSMISSION}  { BEGIN( N ); return ENDS;}
<TRANSMISSION_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }


 /* Rules to be applied in the .OPTIMIZE section only */
{BEG_OPTIMIZE}             { BEGIN( OPTIMIZE_SECTION); return OPTIMIZE;}
<OPTIMIZE_SECTION>{END_OPTIMIZE}  { BEGIN( N ); return ENDS;}
<OPTIMIZE_SECTION>"A"		{ return AREA; }
<OPTIMIZE_SECTION>"E"		{ return ENGY; }
<OPTIMIZE_SECTION>"P"		{ return PWR; }
<OPTIMIZE_SECTION>"D"		{ return DELAY; }
<OPTIMIZE_SECTION>"draw"		{ return DRAW; }
<OPTIMIZE_SECTION>"write"		{ return WRITE; }
<OPTIMIZE_SECTION>"analysis"	{ return ANALYSIS; }
<OPTIMIZE_SECTION>"criticality_distribution"	{ return CRCALDIST; }
<OPTIMIZE_SECTION>"widths_distribution"		{ return WIDTHDIST; }
<OPTIMIZE_SECTION>"path_lengths"	{ return PATHLNGS; }
<OPTIMIZE_SECTION>"path_variances"	{ return PATHVARS; }
<OPTIMIZE_SECTION>"mwrite"	{ return MWRITE; }
<OPTIMIZE_SECTION>"criticalities"	{ return CRCALS; }
<OPTIMIZE_SECTION>"widths"	{ return WIDTHS; }
<OPTIMIZE_SECTION>"gate_dios"		{ return GATEDIOS; }
<OPTIMIZE_SECTION>"monte"		{ return MONTE; }
<OPTIMIZE_SECTION>"opt"		{ return OPT; }
<OPTIMIZE_SECTION>"pdf"		{ return PDF; }
<OPTIMIZE_SECTION>"cdf"		{ return CDF; }
<OPTIMIZE_SECTION>"iter"		{ return ITER; }
<OPTIMIZE_SECTION>"m_var_name"	{ return MVARNAME; }
<OPTIMIZE_SECTION>"minimize"	{ return MINIMIZE; }
<OPTIMIZE_SECTION>"kappa"		{ return KAPPA; }
<OPTIMIZE_SECTION>"kappas"	{ return KAPPAS; }
<OPTIMIZE_SECTION>"beta"		{ return BETA; }
<OPTIMIZE_SECTION>"values"	{ return VALUES; }
<OPTIMIZE_SECTION>{identifier}		{ HandleToken();return IDENTIFIER; }
 









 /* -------------------- Default rule (error) -------------------- */


%%


static void HandleToken()
{
   //string buffer(yytext);
	 ciroptlval.stringV = new std::string(yytext);

   // printf("handling token: %s\n", yytext);

}

static void HandleNumber()
{
   //string buffer(yytext);
	 ciroptlval.doubleV = atof(yytext); 

   // printf("handling token: %s\n", yytext);

}
