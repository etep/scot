%{
#include "gp.hpp"
#include "../obj/dio.tab.hpp"
unsigned dio_line_number = 1;
extern std::string dio_filename;
%}

%option noyywrap

number		[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
identifier	_?[A-Za-z][A-Za-z0-9_\.\[\]]*

%%
^[ \t]*"*".*\n	{ dio_line_number ++; /* COMMENT */}

"max"		{ return MAX; }
"rf"		{ return RF; }
"fr"		{ return FR; }
"rr"		{ return RR; }
"ff"		{ return FF; }
"std"		{ return STD; }

{number}	{ diolval.doubleV = atof(yytext); return PNUMBER; }
{identifier}	{ diolval.charpV = strdup(yytext); return IDENTIFIER; }

":"	{ return COLON; }
";"	{ return DELIMITER; }
"+"	{ return PLUS; }
"-"	{ return MINUS; }
"*"	{ return TIMES; }
"/"	{ return DIVIDE; }
"^"	{ return POWER; }
"("	{ return POPEN; }
")"	{ return PCLOSE; }
","	{ return COMMA; }

\n	{ dio_line_number ++; }
[ \t]+	;

.	{ return ILLEGAL_TOKEN;  }
%%
