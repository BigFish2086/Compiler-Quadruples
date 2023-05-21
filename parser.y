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
%type <struct ExprStmt*> expr expr_in_parenthsis function_invokation
%type <struct TypedList*> typed_parameter_list argument_list
%type <struct StrList*> enum_variants

%type <struct GStmt*> stmts stmt code_block function_declaration print_stmt return_stmt
%type<struct GStmt*> assignment declaration optional_declaration optional_assignment

%type <struct IFPart*> if_part optional_else_part
%type <struct IFStmt*> if_stmt

%type <struct WhileStmt*> while_stmt
%type <struct ForStmt*> for_stmt

%type <struct CaseStmt*> switch_case_branch optional_switch_default_branch
%type <struct CaseStmtList*> switch_branches
%type <struct SwitchStmt*> switch_stmt

%token ERROR PRINT RETURN DOUBLE_COLON
%token WHILE FOR
%token IF ELSE SWITCH CASE DEFAULT
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
  stmts { exitScope(); outputFile << $1->repr(); } 
  ;

stmts:
   stmts stmt  { string repr = $2->repr(); $$ = $1->append(repr); }
   | %empty { $$ = new GStmt(""); }
  ;

stmt:
    code_block
  | assignment ';'
  | declaration ';'
  | function_declaration
  | expr ';'  { pop(); $$ = new GStmt(""); }
  | print_stmt ';'
  | return_stmt ';'
  | if_stmt { $$ = new GStmt($1->repr()); }
  | while_stmt { $$ = new GStmt($1->repr()); }
  | for_stmt { $$ = new GStmt($1->repr()); }
  | switch_stmt { $$ = new GStmt($1->repr()); }
  | ERROR { syntax_error_msg; }
  | ';' { $$ = new GStmt(""); }
  ;

print_stmt:
   PRINT expr { $$ = new GStmt($2->repr() + print()); }
  ;

return_stmt:
   RETURN expr { validFuncReturnType($2->getExpr()); $$ = new GStmt($2->repr() + ret()); }
  ;

code_block:
   '{' { enterScope(); } stmts '}' { exitScope(); $$ = new GStmt($3->repr()); }
  ;

assignment:
  IDENTIFIER '=' expr 
  { 
    VarID *id = getID<VarID>($1);
    id->setExpr($3->getExpr());
    string repr = $3->repr() + popv($1, id->scope);
    $$ = new GStmt(repr);
  }
  ;

declaration:
    type IDENTIFIER { declareID(new VarID($1, $2)); $$ = new GStmt(""); }
  | type IDENTIFIER '=' expr 
    { 
      VarID* id = new VarID($1, $2);
      declareID(id);
      id->setExpr($4->getExpr());
      string repr = $4->repr() + popv($2, id->scope);
      $$ = new GStmt(repr);
    }
  | CONST_TYPE type IDENTIFIER '=' expr
    { 
      VarID* id = new VarID($2, $3, $5->getExpr());
      declareID(id);
      string repr = $5->repr() + popv($3, id->scope);
      $$ = new GStmt(repr);
    }
  // for enums
  | ENUM_TYPE IDENTIFIER '{' enum_variants '}'
    {
      EnumID *id = new EnumID($2, $4);
      declareID(id);
      $$ = new GStmt("");
    }
  | IDENTIFIER IDENTIFIER
    {
      EnumID *eid = getID<EnumID>($1);
      VarID *vid = new VarID($2, eid->name);
      declareID(vid);
      $$ = new GStmt("");
    }
  | IDENTIFIER IDENTIFIER '=' expr  // should be EnmStmt(EnumExpr)
    {
      EnumID *eid = getID<EnumID>($1);
      VarID *vid = new VarID($2, eid->name);
      vid->setExpr($4->getExpr());
      declareID(vid);
      $$ = new GStmt("");
    }
  ;

