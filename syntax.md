## Compiler-Quadruples
> Imaginary programming language compiler using Lex & Yacc

## Language is similar to C or C++

- almost each line should end with a semicolon `;`

- Its a typed language; meaning each variable should have a type similarly each function should have a return type.
    
    - In case of having a `void` type function, for now you can declare it as an `int` function and have it returning `zero`

- To print an `Expression` or a printable `Variable` use `print` keyword: 
    - ` print "hello world"; `
    - ` int x = 5; print  x; `

## Supported Data Types

- Integer (int), Float (flt), Boolean (bool), String (str)

- you can have also Constants (const), and Enums (enum)

## Supported Operations

- Arithmetic operations like `+, -, *, /` are supported between numerical types `(bool, int, flt)` 

- two strings can be concatenated using `+` operator

- Comparison operations `<, <=, >, >=, ==, !=` are supported between cast-able types like numerical types can compared with one anther and also strings can be compared together

- Logical operation `Not(!), And(&&), Or(||)` are supported only between Logical types


## Variables

- Declaration: `int x; flt y; bool flag; string sval`

- Assignment: `x = 5; flt fvalue = 10.5;`

- Writing a Constant: `const str HELLO = "hello";`

- Declaring an Enum: `enum Grade { A, B, C } `

## Functions

### Definition:
- Functions can be like this:

    ```cpp
    int printHello() {
        print "hello world";

        return 0;
    }
    ```
- Or it can have some typed list of arguments:

    ```cpp
    int adder(int a, int b) {
        return a + b;
    }
    ```

### Invocation:
- call a function like `printHello();` or `print adder(5, 4.5);`

## Control Statements

### IF, IF-ELSE statements: 
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
### Switch statement: 
- it should have at least one `case` branch and/or a `default` branch

    ```cpp
    int password = 60;
    switch (password) {
        case 86: { print "correct!"; }
        default: { print "try harder"; }
    }
    ```

## Loop Statements

### While Loop:
- using keyword `while`

    ``` cpp
    int x = 1;
    while (x < 5) {
        print x;
        x = x + 1;
    }
    ```
### For Loop:
- it has optional declaration and assignment of looping variable

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






