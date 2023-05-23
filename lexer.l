%{
  #include "parser.tab.h"
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

"while"       return WHILE;
"for"         return FOR;
"repeat"      return REPEAT;
"until"       return UNTIL;

"print"       return PRINT;
"return"      return RETURN;

"::"  return DOUBLE_COLON;

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
  yylval.STRING = strdup(yytext + 1);
  yylval.STRING[yyleng - 2] = 0;
  return STRING;
}

true {
  yylval.BOOL = true;
  return BOOL;
}

false {
  yylval.BOOL = false;
  return BOOL;
}

[0-9]+ {
  yylval.INTEGER = atoi(yytext);
  return INTEGER;
}

[0-9]*\.[0-9]+ {
  yylval.FLOAT = atof(yytext);
  return FLOAT;
}

[_a-zA-Z][_a-zA-Z0-9]* {
  yylval.IDENTIFIER = strdup(yytext);
  return IDENTIFIER;
}

[\.,;:=(){}[\]] return *yytext;

. return ERROR;

%%