expr:
    IDENTIFIER 
    { 
      VarID *id = getID<VarID>($1);
      string repr = pushv($1, id->scope);
      $$ = new ExprStmt(id->getExpr(), repr);
    }
  | INTEGER 
    { 
      Expr *expr = new Expr(Value($1));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
    }
  | FLOAT
    { 
      Expr *expr = new Expr(Value($1));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
    }
  | BOOL
    {
      Expr *expr = new Expr(Value($1));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
    }
  | STRING
    {
      Expr *expr = new Expr(Value($1));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
    }

  | expr_in_parenthsis        { $$ = $1; }

  // enums
  | IDENTIFIER DOUBLE_COLON IDENTIFIER
    {
      EnumID *eid = getID<EnumID>($1);
      int variant = eid->getVariant($3);
      EnumExpr *expr = new EnumExpr(eid->name, variant);
      string repr = push($1);
      $$ = new ExprStmt(expr, repr);
    }

  // to be able to do something like `print adder(1, 2.2);`
  | function_invokation       { $$ = $1; }

  // arithmetic operations
  | MINUS expr %prec UMINUS
    { 
      Expr *expr = -*($2->getExpr());
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $2->repr() + neg();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr PLUS expr         
    {
      Expr *expr = *($1->getExpr())+$3->getExpr();
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + add();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr MINUS expr        
    {
      Expr *expr = *($1->getExpr())-$3->getExpr();
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + sub();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr MULT expr         
    {
      Expr *expr = *($1->getExpr())*$3->getExpr();
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + mult();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr DIV expr          
    {
      Expr *expr = *($1->getExpr())/$3->getExpr();
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + div();
      estmt->setRepr(repr);
      $$ = estmt;
    }

  // comparison operations
  | expr LT expr           
    {
      bool res = *($1->getExpr())<$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + lt();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr GT expr           
    {
      bool res = *($1->getExpr())>$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + gt();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr LE expr           
    {
      bool res = *($1->getExpr())<=$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + le();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr GE expr           
    {
      bool res = *($1->getExpr())>=$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + ge();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr EQ expr           
    {
      bool res = *($1->getExpr())==$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + eq();
      estmt->setRepr(repr);
      $$ = estmt;
    }
  | expr NE expr           
    {
      bool res = *($1->getExpr())!=$3->getExpr();
      Expr *expr = new Expr(Value(res));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + ne();
      estmt->setRepr(repr);
      $$ = estmt;
    }

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
  type IDENTIFIER { enterFunc($1); } '(' typed_parameter_list ')' 
  code_block 
  { 
    exitFunc(); 
    declareID(new FuncID($1, $2, $5));
    funcHasReturnStatment($2);
    $$ = new GStmt(funcdef($2, current_scope) + $5->repr());
  }
  ;

typed_parameter_list:
    typed_parameter_list ',' type IDENTIFIER
    { 
      string repr = popv($4, current_scope);
      declareID(new VarID($3, $4, true));
      $$ = $1->append($3, repr);
    }
  | type IDENTIFIER
    { 
      string repr = popv($2, current_scope);
      declareID(new VarID($1, $2, true)); 
      $$ = new TypedList($1, repr);
    }
  | %empty { $$ = new TypedList(); }
  ;

function_invokation:
    IDENTIFIER '(' argument_list ')' 
    { 
      Expr *expr = callingFunc($1, $3); 
      ExprStmt *estmt = new ExprStmt(expr);
      estmt->setRepr($3->repr() + funcall($1));
      $$ = estmt;
    }
  ;

argument_list:
    argument_list ',' expr  { $$ = $1->append($3->type(), $3->repr()); }
  | expr                    { $$ = new TypedList($1->type(), $1->repr()); }
  | %empty                  { $$ = new TypedList(); }
  ;

if_stmt:
   // else_part is optional
    if_part optional_else_part
    {
      IFPartList *list = new IFPartList();
      list->append($1);
      if($2 != NULL) list->append($2);
      $$ = new IFStmt(list);
    }
  ;

if_part:
    IF expr_in_parenthsis code_block { $$ = new IFPart($2->getExpr(), $3->repr());}
  ;

optional_else_part:
  %empty { $$ = NULL; }
  | ELSE code_block { $$ = new IFPart($2->repr()); }
  ;

while_stmt:
    WHILE expr_in_parenthsis code_block { $$ = new WhileStmt($2->repr(), $3->repr(), $2->getExpr()); }
  ;

for_stmt:
    // new scope, so for loop variables aren't visible outside scope
    FOR { enterScope(); } '(' optional_declaration  ';' expr ';' optional_assignment ')' 
    code_block
    {
      string b4 = "\n", b6 = "\n", b8 = "\n";
      if($4 != NULL) b4 = $4->repr();
      if($8 != NULL) b8 = $8->repr();
      b6 = $6->repr();
      $$ = new ForStmt(b4, b6, b8, $10->repr(), $6->getExpr());
      exitScope();
    }
  ;

optional_declaration:
  %empty { $$ = NULL; }
  | declaration
  ;

optional_assignment:
  %empty { $$ = NULL; }
  | assignment
  ;

switch_stmt:
    // default branch is optional
    SWITCH expr_in_parenthsis '{' switch_branches optional_switch_default_branch '}'
    {
      if($5 == NULL) $$ = new SwitchStmt($2->getExpr(), $4);
      else $$ = new SwitchStmt($2->getExpr(), $4->append($5));
    }
  ;

switch_branches:
    switch_branches switch_case_branch { $$ = $1->append($2); }
  | switch_case_branch { $$ = new CaseStmtList($1); }
  ;

switch_case_branch:
    CASE expr ':' code_block { $$ = new CaseStmt ($2->getExpr(), $4->repr()); }
  ;

optional_switch_default_branch:
  %empty { $$ = NULL; }
  | DEFAULT ':' code_block { $$ = new CaseStmt($3->repr()); }
  ;

%%

int main(int argc, char** argv) {

   yyin = fopen(argv[1], "r");
   #if defined(YYDEBUG) && (YYDEBUG==1)
       yydebug = 1;
   #endif

  fout = argv[1];
  outputFile.open(fout + ".q");
  outputFile << fixed << setprecision(5);

  // Handle syntax errors.
  if (yyparse()) syntax_error_msg;
  if (syntax_errors) {
    cerr << "Found " << syntax_errors <<" syntax error(s)" <<endl;
    abort();
  }

  return 0;
}
