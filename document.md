# Compiler-Quadruples
> Imaginary programming language compiler using Lex & Yacc

# Team Members

| Name                  | Section | Bench Number   |
|-----------------------|---------|----------------|
| Ahmed Emad            |    01   |      07        |
| Ahmed Mohame Ibrahim  |    01   |      08        |
| Ahmed Hafez           |    01   |      11        |
| Gaser Ashraf          |    01   |      23        | 

# Project Overview
- almost each line should end with a semicolon `;`
- Its a typed language; meaning each variable should have a type similarly each function should have a return type.
    - In case of having a `void` type function, for now you can declare it as an `int` function and have it returning `zero`
- To print an `Expression` or a printable `Variable` use `print` keyword: 
    - ` print "hello world"; `
    - ` int x = 5; print  x; `

- Supported Data Types
    - Integer (int), Float (flt), Boolean (bool), String (str)
    - you can have also Constants (const), and Enums (enum)

- Supported Operations
    - Arithmetic operations like `+, -, *, /` are supported between numerical types `(bool, int, flt)` 
    - two strings can be concatenated using `+` operator
    - Comparison operations `<, <=, >, >=, ==, !=` are supported between cast-able types like numerical types can compared with one anther and also strings can be compared together
    - Logical operation `Not(!), And(&&), Or(||)` are supported only between Logical types

- Variables
    - Declaration: `int x; flt y; bool flag; string sval`
    - Assignment: `x = 5; flt fvalue = 10.5;`
    - Writing a Constant: `const str HELLO = "hello";`
    - Declaring an Enum: `enum Grade { A, B, C } `

- Functions Definition and Invocation
    ```cpp
    int adder(int a, int b) {
        return a + b;
    }
    print adder(5, 4.5);
    ```
- Control Statements

- IF, IF-ELSE statements: 
    - `IF` statement: `int x = 5; if (x == 5) { print "x is 5!"; }`
    - `IF-ELSE` statement:
    ```cpp
    int grade = 10;
    if (grade < 5) { print "low grade :("; }
    else {print "highe grade :)"; } 
    ```

    - there's yet no `else if` statement , so to nest more `IF-Else` Statements:
    ```cpp
    if (x < 5) { print "L"; } 
    else { 
        if (x == 5) { print "exact"; } 
        else { print "W"; } 
    }
    ```
- Switch statement: 
    - it should have at least one `case` branch and/or a `default` branch
    ```cpp
    int password = 60;
    switch (password) {
        case 86: { print "correct!"; }
        default: { print "try harder"; }
    }
    ```

- Loop Statements
    - While Loop:
    ``` cpp
    int x = 1;
    while (x < 5) {
        print x;
        x = x + 1;
    }
    ```
    - For Loop:
    ```cpp
    for(int i = 0; i < 10; i=i+1) { print i; }

    int x = 0; 
    for(; x < 10; x=x+1) { print x; }

    for(int k = 0; k < 10;) {
        print k;
        k = k + 1;
    }

    int a = 0; 
    for(; a < 10; ) { 
        print a; 
        a = a + 1;
    }
    ```

# List of Tokens
- Data Type: INT_TOKEN, FLOAT_TYPE, BOOL_TYPE, STRING_TYPE, ENUM_TYPE, CONST_TYPE
- Control Statments: IF, ELSE, SWITCH, CASE, DEFAULT, RETURN
- Loop Statments: FOR, WHILE, REPEAT, UNTIL 
- Operations Tokens: PLUS, MINUS, MULT, DIV, NOT, AND, OR, EQ, NE, GT, LT, GE, LE,
- Other Tokens: PRINT, DOUBLE_COLON

# Language Production Rules

- the `yacc` program starts from this symbol by defualt
```
program: 
  stmts 
  ;
```

- the parser takes each of those lexer tokens that represent data type
and stores the data itself by the following rule.
```
type:
    INT_TYPE        { $$ = INTEGER; }
  | FLOAT_TYPE      { $$ = FLOAT; }
  | BOOL_TYPE       { $$ = BOOL; }
  | STRING_TYPE     { $$ = STRING; }
  ;
```

- the `stmts` rule is the second most generic rule since it can be empty or it can
- be stmt which is in turn expands to another rules.
```
stmts:
   stmts stmt 
   | %empty 
  ;

stmt:
    code_block
  | assignment ';'
  | declaration ';'
  | function_declaration
  | expr ';'  
  | print_stmt ';'
  | return_stmt ';'
  | if_stmt 
  | while_stmt 
  | for_stmt 
  | repeat_until_stmt ';' 
  | switch_stmt 
  | ERROR 
  | ';' 
  ;
```


- the `print_stmt` and the `return_stmt` are just an intermediate rules
```
print_stmt:
   PRINT expr 
  ;

return_stmt:
   RETURN expr 
  ;
```

- here comes the most generic statement the `code_block` since it can carry `stmts` itself
it represents scopes `{ ... }`
```
code_block:
  '{' { enterScope(); } stmts '}'
  ;
```

