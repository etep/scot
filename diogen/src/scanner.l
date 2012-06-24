/* File:  scanner.l
 * ----------------
 * Lex inupt file to generate the scanner for the compiler.
 */

%{

#include <string>
#include "../src/scanner.h"
#include "../src/utility.hpp" // for PrintDebug()

#include "../src/parser.h" // for token codes, yylval

//#include "location.h"

using namespace std;
int lineno = 0;

#define TAB_SIZE 8

/* Global variables
 * ----------------
 * (For shame!) But we need a few to keep track of things that are
 * preserved between calls to yylex or used outside the scanner.
 */
static int curLineNum, curColNum;

static void DoBeforeEachAction(); 
static void HandleToken();
//#define YY_USER_ACTION DoBeforeEachAction();

%}

%s N
%x BASE_DIOS GLB_PARAMS SUB_CIRCUIT CONNECTIVITY PI PO GLBCNSTR IN_ANTICORR IN_INTPRECHARGE POWER DUTY TRANSMISSION IRSIM


DIGIT             ([0-9])
INTEGER           ({DIGIT}+)
TIME              ({INTEGER}"."{INTEGER}"ns")
DOUBLE            [0-9]*\.[0-9]+([eE][-+]?[0-9]+)?
DOUBLE_DEC        [0-9]+[eE][-+]?[0-9]+
DOUBLE_PAIR       ("("{DOUBLE}","{DOUBLE}")")  
 /* found in .PI section */

IDENTIFIER        ([a-zA-Z_][[\]a-zA-Z_0-9\.]*)



