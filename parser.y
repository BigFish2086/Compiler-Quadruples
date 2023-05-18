%{
  #include <iostream>
  #include <string>
  #include "parser.hpp"
  using namespace std;

  extern int yylex();
  void yyerror(string e) {}

  extern FILE* yyin;
%}

%define api.value.type union
%token <int> INTEGER
%token <float> FLOAT
%token <bool> BOOL
%token <char *> STRING
%token <char *> IDENTIFIER

%type <yytokentype> type
%type <struct Expr*> expr expr_in_parenthsis function_invokation
%type <struct TypedList*> typed_parameter_list argument_list
%type <struct StrList*> enum_variants

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
    INT_TYPE        { $$ = INTEGER; }
  | FLOAT_TYPE      { $$ = FLOAT; }
  | BOOL_TYPE       { $$ = BOOL; }
  | STRING_TYPE     { $$ = STRING; }
  ;

program: 
  stmts { exitScope(); }
  ;

stmts:
  | stmts stmt  //  { symtable log }
  ;

stmt:
    code_block
  | assignment ';'
  | declaration ';'
  | function_declaration
  | expr ';'  { pop(); }
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
   '{' { enterScope(); } stmts '}' { exitScope(); }
  ;

assignment:
  IDENTIFIER '=' expr { getID<VarID>($1)->setExpr($3); }
  ;

declaration:
    type IDENTIFIER { declareID(new VarID($1, $2)); }
  | type IDENTIFIER '=' expr 
    { 
      VarID* id = new VarID($1, $2, true);
      declareID(id);
      id->setExpr($4);
    }
  | CONST_TYPE type IDENTIFIER '=' expr
    { 
      VarID* id = new VarID($2, $3, $5);
      declareID(id);
      popv(id->name, id->scope);
    }
  // for enums
  | ENUM_TYPE IDENTIFIER '{' enum_variants '}'
  | IDENTIFIER IDENTIFIER
  | IDENTIFIER IDENTIFIER '=' expr
  ;

expr:
    IDENTIFIER                { VarID *id = getID<VarID>($1); $$ = id->getExpr(); pushv($1, id->scope); }
  | INTEGER                   { $$ = new Expr(Value($1), true); push($1); }
  | FLOAT                     { $$ = new Expr(Value($1), true); push($1); }
  | BOOL                      { $$ = new Expr(Value($1), true); push($1); }
  | STRING                    { $$ = new Expr(Value($1), true); pushs($1); }
  | expr_in_parenthsis        { $$ = $1; }

  // enums
  | IDENTIFIER '.' IDENTIFIER

  // to be able to do something like `print adder(1, 2.2);`
  | function_invokation       { $$ = $1; }

  // arithmetic operations
  | MINUS expr %prec UMINUS   { $$ = -(*$2); neg(); }
  | expr PLUS expr            { $$ = *$1+$3; add(); }
  | expr MINUS expr           { $$ = *$1-$3; sub(); }
  | expr MULT expr            { $$ = *$1*$3; mult(); }
  | expr DIV expr             { $$ = *$1/$3; div(); }

  // comparison operations
  | expr LT expr              { $$ = new Expr(Value(*$1<$3), true); lt(); }
  | expr GT expr              { $$ = new Expr(Value(*$1>$3), true); gt(); }
  | expr LE expr              { $$ = new Expr(Value(*$1<=$3), true); le(); }
  | expr GE expr              { $$ = new Expr(Value(*$1>=$3), true); ge(); }
  | expr EQ expr              { $$ = new Expr(Value(*$1==$3), true); eq(); }
  | expr NE expr              { $$ = new Expr(Value(*$1!=$3), true); ne(); }

  // logical operations - TODO: solve the resultant error
  // | expr AND expr             { $$ = new Expr(Value(*$1&&$3), true); and(); }
  // | expr OR expr              { $$ = new Expr(Value(*$1||$3), true); or(); }
  // | NOT expr                  { $$ = new Expr(Value(!(*$2)), true); not(); }
  ;

expr_in_parenthsis:
    '(' expr ')'    { $$ = $2; }
  ;

enum_variants:
    enum_variants ',' IDENTIFIER     { $$ = $1->append($3); }
  | IDENTIFIER                       { $$ = new StrList($1); }
  ;

function_declaration:
  type IDENTIFIER { enterFunc($1); } '(' typed_parameter_list ')' code_block { exitFunc(); declareID(new FuncID($1, $2, $5)); funcHasReturnStatment($2);  }
  ;

typed_parameter_list:
    typed_parameter_list ',' type IDENTIFIER
    { 
      $$ = $1->append($3);
      declareID(new VarID($3, $4, true));
    }
  | type IDENTIFIER
    { 
      $$ = new TypedList($1); 
      declareID(new VarID($1, $2, true)); 
    }
  | { $$ = new TypedList(); }
  ;

function_invokation:
    IDENTIFIER '(' argument_list ')' { $$ = callingFunc($1, $3); }
  ;

argument_list:
    argument_list ',' expr  { $$ = $1->append($3->type()); }
  | expr                    { $$ = new TypedList($1->type()); }
  |                         { $$ = new TypedList(); }
  ;

if_stmt:
    if_part
  | if_part /*{}*/ ELSE code_block /*{}*/
  ;

if_part:
    IF expr_in_parenthsis /*{}*/ code_block /*{}*/
  ;

while_stmt:
    WHILE /*{}*/ expr_in_parenthsis /*{}*/ code_block /*{}*/
  ;

for_stmt:
    // Note: We are creating a new scope here for the (optional) loop variable
    // so it doesn't conflict with variables from the parent scope.
    FOR /*{}*/ '(' optional_declaration /*{}*/ ';' expr /*{}*/ ';' optional_assignment ')' /*{}*/ code_block /*{}*/
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
    // Note: A switch statement has to have at least one CASE branch.
    SWITCH expr_in_parenthsis
    '{' switch_branches switch_default_branch '}'
  ;

switch_branches:
    switch_branches switch_case_branch
  | switch_case_branch
  ;

switch_case_branch:
    CASE /*{}*/ expr /*{}*/ ':' code_block /*{}*/
  ;

switch_default_branch:
  // Note: there might be no default branch.
  | DEFAULT ':' code_block
  ;

%%

int main(int argc, char** argv) {

   yyin = fopen(argv[1], "r");
   #if defined(YYDEBUG) && (YYDEBUG==1)
       yydebug = 1;
   #endif

  fout = argv[1];
  output.open(fout + ".q");
  output << fixed << setprecision(5);

  // Handle syntax errors.
  if (yyparse()) log_syntax;
  if (syntax_errors) {
    cerr << "Found " << syntax_errors <<" syntax error(s)" <<endl;
    abort();
  }

  return 0;
}
