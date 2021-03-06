

# Interpreter for classical One Instruction Language


 ```
Compile : gcc -Wall -o in interpreter.c constants.c
Run: ./in -i
Help: ./in -h
```

File structure
1. interpreter.c : the main function to simulate and process the interpreter
2. constants.c, constants.h : holds majority of the prompts used
3. Several oil macros
    - add
    - mult
    - div
    - exp
    - mod
    - gcd
    - pseudo_random

See below for the main options to use in the interpreter

Import <macro>
------------------------------------------------------------------------------------------------
To import a macro from your directory
Automatic import of dependencies, if present

New
------------------------------------------------------------------------------------------------
1. Prompt:
```
name : name of the new macro
length : length of the program, you can change this later using the command change_size
Num of input variables 
    names of input variables
name of result variable : OIL gives one response, but we display all the current values of the variables and return the result
```

2. Program creation:

line_number , - {varline,macro line, * , change_size, change <line_num> }   
^ automatic

```
1. varline :  x, y, t => x = x-y if now x==0, go to t else go to next line
2. macro line : <macro_path> x1, x2, x3, .... xn y
computes the macro accoriding to curr values of input_vars(x s) and stores the result in y
(xi s and y are variables of the current program)
Automatically goes to the next line
3. * : Shows all the previous lines that refer to the current line
4. change_size: changes the size of the current program
5. change <line_num> : Prompt to change the line_num
```

At the end, you are asked if you want to save the new macro, if you type y=> saves in the current directoy

Run
------------------------------------------------------------------------------------------------
```
prog_name : name of the program you want to run
Initial variable values : can use any for auxillary inputs
```

 Features:
Basic infinite loop detection by checking if a line is reached with exactly the same values at a given point

 ------------------------------------------------------------------------------------------------
