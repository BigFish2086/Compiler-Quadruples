%{
  #include "y.tab.h"
  int line_num = 1;
%}
%option yylineno
%option noyywrap

%%

"int"     return INT_TYPE;
"flt"     return FLOAT_TYPE;
"bool"    return BOOL_TYPE;
"str"     return STRING_TYPE;
"enum"    return ENUM_TYPE;
"const"   return CONST_TYPE;

"if"          return IF;
"else"        return ELSE;

"switch"      return SWITCH;
"case"        return CASE;
"default"     return DEFAULT;
"break"       return BREAK;

"while"       return WHILE;
"for"         return FOR;

"print"       return PRINT;
"return"      return RETURN;

"+"     return PLUS;
"-"     return MINUS;
"*"     return MULT;
"/"     return DIV;
"!"     return NOT;
"&"     return AND;
"|"     return OR;
"=="    return EQ;
"!="    return NE;
">"     return GT;
"<"     return LT;
">="    return GE;
"<="    return LE;

" " ;
\r ;
\n line_num++;
[ \t]+ ;
\/\/[^\n]* ;

\"[^"\n]*\" {
  yylval.sval = strdup(yytext + 1);
  yylval.sval[yyleng - 2] = 0;
  return STRING;
}

true {
  yylval.bval = true;
  return BOOL;
}

false {
  yylval.bval = false;
  return BOOL;
}

[0-9]+ {
  yylval.ival = atoi(yytext);
  return INTEGER;
}

[0-9]*\.[0-9]+ {
  yylval.fval = atof(yytext);
  return FLOAT;
}

[_a-zA-Z][_a-zA-Z0-9]* {
  yylval.id = strdup(yytext);
  return IDENTIFIER;
}

[\.,;:=(){}[\]] return *yytext;

. return ERROR;

%%