OPERATOR          ([-+/*%=.,;!<>()[\]{}])

  /*SINGLE_COMMENT    (";".*|"*"[^\n]*) */
SINGLE_COMMENT    ("*"[^\n]*)

BEG_BASE_DIOS     (".base_dios"|".BASE_DIOS")
END_BASE_DIOS     (".ends"|".ENDS")

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

BEG_GLBCNSTR            (".glbcnstr"|".GLBCNSTR")
END_GLBCNSTR            (".ends"|".ENDS")

BEG_POWER            (".power"|".POWER")
END_POWER            (".ends"|".ENDS")

BEG_DUTY            (".duty"|".DUTY")
END_DUTY            (".ends"|".ENDS")

BEG_TRANSMISSION            (".transmission"|".TRANSMISSION")
END_TRANSMISSION            (".ends"|".ENDS")

BEG_IRSIM         "|"
 /* NO END_IRSIM yet.. */



 /* token types inside BASE_DIOS section */
 //EXPR		  ([0-9a-zA-Z+-/*.()^_]+)
 //EXPR		  ([0-9a-zA-Z\(\)_][0-9a-zA-Z_+-/\.*\(\)^]*[^,^+^-^/^*])
MATHEXPR		  ([0-9a-zA-Z\(\)_][0-9a-zA-Z_+-/\.*\(\)^]*[0-9a-zA-Z\(\)])

 /* token types inside SUB_CIRCUIT section */
MOS_TYPE          ("nmos"|"NMOS"|"pmos"|"PMOS"|"pch"|"PCH"|"nch"|"NCH")

 /* token types inside GLBCNSTR section */
ANTICORR          ("AntiCorr")
INTPRECHARGE          ("IntPrecharge")

 /* token types inside CONNECT section */
INSTANCE_NAME     ("[Xx]"[a-zA-Z][a-zA-Z_0-9]*)
CAP_NAME          ("[Cc]"{IDENTIFIER})

%%             /* BEGIN RULES SECTION */

[ \t\r]+    { }
<*>[\t]                { curColNum += TAB_SIZE - curColNum%TAB_SIZE + 1; }
<*>[\n]           {lineno++;}
 /* -------------------- Comments ----------------------------- */
<*>{SINGLE_COMMENT}    {}


 /* Rules for base dio sections */
{BEG_BASE_DIOS}             { lineno = 1;BEGIN( BASE_DIOS ); }
<BASE_DIOS>{END_BASE_DIOS}  { BEGIN( N ); }

<BASE_DIOS>[ \n\t\r]+    { }
<BASE_DIOS>{IDENTIFIER}	    { HandleToken(); 
                              //printf("TOKEN1: %s\n", yytext);
															return T_CONST_NAME; }
<BASE_DIOS>{MATHEXPR}           { HandleToken(); 
		                          //printf("TOKEN2: %s\n", yytext);
															return T_CONST_EXPR; }
<BASE_DIOS>{INTEGER}           { HandleToken();
		                            //printf("TOKEN3: %s\n", yytext);
															return T_CONST_EXPR; } 
<BASE_DIOS>"="              { return T_EQUAL; }
<BASE_DIOS>";"              { return T_SEMICOLON; } 


 /* Rules for global paramters section (all ignore) */
{BEG_GLBPARAMS}              { lineno = 8 ;BEGIN( GLB_PARAMS ); } 
        /*the global params start at line 8 */ 
<GLB_PARAMS>{END_GLBPARAMS} { BEGIN( N ); }
<GLB_PARAMS>[ \t\r]+    { }
<GLB_PARAMS>.             { }


 /* Rules for subcircuit section */
{BEG_SUBCKT}                { BEGIN( SUB_CIRCUIT ); return T_SUB_CIRCUIT; }
<SUB_CIRCUIT>{END_SUBCKT}    { BEGIN( N ); }

<SUB_CIRCUIT>[ \t\r]+    { }
<SUB_CIRCUIT>{INTEGER}         { HandleToken(); return T_IDENTIFIER; }
<SUB_CIRCUIT>{DOUBLE}         { HandleToken(); return T_IDENTIFIER; }
<SUB_CIRCUIT>{DOUBLE_DEC}         { HandleToken(); return T_IDENTIFIER; }
<SUB_CIRCUIT>{MOS_TYPE}        { HandleToken(); return T_MOS_TYPE; }
<SUB_CIRCUIT>^{IDENTIFIER}     { HandleToken(); return T_DEVICE_NAME; }
<SUB_CIRCUIT>{IDENTIFIER}      { HandleToken(); return T_IDENTIFIER; }
<SUB_CIRCUIT>{MATHEXPR}            { HandleToken(); return T_IDENTIFIER; }
<SUB_CIRCUIT>":"               { return T_COLON; }


<SUB_CIRCUIT>"="               { return T_EQUAL; }
<SUB_CIRCUIT>";"               { return T_SEMICOLON; } 


 /* Rules for connectivity section */
{BEG_CONNECT}             { BEGIN( CONNECTIVITY ); }
<CONNECTIVITY>{END_CONNECT}    { BEGIN( N ); }

 /* <CONNECTIVITY>{INSTANCE_NAME}   { HandleToken(); return T_INSTANCE_NAME; } */
 /* <CONNECTIVITY>{CAP_NAME}        { HandleToken(); return T_CAP_NAME; } */
<CONNECTIVITY>{IDENTIFIER}      { HandleToken(); return T_IDENTIFIER; }
<CONNECTIVITY>{DOUBLE}          { HandleToken(); return T_IDENTIFIER; }
<CONNECTIVITY>{DOUBLE_DEC}          { HandleToken(); return T_IDENTIFIER; }
<CONNECTIVITY>{INTEGER}         { HandleToken(); return T_IDENTIFIER; }
<CONNECTIVITY>{MATHEXPR}         { HandleToken(); return T_IDENTIFIER; }


<CONNECTIVITY>";"              { return T_SEMICOLON; }
<CONNECTIVITY>":"              { return T_COLON; }
<CONNECTIVITY>"="               { return T_EQUAL; }
<CONNECTIVITY>.            { /* ignore */ }
<CONNECTIVITY>[ \t\r]+    { }


 /* Rules for PI section */
{BEG_PI}                      { BEGIN(PI); }
<PI>{END_PI}                  { BEGIN(N); }
 /*<PI>","                       { HandleToken(); return T_COMMA; }*/
<PI>","                       { /*ignore*/ }
<PI>{IDENTIFIER}              { HandleToken(); 
		 													  //printf("TOKEN1: %s\n", yytext); 
																return T_IDENTIFIER; }
<PI>{DOUBLE}                  { HandleToken(); return T_IDENTIFIER; }
<PI>{DOUBLE_DEC}              { HandleToken(); return T_IDENTIFIER; }
<PI>{DOUBLE_PAIR}             { HandleToken(); return T_DOUBLE_PAIR; }
<PI>{INTEGER}                 { HandleToken(); return T_IDENTIFIER; }
<PI>";"                       { return T_SEMICOLON; }
<PI>"="                       { return T_EQUAL; }
<PI>.                         { /* ignore */ }
<PI>{MATHEXPR}                    { HandleToken(); 
		                            //printf("TOKEN2: %s\n", yytext); 
		                            return T_IDENTIFIER; }
<PI>[ \t\r]+    { }

 /* Rules for PO section */
{BEG_PO}                      { BEGIN(PO); }
<PO>{END_PO}                  { BEGIN(N); }
<PO>.                         { /* ignore */ }
<PO>[ \t\r]+    { }
 /*<PO>","                       { HandleToken(); return T_COMMA; }
 <PO>","                       {  }
 <PO>{IDENTIFIER}              { HandleToken(); 
		 													  //printf("TOKEN1: %s\n", yytext); 
																return T_IDENTIFIER; }
<PO>{DOUBLE}                  { HandleToken(); return T_IDENTIFIER; }
<PO>{DOUBLE_DEC}              { HandleToken(); return T_IDENTIFIER; }
<PO>{DOUBLE_PAIR}             { HandleToken(); return T_DOUBLE_PAIR; }
<PO>{INTEGER}                 { HandleToken(); return T_IDENTIFIER; }
<PO>";"                       { return T_SEMICOLON; }
<PO>"="                       { return T_EQUAL; }
<PO>{MATHEXPR}                    { HandleToken(); 
		                            //printf("TOKEN2: %s\n", yytext); 
		                            return T_IDENTIFIER; }
<PO>[ \t\r]+    { }
*/

 /* Rules for GLBCNSTR section */
{BEG_GLBCNSTR}                      { BEGIN(GLBCNSTR); }
<GLBCNSTR>{END_GLBCNSTR}                  { BEGIN(N); }
<GLBCNSTR>{ANTICORR}                  { BEGIN(IN_ANTICORR);HandleToken(); return ANTICORR; }
<IN_ANTICORR>";"                      {BEGIN(GLBCNSTR); return T_SEMICOLON;}
<IN_ANTICORR>{IDENTIFIER}             { HandleToken(); 
		                                  //printf("TOKEN1: %s\n", yytext); 
																      return T_IDENTIFIER; }
<IN_ANTICORR>.                        {}  													
<GLBCNSTR>{INTPRECHARGE}                  { BEGIN(IN_INTPRECHARGE);HandleToken(); return INTPRECHARGE; }
<IN_INTPRECHARGE>";"                      {BEGIN(GLBCNSTR); return T_SEMICOLON;}
<IN_INTPRECHARGE>{IDENTIFIER}             { HandleToken(); 
		                                  //printf("TOKEN1: %s\n", yytext); 
																      return T_IDENTIFIER; }
<IN_INTPRECHARGE>.                        {}
  /*<GLBCNSTR>{IDENTIFIER}              { HandleToken(); 
		                                  printf("TOKEN1: %s\n", yytext); 
																      return T_IDENTIFIER; }*/
<GLBCNSTR>.                         { /* ignore */ }
<GLBCNSTR>[ \t\r]+    { }


 /* Rules for GLBCNSTR section */
{BEG_POWER}                      { BEGIN(POWER); }
<POWER>{END_POWER}                  { BEGIN(N); }
<POWER>.                         { /* ignore */ }
<POWER>[ \t\r]+    { }

 /* Rules for DUTY section */
{BEG_DUTY}                      { BEGIN(DUTY); }
<DUTY>{END_DUTY}                  { BEGIN(N); }
<DUTY>.                         { /* ignore */ }
<DUTY>[ \t\r]+    { }

 /* Rules for TRANSMISSION section */
{BEG_TRANSMISSION}                      { BEGIN(TRANSMISSION); }
<TRANSMISSION>{END_TRANSMISSION}                  { BEGIN(N); }
<TRANSMISSION>.                         { /* ignore */ }
<TRANSMISSION>[ \t\r]+    { }

 /* Rules for IRSIM section */
{BEG_IRSIM}              { BEGIN(IRSIM); return T_VERTICAL_BAR; }
<IRSIM>"|"           { return T_VERTICAL_BAR; }
<IRSIM>"time"        { return T_TIME; }
<IRSIM>"="           { return T_EQUAL; }
<IRSIM>{TIME}        { return T_TIME_EXPR; }
<IRSIM>{INTEGER}     { HandleToken(); return T_IDENTIFIER; }
<IRSIM>{DOUBLE}     { HandleToken(); return T_IDENTIFIER; }
<IRSIM>{DOUBLE_DEC}     { HandleToken(); return T_IDENTIFIER; }
<IRSIM>{IDENTIFIER}  { HandleToken(); return T_IDENTIFIER; }
<IRSIM>[ \n\r\t]+     { }














 /* -------------------- Default rule (error) -------------------- */
.                   { /*ReportError::UnrecogChar(&yylloc, yytext[0]);*/ }



%%


/* Function: InitScanner
 * ---------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */
void InitScanner()
{
 //   PrintDebug("lex",  "Initializing scanner");
    yy_flex_debug = false;
    BEGIN(N);
    //yy_push_state(COPY); // copy first line at start
    curLineNum = 1;
    curColNum = 1;
}


/* Function: DoBeforeEachAction()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 * On each match, we fill in the fields to record its location and
 * update our column counter.
 */

/*
static void DoBeforeEachAction()
{
   yylloc.first_line = curLineNum;
   yylloc.first_column = curColNum;
   yylloc.last_column = curColNum + yyleng - 1;
   curColNum += yyleng;
}
*/

static void HandleToken()
{
   //string buffer(yytext);
   yylval.identifier = new string(yytext);

    // printf("handling token: %s\n", yytext);
/*    if (strlen(yytext) > MaxIdentLen)
       printf("string too long\n");
    strncpy(yylval.identifier, yytext, MaxIdentLen);
       yylval.identifier[MaxIdentLen] = '\0';
*/

}
