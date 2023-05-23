%{
  #include <iostream>
  #include <string>
  #include <vector>
  #include <memory>
  #include "parser.hpp"
  #define all(v) (v).begin(), (v).end()
  using namespace std;

  extern int yylex();
  void yyerror(string e) {}

  extern FILE* yyin;

  string cleanStr(char* s) {
    string ret(s);
    free(s);
    return ret;
  }

  vector<GStmt*> gstmtv;
  vector<ExprStmt*> exprv;
  vector<SwitchStmt*> switchv;
  vector<ForStmt*> forv;
  vector<WhileStmt*> whilev;
  vector<IFStmt*> ifv;
  vector<TypedList*> typedlistv;

  template <typename T>
  struct vdel {
    void operator()(T* ptr) const {
      delete ptr;
    }
  };

  void cleanup() {
    for_each(all(gstmtv), vdel<GStmt>());
    for_each(all(exprv), vdel<ExprStmt>());
    for_each(all(switchv), vdel<SwitchStmt>());
    for_each(all(forv), vdel<ForStmt>());
    for_each(all(whilev), vdel<WhileStmt>());
    for_each(all(ifv), vdel<IFStmt>());
    for_each(all(typedlistv), vdel<TypedList>());

    gstmtv.clear();
    exprv.clear();
    switchv.clear();
    forv.clear();
    whilev.clear();
    ifv.clear();
    typedlistv.clear();
  }

  void closeFiles() {
    symlog.close();
    outputFile.close();
  }
  
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
%type <struct GStmt*> assignment declaration optional_declaration optional_assignment

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
  stmts { exitScope(); outputFile << $1->repr(); /* cleanup();*/ } 
  ;

stmts:
   stmts stmt { string repr = $2->repr(); $$ = $1->append(repr);  logSymbolTable(); }
   | %empty { $$ = new GStmt(""); gstmtv.push_back($$); }
  ;

stmt:
    code_block
  | assignment ';'
  | declaration ';'
  | function_declaration
  | expr ';'  { $$ = new GStmt($1->repr() + pop()); gstmtv.push_back($$); }
  | print_stmt ';'
  | return_stmt ';'
  | if_stmt { $$ = new GStmt($1->repr()); gstmtv.push_back($$); }
  | while_stmt { $$ = new GStmt($1->repr()); gstmtv.push_back($$); }
  | for_stmt { $$ = new GStmt($1->repr()); gstmtv.push_back($$); }
  | switch_stmt { $$ = new GStmt($1->repr()); gstmtv.push_back($$); }
  | ERROR { syntax_error_msg; }
  | ';' { $$ = new GStmt(""); gstmtv.push_back($$); }
  ;

print_stmt:
   PRINT expr { $$ = new GStmt($2->repr() + print()); gstmtv.push_back($$); }
  ;

return_stmt:
   RETURN expr { validFuncReturnType($2->getExpr()); $$ = new GStmt($2->repr() + ret()); gstmtv.push_back($$); }
  ;

code_block:
   '{' { enterScope(); } stmts '}' { exitScope(); $$ = new GStmt($3->repr()); gstmtv.push_back($$); }
  ;

assignment:
  IDENTIFIER '=' expr 
  { 
    string name = cleanStr($1);
    shared_ptr<VarID> id = getID<VarID>(name);
    id->setExpr($3->getExpr());
    string repr = $3->repr() + popv(name, id->scope);
    $$ = new GStmt(repr);
    gstmtv.push_back($$);
  }
  ;

declaration:
    type IDENTIFIER 
    { 
      string name = cleanStr($2);
      shared_ptr<VarID> id (new VarID($1, name));
      declareID(id);
      $$ = new GStmt("");
      gstmtv.push_back($$);
    }
  | type IDENTIFIER '=' expr 
    { 
      string name = cleanStr($2);
      shared_ptr<VarID> id (new VarID($1, name));
      id->setExpr($4->getExpr());
      declareID(id);
      string repr = $4->repr() + popv(name, id->scope);
      $$ = new GStmt(repr);
      gstmtv.push_back($$);
    }
  | CONST_TYPE type IDENTIFIER '=' expr
    { 
      string name = cleanStr($3);
      shared_ptr<VarID> id (new VarID($2, name, $5->getExpr()));
      declareID(id);
      string repr = $5->repr() + popv(name, id->scope);
      $$ = new GStmt(repr);
      gstmtv.push_back($$);
    }
  // for enums
  | ENUM_TYPE IDENTIFIER '{' enum_variants '}'
    {
      string name = cleanStr($2);
      shared_ptr<EnumID> eid(new EnumID(name, $4));
      declareID(eid);
      $$ = new GStmt("");
      gstmtv.push_back($$);
    }
  | IDENTIFIER IDENTIFIER
    {
      string eid_name = cleanStr($1);
      string vid_name = cleanStr($2);
      shared_ptr<EnumID> eid = getID<EnumID>(eid_name);
      shared_ptr<VarID> vid(new VarID(vid_name, eid_name));
      declareID(vid);
      $$ = new GStmt("");
      gstmtv.push_back($$);
    }
  | IDENTIFIER IDENTIFIER '=' expr
    {
      string eid_name = cleanStr($1);
      string vid_name = cleanStr($2);
      shared_ptr<EnumID> eid = getID<EnumID>(eid_name);
      shared_ptr<VarID> vid(new VarID(vid_name, eid_name));
      vid->setExpr($4->getExpr());
      declareID(vid);
      string repr = $4->repr() + popv(vid_name, vid->scope);
      $$ = new GStmt(repr);
      gstmtv.push_back($$);
    }
  ;

expr:
    IDENTIFIER 
    { 
      string name = cleanStr($1);
      shared_ptr<VarID> id = getID<VarID>(name);
      string repr = pushv(name, id->scope);
      $$ = new ExprStmt(id->getExpr(), repr);
      exprv.push_back($$);
    }
  | INTEGER 
    { 
      shared_ptr<Expr> expr(new Expr(Value($1)));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
      exprv.push_back($$);
    }
  | FLOAT
    { 
      shared_ptr<Expr> expr(new Expr(Value($1)));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
      exprv.push_back($$);
    }
  | BOOL
    {
      shared_ptr<Expr> expr(new Expr(Value($1)));
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
      exprv.push_back($$);
    }
  | STRING
    {
      string name = cleanStr($1);
      shared_ptr<Expr> expr(new Expr(Value(name)));
      string repr = pushs(expr->repr());
      $$ = new ExprStmt(expr, repr);
      exprv.push_back($$);
    }

  | expr_in_parenthsis        { $$ = $1; exprv.push_back($$); }

  // enums
  | IDENTIFIER DOUBLE_COLON IDENTIFIER
    {
      string eid_name = cleanStr($1);
      string vid_name = cleanStr($3);
      shared_ptr<EnumID> eid = getID<EnumID>(eid_name);
      int variant = eid->getVariant(vid_name);
      shared_ptr<Expr> expr(new EnumExpr(eid->name, variant));  // ??
      string repr = push(expr->repr());
      $$ = new ExprStmt(expr, repr);
      exprv.push_back($$);
    }

  // to be able to do something like `print adder(1, 2.2);`
  | function_invokation       { $$ = $1; exprv.push_back($$); }

  // arithmetic operations
  | MINUS expr %prec UMINUS
    { 
      shared_ptr<Expr> expr(-*($2->getExpr()));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $2->repr() + neg();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr PLUS expr         
    {
      shared_ptr<Expr> expr(*($1->getExpr())+$3->getExpr());
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + add();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr MINUS expr        
    {
      shared_ptr<Expr> expr(*($1->getExpr())-$3->getExpr());
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + sub();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr MULT expr         
    {
      shared_ptr<Expr> expr(*($1->getExpr())*$3->getExpr());
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + mult();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr DIV expr          
    {
      shared_ptr<Expr> expr(*($1->getExpr())/$3->getExpr());
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + div();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }

  // comparison operations
  | expr LT expr           
    {
      bool res = *($1->getExpr())<$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + lt();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr GT expr           
    {
      bool res = *($1->getExpr())>$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + gt();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr LE expr           
    {
      bool res = *($1->getExpr())<=$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + le();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr GE expr           
    {
      bool res = *($1->getExpr())>=$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + ge();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr EQ expr           
    {
      bool res = *($1->getExpr())==$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + eq();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr NE expr           
    {
      bool res = *($1->getExpr())!=$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + ne();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr AND expr
    { 
      bool res = *($1->getExpr())&&$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + qand();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | expr OR expr
    { 
      bool res = *($1->getExpr())||$3->getExpr();
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $1->repr() + $3->repr() + qor();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  | NOT expr
    { 
      bool res = (!*($2->getExpr()));
      shared_ptr<Expr> expr(new Expr(Value(res)));
      ExprStmt *estmt = new ExprStmt(expr);
      string repr = $2->repr() + qnot();
      estmt->setRepr(repr);
      $$ = estmt;
      exprv.push_back($$);
    }
  ;

expr_in_parenthsis:
    '(' expr ')'    { $$ = $2; }
  ;

enum_variants:
    enum_variants ',' IDENTIFIER     { string name = cleanStr($3); $$ = $1->append(name); }
  | IDENTIFIER                       { string name = cleanStr($1); $$ = new StrList(name); }
  ;

function_declaration:
  type IDENTIFIER { enterFunc($1, $2); } '(' typed_parameter_list ')' 
  code_block 
  { 
    string name = cleanStr($2);
    exitFunc(name);
    shared_ptr<FuncID> func(new FuncID($1, name, $5));
    declareID(func);
    $$ = new GStmt(funcdef(name, current_scope) + $5->repr() + $7->repr());
    gstmtv.push_back($$);
  }
  ;

typed_parameter_list:
    typed_parameter_list ',' type IDENTIFIER
    { 
      string name = cleanStr($4);
      string repr = popv(name, current_scope);
      shared_ptr<VarID> var(new VarID($3, name));
      var->setExpr(shared_ptr<Expr>(new Expr(type2Default[$3])));
      declareID(var);
      $$ = $1->append($3, repr);
    }
  | type IDENTIFIER
    { 
      string name = cleanStr($2);
      string repr = popv(name, current_scope);
      shared_ptr<VarID> var(new VarID($1, name));
      var->setExpr(shared_ptr<Expr>(new Expr(type2Default[$1])));
      declareID(var);
      $$ = new TypedList($1, repr);
    }
  | %empty { $$ = new TypedList(); }
  ;

function_invokation:
    IDENTIFIER '(' argument_list ')' 
    { 
      string name = cleanStr($1);
      ExprStmt *estmt = new ExprStmt(callingFunc(name, $3));
      estmt->setRepr($3->repr() + funcall(name));
      $$ = estmt;
    }
  ;

argument_list:
    argument_list ',' expr  { $$ = $1->append($3->type(), $3->repr()); }
  | expr                    { $$ = new TypedList($1->type(), $1->repr()); typedlistv.push_back($$); }
  | %empty                  { $$ = new TypedList(); typedlistv.push_back($$); }
  ;

if_stmt:
   // else_part is optional
    if_part optional_else_part
    {
      IFPartList *list = new IFPartList();
      list->append($1);
      if($2 != NULL) list->append($2);
      $$ = new IFStmt(list);
      ifv.push_back($$);
    }
  ;

if_part:
    IF expr_in_parenthsis code_block { $$ = new IFPart($2->getExpr(), $3->repr()); }
  ;

optional_else_part:
  %empty { $$ = NULL; }
  | ELSE code_block { $$ = new IFPart($2->repr()); }
  ;

while_stmt:
    WHILE expr_in_parenthsis code_block 
    {
      $$ = new WhileStmt($2->repr(), $3->repr(), $2->getExpr());
      whilev.push_back($$);
    }
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
      forv.push_back($$);
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
      switchv.push_back($$);
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
  symlog.open(fout + ".log");

  // Handle syntax errors.
  try {
    if (yyparse()) syntax_error_msg;
    if (syntax_errors) {
      cerr << "Found " << syntax_errors <<" syntax error(s)" <<endl;
    }
  } catch (const runtime_error &e) {
    cerr << e.what() << endl;
  }
  cout << "Parsing complete." << endl;
  ForceSymbolTableClean();
  closeFiles();
  cleanup();

  return 0;
}