- the following is the rules needed to have `assignment` and `declaration` of `IDENTIFIER`s
whether they're constants, enums, or just variables
```
assignment:
  IDENTIFIER '=' expr 
  ;

declaration:
    type IDENTIFIER 
  | type IDENTIFIER '=' expr 
  | CONST_TYPE type IDENTIFIER '=' expr
  | ENUM_TYPE IDENTIFIER '
  | IDENTIFIER IDENTIFIER
  | IDENTIFIER IDENTIFIER '=' expr
  ;
```

- when intitializing an `Enum` with some variants, this rule what gets the job done
so it with the declaration rule can lead to statments like: `enum grade {a, b, c};`
```
enum_variants:
    enum_variants ',' IDENTIFIER     
  | IDENTIFIER                       
  ;
```

- the following what represents an `Expression` in this imaginary programming language
they're the data itself like `5, 5.5, true, "string value"` and the operations that happens 
between any of them, the actions of this rule also performs some checks for `type conversions`

```
expr:
    IDENTIFIER 
  | INTEGER 
  | FLOAT
  | BOOL
  | STRING
  | expr_in_parenthsis        
  
  // to have `grade x = grade::a` i.e for enums
  | IDENTIFIER DOUBLE_COLON IDENTIFIER

  // it's an expression to have something like `print func(a,b);`
  | function_invokation

  // arithmitic operations
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

  // bitwise operations (works only with `bool` type)
  | expr AND expr
  | expr OR expr
  | NOT expr
  ;

expr_in_parenthsis:
    '(' expr ')'    
  ;
```


- next is how to declare a function, so you can have something like:
```
int add (int b, flt c) {
 return b + c;
}
```

storing the types of the parameters for function itself requires some typedlist for that

```
typed_parameter_list:
    typed_parameter_list ',' type IDENTIFIER
  | type IDENTIFIER
  | %empty 
  ;
```

then comes the full rule that makes such declaration happen correctly

```
function_declaration:
  type IDENTIFIER  '(' typed_parameter_list ')' code_block 
  ;
```

- and to call/invoke it, just do almost the same, but store the types of the 
arguments the function will be called with which ease the comparison of the 
function parameters and the passed arguments for different checks

```
argument_list:
    argument_list ',' expr  
  | expr                    
  | %empty                  
  ;
```

and calling / invoking the function comes with rule to have `print add(5, 9);`

```
function_invokation:
    IDENTIFIER '(' argument_list ')' 
  ;
```

- the following is the `if-else` control statments, it's broken into multiple rules
for easier understanding and to control the actions better

```
if_stmt:
    if_part optional_else_part
  ;

if_part:
    IF expr_in_parenthsis code_block 
  ;
  
optional_else_part:
  %empty 
  | ELSE code_block 
  ;
```

- next is the `while` loop, simple token, condition and body

```
while_stmt:
    WHILE expr_in_parenthsis code_block 
  ;
```

- and the `for` loop, having that optional_declaration and optional_assignment allows
for syntax with and without those parts

```
for_stmt:
    FOR '(' optional_declaration  ';' expr ';' optional_assignment ')' 
    code_block
  ;

optional_declaration:
  %empty 
  | declaration
  ;

optional_assignment:
  %empty 
  | assignment
  ;
 ```

- another loop statment `repeat { ... } until(cond); `

```
repeat_until_stmt:
  REPEAT code_block UNTIL expr_in_parenthsis 
  ;
```

- finally is the production rule for the `switch-case` control statment, it must have 
at least one `case` and an optional `default` block, no need for `BREAK` since here
scopes are used as well and each `case` is ended with the end of its body

```
switch_stmt:
    SWITCH expr_in_parenthsis '{' switch_branches optional_switch_default_branch '}'
  ;

switch_branches:
    switch_branches switch_case_branch 
  | switch_case_branch 
  ;

switch_case_branch:
    CASE expr ':' code_block 
  ;

optional_switch_default_branch:
  %empty 
  | DEFAULT ':' code_block 
  ;
```

# Quadruples List
- this language is `stack` based, meaning pushing to the stack and functions
or even type conversions will be done to the top of stack which causes another pop and so on.

| Quad                  | Description                                           |
|-----------------------|--------------------------------------------------------
| push <expr>           | pushing an expression onto stack                      |
| pop                   | pop expression                                        |
| push tmp              | create tmp var on fly, push it to stack               |        
| pop  tmp              | poping that tmp var                                   |
| push v_<var>          | pushing a variable on the stack                       |
| pop v_<var>           | poping it                                             |
| push s_<str>          | pushing a string                                      |
| def f_<func>          | define a function variable                            |
| ret                   | return from functions                                 |
| call f_<func>         | perform the function call                             |
| print <expr>          | calling sort of a system print statment               |       
| neg                   | negate the expression on the top of the stack         |              
| add,sub,mult,div      | add,sub,mult,div the top 2 expressions on the stack   |                     
| lt,gt,le,ge,eq,ne     | comparing the top 2 expressoins on the stack          |             
| qand,qor              | bitwise and, bitwise or of the top 2 expressions      |                  
| qnot                  | perform bitwise not over the top of the stack         |               
| jmp                   | unconditional jmp to different lables                 |       
| jz,jnz                | jump if the top of the stack is(not) zero             |
| label                 | to have a label to jump to with jmp,jz,jnz            |            
|                       |                                                                                |
   
   
   

