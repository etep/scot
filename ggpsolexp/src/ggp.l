%{
#include <vector>
#include "gp.hpp"
#include "ggp.hpp"
#include "ggp.tab.hpp"

int line_number = 1;

%}

%option noyywrap

number		[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
variable	_?[A-Za-z][A-Za-z0-9_\.\[\]]*
minimize	minimize
max		max
inv		inv
sqrt		sqrt

%x CMNT
%x LCMNT
%%
{number}	{ ggplval.doubleV = atof(yytext); 
//		 				cout << yytext;
		 				return PNUMBER; }
{minimize}	{ return MINIMIZE; }
{max}		{ return MAX; }
{inv}		{ return INV; }
{sqrt}		{ return SQRT; }
{variable}	{ ggplval.stringV = strdup(yytext); 
//		 					cout << yytext << " ";
		 					return VARIABLE; }

"+"		{ return PLUS; } 
"-"		{ return MINUS; }
"*"		{ return TIMES; }
"/"		{ return DIVIDE; }
"^"		{ return POWER; }
"("		{ return POPEN; }
")"		{ return PCLOSE; }
","		{ return COMMA; }
":"		{ return COLON; }
";"		{ return DELIMITER; }

"="		{ return EQUALTO; }
"<"		{ return LESSTHAN; }
">"		{ return GREATERTHAN; }

\n		{ line_number ++; }
[ \t]+		;

"/*"		BEGIN CMNT;
<CMNT>.		;
<CMNT>\n	{ line_number ++; }
<CMNT>"*/"	BEGIN INITIAL;

"%"		BEGIN LCMNT;
<LCMNT>.	;
<LCMNT>\n	{ line_number ++; BEGIN INITIAL; }

.		{ return ILLEGAL_TOKEN;  }
%%
