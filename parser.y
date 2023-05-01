%{
  #include <iostream>
  #include <string>
  using namespace std;

  extern int yylex();
  void yyerror(string e) {}

  extern FILE* yyin;
%}

%union {
  int ival;
  float fval;
  bool bval;
  char* sval;

  char* id;
}
%token <ival> INTEGER
%token <fval> FLOAT
%token <bval> BOOL
%token <sval> STRING
%token <id> IDENTIFIER

%type <yytokentype> type

// for function declaration and invokation
// TODO: may add types to these tokens
%type expr expr_in_parenthsis function_invokation
%type typed_parameter_list argument_list
%type parameter_list

%token ERROR PRINT RETURN 
%token WHILE FOR
%token IF ELSE SWITCH CASE DEFAULT BREAK CONTINUE
%token INT_TYPE FLOAT_TYPE BOOL_TYPE STRING_TYPE ENUM_TYPE CONST_TYPE

%token NOT AND OR
%token PLUS MINUS MULT DIV
%token EQ NE LT GT LE GE

%left AND OR
%left EQ NE LT GT LE GE
%left PLUS MINUS
%left MULT DIV
%right NOT

%nonassoc UMINUS

%start program

%%

type:
    INT_TYPE        // { $$ = INTEGER; }
  | FLOAT_TYPE      // { $$ = FLOAT; }
  | BOOL_TYPE       // { $$ = BOOL; }
  | STRING_TYPE     // { $$ = STRING; }
  ;

program: 
  stmts
  ;

stmts:
  | stmts stmt
  ;

stmt:
    code_block
  | assignment ';'
  | declaration ';'
  | function_declaration
  | expr ';'
  | RETURN expr ';'
  | PRINT expr ';'
  | if_stmt
  | while_stmt
  | for_stmt
  | switch_stmt
  | ERROR
  | ';'
  ;

code_block:
   '{' stmts '}'
  ;

assignment:
  IDENTIFIER '=' expr
  ;

declaration:
    type IDENTIFIER
  | type IDENTIFIER '=' expr
  | CONST_TYPE type IDENTIFIER '=' expr
  // for enums
  | ENUM_TYPE IDENTIFIER '{' parameter_list '}'
  | IDENTIFIER IDENTIFIER
  | IDENTIFIER IDENTIFIER '=' expr
  ;

expr:
  // TODO: add other rules for enums and function invokations
    IDENTIFIER
  | INTEGER
  | FLOAT
  | BOOL
  | STRING
  | expr_in_parenthsis

  // arithmetic operations
  | MINUS expr %prec UMINUS
  | expr PLUS expr
  | expr MINUS expr
  | expr MULT expr
  | expr DIV expr

  // comparison operations
  | expr LT expr
  | expr GT expr
  | expr LE expr
  | expr GE expr
  | expr EQ expr
  | expr NE expr

  // logical operations
  | expr AND expr
  | expr OR expr
  | NOT expr
  ;

expr_in_parenthsis:
    '(' expr ')'
  ;

function_declaration:
  type IDENTIFIER '(' typed_parameter_list ')' code_block
  ;

parameter_list:
    parameter_list ',' IDENTIFIER
  | IDENTIFIER
  ;

typed_parameter_list:
    typed_parameter_list ',' type IDENTIFIER
  | type IDENTIFIER
  |
  ;

function_invokation:
    IDENTIFIER '(' argument_list ')'
  ;

argument_list:
    argument_list ',' expr
  | expr
  |
  ;

if_stmt:
    if_part
  | if_part ELSE
  ;

if_part:
    IF expr_in_parenthsis
  ;

while_stmt:
    WHILE
  ;

for_stmt:
    // Note: We are creating a new scope here for the (optional) loop variable
    // so it doesn't conflict with variables from the parent scope.
    FOR
    code_block
  ;

optional_declaration:
  // will be used in for_stmt
  | declaration
  ;

optional_assignment:
  // will be used in for_stmt
  | assignment
  ;

switch_stmt:
    // Note: A switch statement has to have atleast one CASE branch.
    SWITCH expr_in_parenthsis
    '{' switch_branches switch_default_branch '}'
  ;

switch_branches:
    switch_branches switch_case_branch
  | switch_case_branch
  ;

switch_case_branch:
    CASE
  ;

switch_default_branch:
  // Note: there might be no default branch.
  | DEFAULT ':' code_block
  ;

%%

int main(int argc, char** argv) {

   #if defined(YYDEBUG) && (YYDEBUG==1)
       yydebug = 1;
   #endif
  yyparse();

  return 0;
}
