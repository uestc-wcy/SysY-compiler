%{
#include"SysY.tab.h"

int line=1;
%} 

decimalConst ([1-9][0-9]*)|[0]
octalConst ([0][0-7]*)|[0]
hexadecimalDigit ("0x"|"0X")([0-9a-fA-F]*)

digit [0-9]
letter [A-Za-z_]
Ident {letter}({letter}|{digit})*
IntConst {decimalConst}|{octalConst}|{hexadecimalDigit}

  
%%
\n {line++;}
[ \t] {}

"/*"(([^\*]*(\*[^\/])?)*)"*/" {}
\/\/.* {}

int       {return INT;}
void      {return VOID;}
if        {return IF;}
else      {return ELSE;}
while     {return WHILE;}
break	  {return BREAK;}
continue  {return CONTINUE;}
const     {return CONST;}
return    {return RETURN;}


"+" |
"-" |
"*" |
"/" |
"%" |
"," |
";" |
"(" |
")" |
"=" |
"{" |
"}" |
"[" |
"!" |
"]" {return yytext[0];}


"<"     {yylval.sValue=strdup("<");    return CMP;}
">"     {yylval.sValue=strdup(">");    return CMP;}
"<="    {yylval.sValue=strdup("<=");   return CMP;}
">="    {yylval.sValue=strdup(">=");   return CMP;}

"=="    {yylval.sValue=strdup("==");   return JUDGE;}
"!="    {yylval.sValue=strdup("!=");   return JUDGE;}

"&&"	{yylval.sValue=strdup("&&");   return LOGIC_AND;}
"||"	{yylval.sValue=strdup("||");   return LOGIC_OR;} 

\"([^\"]*)\" {
	char*tem=strdup(yytext+1);
	tem[strlen(tem)-1]=0;
	yylval.sValue=strdup(tem);
	free(tem);
	return StringConst;
}

{IntConst} {yylval.iValue=atoi(yytext);  return IntConst;}
{Ident}     {yylval.sValue=strdup(yytext);return Ident;}

<<EOF>> {return EOP;}

%%

int yywrap()
{
	return 1;
} 
