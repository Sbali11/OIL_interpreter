/* 

Author Shreya Bali
CDM Course Project

**************************************************************************************************
                Interpreter for classical One Instruction Language
**************************************************************************************************

Compile : gcc -Wall -o in interpreter.c constants.c
Run: ./in -i
Help: ./in -h

Features of interpreter:
------------------------------------------------------------------------------------------------
import <macro>: 
To import a macro from your directory
Automatic import of dependencies, if present

------------------------------------------------------------------------------------------------
new : To create a new macro
Prompt
> name : name of the new macro
> length : length of the program, you can change this later using the command change_size
> Num of input variables 
    names of input variables
> name of result variable : OIL gives one response, but we display all the current values of the variables and return the result


Program creation:
line_number , - {varline,macro line, * , change_size, change <line_num> }
___________   
^ automatic

1. varline : 
x, y, t => x = x-y if now x==0, go to t else go to next line

2. macro line : 
<macro_name> x1, x2, x3, .... xn y , computes the macro accoriding to curr values of input_vars(x s) and stores the result in y
(xi s and y are variables of the current program)
Automatically goes to the next line

3. *: 
Shows all the previous lines that refer to the current line

4. change_size
changes the size of the current program

5. change <line_num>
Prompt to change the line_num


At the end, you are asked if you want to save the new macro, if you type y=> saves in the current directoy

------------------------------------------------------------------------------------------------

run
 > prog_name : name of the program you want to run
 > Initial variable values : can use any for auxillary inputs

 Features:
Basic infinite loop detection by checking if a line is reached with exactly the same values at a given point

 ------------------------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "constants.h"
#include "uthash.h"

/*
MACROS cannot be numbers , keywords
SCANF 
multiple outputs
*/


/************************************************************************************************
 *                               Defining Types                                                 *
*************************************************************************************************/


// Types of lines in a new program
typedef enum 
{
    EXIT,     // exit interpreter
    RESTART, // new program, discard present
    REF_LINE, // show prev lines referring to curr line
    TO_EVAL, // var_line/macro_line
    COMMENT, 
    CHANGE, // change line_num
    CHANGE_SIZE, // change size

} parse_type;


// types for to_eval lines
typedef enum
{
    VAR,
    MACRO,
    UNDEF

} cmdline_type;

// set of vars for each program
typedef struct prog_vars_t 
{
    char* var_name;             // key 
    UT_hash_handle hh;         // makes this structure hashable 

} prog_vars_set;

// contains set of vars, num of vars
typedef struct process_var_strings_t
{
    int num_input_vars;
    char** vars;
} process_vars_t; 

/* dictionary=> var_name : ref
    var_name is the name of the variable in the program referred to
    ref is the name of the variable in the current program
    mplies set var_name = ref 
*/

typedef struct prog_vars_refs_t
{
    char* var_name;                    //key 
    char* ref;
    UT_hash_handle hh;         //makes this structure hashable  

} prog_vars_refs;


/* contents of the specified var_line
format of varline:
var1, var2, t
(vars[0], vars[1], i)
*/

typedef struct var_line_t
{
    char* vars[2];
    int i;

} var_line;


/* contents of the specified macro_line
format of varline:
macro_name, x1, x2, x3, ..... xn, y
where xi's are the input vars, y is output var

- macro_name
- vrefs: dictionary of key(macro vars), value(xi's and y) matched according to input
(vars[0], vars[1], i)
*/
typedef struct macro_line_t
{
    char* macro_name;
    prog_vars_refs*  vrefs; // DICT
} macro_line;


// union for each TO_EVAL: var/macro line
union parsed_vals
{
    var_line* var;
    macro_line* macro;
};

// code_line structure for each line in the code: only consists of var_lines/macro_lines
typedef struct code_t
{
    char* line; // actual line
    cmdline_type linet; // type of line: VAR/MACRO
    union parsed_vals* pv; // info about the line(see above)
} code_line;

// for each macro(program)
typedef struct macros_t 
{
    char* prog_name;      // key : name of prog
    int line_nums;        // num of lines
    code_line** code;     // array of code_line for each line in the prog             
    prog_vars_set* all_vars; // set of vars
    char* res_var;           // name of res_var
    UT_hash_handle hh;       // makes this structure hashable 

} all_macros;

// for extracting first element in the code line
typedef struct first_key_t
{
    char* val;
    bool intended_var;
    int c;

} first_key;

// extracting number from string
typedef struct extract_number_t
{
    bool is_number;
    int number;

} extract_num;

// var_name(key): value(val)
typedef struct prog_vars_vals_t
{
    char* var_name;                    /* key */
    int value;
    UT_hash_handle hh;         /* makes this structure hashable */

} prog_vars_vals;

// set of lines the curr_line is referred by 
typedef struct lines_referred_by_t
{
    int i;                      //key: line number that refers to the curr_line 
    UT_hash_handle hh;         /* makes this structure hashable */
} lines_referred_by;

//Defining constants
all_macros* defined_progs = NULL;

// GLOBAL variables

int n = -1;


// FUNCTIONS
void err_print(const char* ERR_MSG);
char* set_program_name();
bool check_char(char c);
extract_num* extract_num_from_string(char * buffer);
bool check_vname_valid(char* var_name, prog_vars_set* all_vars);
void interpreter_mode();
void new_program();
int eval(int i, char* cmdline, code_line* cline, prog_vars_set* all_vars, bool is_file, FILE* in);
void process_program(int n, prog_vars_set* all_vars, char* prog_name, char* res_vname);
int process_file_macros(char* file_name);
void save_program(char* prog_name, code_line** code, int n, prog_vars_set* all_vars, char* res_vname);




/************************************************************************************************
 *                         Helper functions                                                     *
 *               -   set up of interpreter messages                                             *
 *               -   processing inputs                                                          *
*************************************************************************************************/


// print error message in RED
void err_print(const char* ERR_MSG)
{
    printf("\033[1;31m");
    printf("\t%s", ERR_MSG);
    printf("\033[0m");
}

// set name of program by asking user
char* set_program_name()
{
    char* name = malloc(20* sizeof(char));
    printf("%s", P_NAME_MSG);
    scanf("%s", name);
    return name;
}

// check if c is a letter
bool check_char(char c)
{
    if(c>='a' && c<='z')
        return true;
    if(c>='A' && c<='Z')
        return true;
    return false;

}

// substring:  src[offset, offset+ length], returns and saves this in dst
char *substr(char *dst, char *src, size_t offset, size_t length) 
{
    memcpy(dst, src + offset, length);
    dst[length] = '\0';
    return dst;
}

/*
extracts number from string
buffer: input string
return - res such that res-> is_number is false iff buffer is not intended as a number(except whitespaces)
                       res-> number : number conversion of string
*/
extract_num* extract_num_from_string(char * buffer) 
{
    bool digit = false;
    extract_num* res = malloc(sizeof(extract_num));
    int c = 0;
    while (buffer[c]!='\0') 
    {
        if(!digit && isdigit(buffer[c]))
        {
            digit = true;
        }

        if ( !isdigit(buffer[c]) && !isspace(buffer[c]) ) 
        {
            res -> is_number = false;
            res -> number = -1;
            return res;
        }
        c++;
    }
    res -> is_number = digit;
    res -> number = atoi(buffer);

    return res ;
}

// extracts the first word in the line
char* extract_first_word(char* line)
{
    char* val = malloc(16*sizeof(char));
    int c = 0;

    while(line[c]!=  '\0' && line[c]!= '\n' && line[c]== ' ')
    {
        c++;
    }

    while(line[c]!= '\0' && line[c] != ' ' && line[c] != ',' && line[c]!= '\n')
    {
        val[c] =  line[c];
        c++;
    }

    val[c] = '\0';

    return val;
}




/* checks if var_name is valid
Inputs
    - var_name : variable name
    - all_vars : all variables presently defined
returns true iff
    - var is entered
    - first letter is char
    - consists of chars/nums
    - not defined before as a macro/var_name
*/

bool check_vname_valid(char* var_name, prog_vars_set* all_vars)
{
    if(strlen(var_name)==0)
    {
        printf("%s\n", NO_VAR_ENTERED);
        return false;
    }

    if(!check_char(var_name[0]))
    {
        printf("\033[1;31m");
        printf("\t%s\n", var_name);
        printf("\t^ %s\n", VNAME_START_ERR);
        printf("\033[0m");
        return false;
    }

    int c = 1;
    char err_msg[strlen(var_name)+1];
    strcpy(err_msg, " ");
    char* ln_err =  "%s^Variable names can only have letters/numbers \n";
    char fin_err[strlen(ln_err)+ strlen(var_name)+1];
    while(var_name[c]!= '\0')
    {
        if(!(check_char(var_name[c]) || isdigit(var_name[c])))
        {
            
            printf("\t%s\n", var_name);
            sprintf(fin_err, ln_err, err_msg);
            err_print(fin_err);
            return false;
        }
        
        strcat(err_msg, " ");
        c++;
    }

    prog_vars_set* var;

    HASH_FIND_STR(all_vars, var_name, var);

    if(var != NULL)
    {
        err_print(VAR_NAME_EXTS_ERR);
        printf("\n");
        return false;
    }


    all_macros *macro;

    HASH_FIND_STR(defined_progs, var_name, macro);

    if(macro != NULL)
    {
        printf("\033[1;31m");
        printf("%s\n", VNAME_MACRO_ERR);
        printf("\033[0m");
        return false;
    }

    return true;

}

// check if the variable name is defined
bool check_valid_var(char* var_name, prog_vars_set* all_vars)
{
    prog_vars_set* var;

    HASH_FIND_STR(all_vars, var_name, var);
    if(var==NULL)
    {
        printf("No variable named %s defined\n", var_name);
        return false;
    }
    return true;
}

// create variables by taking in prompt inputs 
// returns set of variables
prog_vars_set* create_vars()
{
    prog_vars_set* all_vars = NULL;
    int num_vars = 0;
    bool is_valid = true;
    char* temp;
    prog_vars_set* to_add;
    prog_vars_set* z_var;

    printf("%s", NUM_VARS_MSG );
    char* val = malloc(129*sizeof(char));
    scanf("%s", val);
    extract_num* e= extract_num_from_string(val);
    if(! e -> is_number)
    {
        printf("%s\n", INVALID_NUMBER_ERR);
        free(val);
        return create_vars();

    }

    num_vars =  e -> number;
 
    if(num_vars<0)
    {
        printf("%s\n", ERROR_NUM_VARS);
        return create_vars();
    }

    if(num_vars == 0)
    {
        return all_vars;
    }
    temp = malloc(2 * sizeof(char)) ;
    strcpy(temp, "z");
    z_var = malloc(sizeof(prog_vars_set));
    z_var -> var_name =  temp;
    printf("%s\n", P_VAR_NAMES);
    HASH_ADD_STR(all_vars, var_name, z_var);
    
    for(int i = 0; i<num_vars; i++)
    {
        temp = malloc(16 * sizeof(char)) ;
        to_add = malloc(sizeof(prog_vars_set));
        printf("%d:  ", i+1);
        scanf("%s", temp);
        is_valid = check_vname_valid(temp, all_vars);
        
        if(! is_valid)
        {
            i--;
            free(temp);
            continue;
        }
        to_add->var_name = temp;
        HASH_ADD_STR(all_vars, var_name, to_add);
    }

    return all_vars;

}

// takes in prompt input for setting the length of the program
// returns entered lenght
int set_program_length()
{
    int size;
    printf("%s ", P_SIZE_MSG );
    char* val = malloc(129*sizeof(char));
    scanf("%s", val);
    extract_num* e= extract_num_from_string(val);
    if(! e -> is_number)
    {
        printf("%s\n", INVALID_NUMBER_ERR);
        free(val);
        return set_program_length();
    }

    size =  e -> number;
    free(val);
    return size;
}

/* identify type of the cmdline 
returns type
*/
parse_type exec(char *cmdline)
{
    //CHECK IF UPPER ARROW KEY

    if(!strncmp(cmdline, "exit", 4))
        return EXIT;

    if(!strncmp(cmdline, "change", 6))
        return CHANGE;

    if(!strncmp(cmdline, "change_size", 11))
        return CHANGE_SIZE;


    if (!strncmp(cmdline, "new", 3))
        return RESTART;

    if (!strncmp(cmdline, "*", 1))
        return REF_LINE;

    if(!strncmp(cmdline, "//", 2))
        return COMMENT;

    return TO_EVAL;
}

/* extracts first intended var from string 
    res -> internded_var : name of variable
    res -> c : current pointer on the string(i.e after first var)
*/

first_key* extract_first_key(char* string)
{
    int c = 0;
    bool var = false;
    first_key* res = malloc(sizeof(first_key));

    while(string[c]!='\0' && string[c] != ' ' && string[c] != ',')
    {
        c++;
    }
    char* dst  =  malloc(sizeof(char)*(c+1));
    dst = substr(dst, string, 0, c);
    res->val = dst;
    if(string[c]=='\0')
    {
        res -> c = c;
        return res;
    }

    if(string[c] != ',')
    {
        while(string[c]!='\0' && string[c] == ' ')
        {
            c++;
        }
    }

    if(string[c]==',')
        var = true;

    else
        c--;

    res -> intended_var = var;
    res-> c = c;
    return res;

}

// free code before exit
void free_code(code_line** code, int n)
{  
    int i = 1;
    var_line* var;
    macro_line* macro;
    prog_vars_refs* vref;
    prog_vars_refs* tmp;

    while(i <= n && code[i]!= NULL)
    {
        if(code[i]-> line != NULL)
            free(code[i] -> line);

        switch(code[i]-> linet)
        {
            case VAR:
                var = code[i] -> pv -> var;
                if(var!=NULL)
                {
                    if(!var-> vars[0])
                        free(var -> vars[0]);
                    if(!var-> vars[1])
                        free(var -> vars[1]);
                    free(var);
                }

                break;

            case MACRO:
                macro = code[i] -> pv -> macro;
                if(macro!=NULL)
                {
                    free(macro -> macro_name);
                    HASH_ITER(hh, macro -> vrefs, vref, tmp) 
                    {
                        HASH_DEL( macro -> vrefs, vref);  /* delete; users advances to next */
                        free(vref);            
                    } 

                }
                break;

            default:

                break;
        }
        i++;

    }
}



/************************************************************************************************
 *                                      New program functions                                   *
*************************************************************************************************/


/* create new program
Inputs
        - n : size of the program
        - all_vars : set of vars to use
        - prog_name : name of the program
        - res_vname : name of the result variable

asks for user input for each line(see above for types of inputs)
processes and parses these lines to store for future use
adds this program to the dictionary of all_programs with prog_name as the key
*/
void process_program(int n, prog_vars_set* all_vars, char* prog_name, char* res_vname)
{
    int i = 1;
    code_line** code = malloc((n+1)*sizeof(code_line*));
    int line_no;
    extract_num* line_no_ext;
    lines_referred_by** refd = malloc((n+1) * sizeof(lines_referred_by*));
    lines_referred_by* to_add_refd;
    lines_referred_by* found_ref;
    lines_referred_by* tmp;
    int line_ref;
    //char* line;
    printf(">>> Start Program %s\n", prog_name);
    int new;
    int new_l;
    code_line* new_code_line;
    char* save_opt;

    for(int j = 1; j<=n ; j++)
        refd[i] = NULL;

    while(i<=n)
    {
        code[i] = malloc(sizeof(code_line));
        code[i] -> line = malloc(129* sizeof(char));
        save_opt = malloc(128*sizeof(char));
        printf("... %d, ", i);
        scanf(" %[^\n]", code[i] -> line);
        
        if(i<n)
        {
            to_add_refd = malloc(sizeof(lines_referred_by));
            to_add_refd -> i = i;
            HASH_ADD_INT( refd[i+1], i, to_add_refd );
        }

        switch(exec(code[i] -> line))
        {
            case EXIT:
                printf("EXIT\n");
                free_code(code, i);
                exit(0);

            case TO_EVAL:
                new = eval(i, code[i] -> line, code[i], all_vars, false, stdin);
                if(new==i)
                {
                    free(code[i]-> line);
                    free(code[i]);
                }
                else
                {
                    switch(code[i]-> linet)
                    {
                        case VAR:
                            to_add_refd = malloc(sizeof(lines_referred_by));
                            to_add_refd -> i = i;
                            line_ref = code[i]-> pv-> var-> i;
                            if(line_ref<n && line_ref!= (i+1))
                                HASH_ADD_INT( refd[line_ref], i, to_add_refd );
                            else
                                free(to_add_refd);
                            break;

                        default:
                            break;
                    }
                }

                i = new;
                break;
            
            case RESTART: 
                printf("RESTART\n");
                printf("%s\n", HALTING_MSG);
                scanf("%s", save_opt);
                if(save_opt[0] == 'y')
                {
                    printf("To Save");
                    save_program(prog_name, code, n, all_vars, res_vname);
                }
                

                all_macros *var = malloc(sizeof(all_macros));
                var-> prog_name = prog_name;
                var-> code = code;
                var-> line_nums = n;
                var-> all_vars = all_vars;
                var-> res_var = res_vname;
                HASH_ADD_STR( defined_progs, prog_name, var );
                return;

            case CHANGE:
                //Change line code
                line_no_ext = extract_num_from_string((code[i]-> line)+6);
                free(code[i]-> line);
                free(code[i]);
                if(!line_no_ext -> is_number)
                {
                    printf("%s\n", CHANGE_EXPECTED_FORMAT);
                    break;
                }
                line_no = line_no_ext-> number;
                if(line_no>=i|| line_no<0)
                {
                    printf("%s\n", INVALID_LINE_MSG);
                    break;
                }

                printf("\t> current line: %d, %s\n", line_no, code[line_no]->line);
                //free(code[line_no]);
                new_code_line = malloc(sizeof(code_line));
                new_code_line-> line  = malloc(129*sizeof(char));
                printf("%s\n... %d, ", NEW_LINE_INPUT_MSG, line_no);
                scanf(" %[^\n]", new_code_line-> line);

                new_l = eval(line_no, new_code_line-> line, new_code_line, all_vars, false, stdin);
                if(new_l!=line_no)
                {
                    free(code[line_no]);
                    code[line_no] =  new_code_line;
                    printf("\t> %s\n", LINE_CHANGE_SUCCESS_MSG);
                }

                break;                

            case CHANGE_SIZE:
                free(code[i]);
                printf("%s\n", P_SIZE_MSG);
                line_no_ext = extract_num_from_string((code[i]-> line)+6);
                if(!line_no_ext -> is_number)
                {
                    printf("%s\n", CHANGE_EXPECTED_FORMAT);
                    break;
                }
                break;

            case REF_LINE:
                HASH_ITER(hh, refd[i], found_ref, tmp) 
                {
                    printf("> %d, %s\n", found_ref->i, code[found_ref->i]-> line);
                } 

                break;

            case COMMENT:

                free(code[i] -> line);
                free(code[i]);
                i++;
                break;

            default:
                printf("NOT YET IMPLEMENTED");
                break;

        }
        free(save_opt);

    }
    all_macros *var = malloc(sizeof(all_macros));
    var-> prog_name = prog_name;
    var-> code = code;
    var-> line_nums = n;
    var-> all_vars = all_vars;
    var-> res_var = res_vname;
    save_opt = malloc(128*sizeof(char));
    printf("%s: ", HALTING_MSG);
    scanf("%s", save_opt);
    if(save_opt[0] == 'y')
    {
        save_program(prog_name, code, n, all_vars, res_vname);
        printf("SAVED");
    }
    HASH_ADD_STR( defined_progs, prog_name, var );
    printf("\n");

}


/*  main function to create a new program
    - user input to prcess name, length, variables, res_var
    - uses this information to call the process_program function(see above)
*/

void new_program()
{
    printf("\n* %s\n", P_VAR);
    printf("* %s\n", P_STAR);
    printf("* %s\n", P_SIZE);
    printf("* %s\n\n", P_CHANGE);
    int size; 
    char* name;
    prog_vars_set* all_vars;
    char* res_vname;
    prog_vars_set* res_vars = malloc(sizeof(prog_vars_set));
    name = set_program_name();
    size = set_program_length();
    all_vars = create_vars();
    res_vname = malloc(16*sizeof(char));
    bool is_valid_vname =  false;

    while(!is_valid_vname)
    {
        printf("%s", P_RES_VAR);
        scanf("%s", res_vname);
        is_valid_vname = check_vname_valid(res_vname, all_vars);
    }
    res_vars -> var_name = res_vname;
    HASH_ADD_STR( all_vars, var_name, res_vars );
    process_program(size, all_vars, name, res_vname);
}


/* helper function to save the program
 n -  length of the program
 all_vars - all variables of the program
 */
process_vars_t* process_var_strings(prog_vars_set* all_vars)
{
    process_vars_t* res = malloc(sizeof(process_vars_t));
    prog_vars_set* var;
    prog_vars_set* prev;
    
    int i = 0;
    HASH_ITER(hh, all_vars, var, prev) 
    {
        i++;
    } 

    char** res_str = calloc(i, 128* sizeof(char));
    HASH_ITER(hh, all_vars, var, prev) 
    {
        res_str[i] =  all_vars-> var_name;
    } 
    res -> num_input_vars = i;
    res -> vars = res_str;
    return res;

}

/* saves the program in a file in current working directory
Inputs
    - prog_name : name of the program
    - code : code_lines of the program
    - n : length of the program
    - all_vars: set of all vars
    - res_vname : name of the result variable
*/

void save_program(char* prog_name, code_line** code, int n, prog_vars_set* all_vars, char* res_vname)
{
    printf("ENTERED\n");
    //
    FILE* file = NULL;
    file  = fopen(prog_name, "w");
    char* length_str = malloc(16 * sizeof(char));
    char* num_vars_str = malloc(16 * sizeof(char));
    char* line;
    sprintf(length_str, "%d", n);
    fputs(length_str, file);
    fputs("\n", file);
    process_vars_t*  v = process_var_strings(all_vars);
    sprintf(num_vars_str, "%d", v-> num_input_vars);
    fputs(num_vars_str, file);
    char** vars =  v-> vars;
    for(int i = 0; i< v-> num_input_vars; i++)
    {
        if(!strncmp(vars[i], res_vname, 16))
            continue;
        fputs("\n", file);
        printf("1\n");
        fputs(vars[i], file);
    }
    fputs("\n", file);
    fputs(res_vname, file);

    for(int i = 1; i<= n; i++)
    {
        fputs("\n", file);

        if(code[i]== NULL)
            break;
        line = malloc(128* sizeof(char));
        sprintf(line, "%d, %s", i, code[i]-> line);
        fputs(line, file);
    }
    fclose(file);
}


/* displays the lines in the code*/
void display_code(code_line** code, int line_nums)
{
    for(int i = 1; i<= line_nums; i++)
    {
        printf("\n... %d, %s", i, code[i]->line);
    }

    return;
}


/* prcesses macro_line
Inputs:
    i -  curr line number
    cline - the function stores info about the line in this pointer
    macro_name - name of the macro_name refferring to the 
    remaining_str - line without the macro_name
    all_vars - set of all variables in the current program
    is_file - true iff the processing is to be done from a file, false if using stdin to make new program
    in - file ptr, if is_file

returns 
    -1 if error
    i++ otherwise
*/

int process_macro_line(int i, code_line* cline, char* macros_name, char* remaining_str, prog_vars_set* all_vars, bool is_file, FILE* in)
{
    all_macros *macro;
    prog_vars_set* var;
    prog_vars_refs* prefs = NULL;
    char* variable_vals = malloc(15*sizeof(char));
    HASH_FIND_STR(defined_progs, macros_name, macro);
    int new_import;
    if(macro == NULL)
    {
        printf("\t%s\n", macros_name);
        err_print("^ No such macro found\n");
        printf("Attempting import\n");

        new_import = process_file_macros(macros_name);
        if(new_import==-1)
        {   printf("OOPS\n");
            return -1;
        }
        printf("Import Successful\n");
        HASH_FIND_STR(defined_progs, macros_name, macro);
    }

    char* display =  malloc(128*sizeof(char));
    
    if(!is_file )
    {
        printf("%s\n", DISPLAY_MACRO_MSG);
        scanf("%s", display);
        if(display[0]=='y')
            display_code(macro->code, macro->line_nums);

        for(var = macro->all_vars; var != NULL; ) 
        {
            variable_vals = malloc(15*sizeof(char));
            if(strncmp(var -> var_name, "z", 1))
            {
                var = var-> hh.next;
                continue;
            }

            printf("%s:  ", var-> var_name);
            fscanf(in, "%s ", variable_vals);
            prog_vars_set* var;

            HASH_FIND_STR(all_vars, variable_vals, var);

            if(!var)
            {
                printf("No variable named %s defined\n", variable_vals);
                return -1;
            }
            else
            {
                prog_vars_refs* to_add =  malloc(sizeof(prog_vars_refs));
                to_add-> var_name = var-> var_name;
                to_add-> ref = variable_vals;
                HASH_ADD_STR(prefs, var_name, to_add);
                var = var->hh.next;

            }

            
        }

    }
    else
    {
        int c = 0;
        int start = 0;
        int length = 0;
        for(var = macro->all_vars; var != NULL; ) 
        {
            variable_vals = malloc(15*sizeof(char));
            if(!strncmp(var -> var_name, "z", 2))
            {
                var = var-> hh.next;
                continue;
            }

            while(remaining_str[c]==' ')
            {
                c++;
            }

            start = c;
            length = 0;

            while(remaining_str[c]!= '\0' && remaining_str[c]!=' ' && remaining_str[c]!='\n')
            {
                length++;
                c++;
            }

            substr(variable_vals, remaining_str, start, length);
            prog_vars_set* found_var;

            HASH_FIND_STR(all_vars, variable_vals, found_var);

            if(found_var == NULL)
            {
                printf("No variable named %s defined\n", variable_vals);
                return -1;
            }
            else
            {

                prog_vars_refs* to_add =  malloc(sizeof(prog_vars_refs));
                to_add-> var_name = var -> var_name;
                to_add-> ref = variable_vals;
                HASH_ADD_STR(prefs, var_name, to_add);
                var = var->hh.next;

            }

            
        }
    }


    cline -> linet = MACRO;
    cline -> pv = malloc(sizeof(union parsed_vals));
    cline -> pv -> macro = malloc(sizeof(macro_line));
    cline -> pv -> macro -> macro_name =  macros_name;
    cline -> pv -> macro -> vrefs =  prefs;

    return i+1;
}


/*
Processes line containing variables
*/
int process_var_line(int i, code_line* cline, char* var1_name, char* remaining_str, prog_vars_set* all_vars)
{
    bool check_var;
    check_var  = check_valid_var(var1_name, all_vars);
    if(!check_var)
    {
        return i;
    }

    first_key* other_var =  extract_first_key(remaining_str);
    if(!other_var->intended_var)
    {
        printf("\t%d: %s\n", i, cline-> line);
        err_print(EXPECTED_FORMAT);
        return i;
    }
    char* var2_name = other_var -> val;
    check_var  = check_valid_var(var2_name, all_vars);

    if(!check_var)
    {
        return i;
    }

    int curr_c = other_var -> c + 1 ;
    extract_num* last_val = extract_num_from_string(remaining_str+curr_c);
    if(!last_val->is_number)
    {
        printf("OOPS! %s\n", EXPECTED_FORMAT);
        return i;
    }
    cline -> linet = VAR;
    cline -> pv = malloc(sizeof(union parsed_vals));
    cline -> pv -> var = malloc(sizeof(var_line));
    cline -> pv -> var -> vars[0] =  var1_name;
    cline -> pv -> var -> vars[1] =  var2_name;
    cline -> pv -> var -> i = last_val->number;

    return i+1;
}


// eval the curr line : var_line/ macro_line
// identifies which type of line is it and calls appropriate function
int eval(int i, char* cmdline, code_line* cline, prog_vars_set* all_vars, bool is_file, FILE* in)
{
    bool var;
    int c;
    char* dst;
    if(cmdline[0] == '\0')
    {
        return i;
    }
    first_key*  res = extract_first_key(cmdline);
    c =  res -> c;
    var = res -> intended_var;
    dst =  res -> val;
    c++;
    while(cmdline[c]!='\0' && cmdline[c]==' ')   
    {
        c++;
    }

    if(var)
    {
        return process_var_line(i, cline, dst, cmdline+c, all_vars);
    }
    return process_macro_line(i, cline, dst, cmdline+c, all_vars, is_file, in);
}

/************************************************************************************************
 *                                      Functions for running code                               *
*************************************************************************************************/


// returns a copied dictionary of prog_vars_vals
prog_vars_vals* var_dict_copy(prog_vars_vals* to_copy)
{
    prog_vars_vals* res =  NULL;
    prog_vars_vals* vars;
    prog_vars_vals* to_add;
    for(vars = to_copy; vars != NULL; vars = vars->hh.next) 
    {
        to_add = malloc(sizeof(prog_vars_vals));
        to_add -> var_name = malloc(16*sizeof(char));
        strcpy(to_add -> var_name , vars -> var_name);
        to_add -> value = vars-> value;
        HASH_ADD_STR(res, var_name, to_add);
    }

    return res;
}
// displats to_display set pf prog_vars_vals
prog_vars_vals* var_dict_display(prog_vars_vals* to_display)
{
    prog_vars_vals* res =  NULL;
    prog_vars_vals* vars;
    for(vars = to_display; vars != NULL; vars = vars->hh.next) 
    {
        printf("%s = %d\n", vars -> var_name , vars -> value);
    }

    return res;
}

/* checks if the values of atleast one variable are different, to use for infinite loops
If false, when comparing the previous vals and curr vals at the same time -> infinite loop
*/
bool is_diff(prog_vars_vals* prev_var_vals, prog_vars_vals* curr_var_vals)
{
    if(prev_var_vals == NULL)
    {
        return true;
    }

    prog_vars_vals* var_v;

    for(prog_vars_vals* vars = curr_var_vals; vars != NULL; vars = vars->hh.next) 
    {

        HASH_FIND_STR(prev_var_vals, vars -> var_name, var_v);
        if(vars-> value != var_v-> value)
        {
            return true;
        }
        
    }

    return false;
}

/* executes code
Inputs : 
    - init_var_vals : dictionary of var_names : values as input by the user
    - macro : program to run
    - show : true to show the values of all variables at the end

returns the value of the result variable
*/
int execute_code(prog_vars_vals* init_var_vals, all_macros* macro, bool show)
{
    prog_vars_vals* var1;
    prog_vars_vals* var2;
    prog_vars_vals* nvar1;
    prog_vars_vals* res;
    char* var1_name;
    char* var2_name;
    char* macro_name;
    code_line** code;
    code_line* curr_cline; 
    prog_vars_vals** vars_line;
    prog_vars_refs*  vrefs;
    prog_vars_vals* macro_var_vals;
    bool check_fin;
    prog_vars_vals* curr_var_vals;
    int n;
    curr_var_vals = init_var_vals;
    n = macro -> line_nums;
    vars_line = calloc(n+1, sizeof(prog_vars_vals*));
    int i = 1;
    code = macro -> code;
    prog_vars_vals* to_add_m;
    prog_vars_vals* var_v;
    all_macros* macro_proc;
    int line_res;
    prog_vars_vals* z_var_new;
    prog_vars_refs* found_ref;

    while(i <= n)
    {
        curr_cline = code[i];

        switch(curr_cline->linet)
        {
            case VAR:
                var1_name = curr_cline -> pv -> var -> vars[0];
                var2_name = curr_cline -> pv -> var -> vars[1];
                HASH_FIND_STR(curr_var_vals, var1_name, var1);
                if(var1==NULL)
                {
                    printf("%s\n", var1_name);
                }
                HASH_FIND_STR(curr_var_vals, var2_name, var2);
                if(var2==NULL)
                {
                    printf("%s\n", var2_name);
                }
                nvar1 =  malloc(sizeof(prog_vars_vals));
                nvar1 -> var_name = var1_name;
                nvar1 -> value =  var1-> value - var2 -> value;
                HASH_DEL(curr_var_vals, var1);
                curr_var_vals = var_dict_copy(curr_var_vals);
                HASH_ADD_STR(curr_var_vals, var_name, nvar1);
                check_fin = is_diff(vars_line[i], curr_var_vals);

                if(!check_fin)
                {
                    printf("Infinite loop detected\n");
                    return 0;
                }
                vars_line[i] = var_dict_copy(curr_var_vals);
                if(nvar1 -> value == 0)
                {
                    i = curr_cline -> pv-> var -> i; 
                }
                else 
                {
                    i++;
                }

                break;

            case MACRO:
                nvar1 =  malloc(sizeof(prog_vars_vals));
                macro_name = curr_cline -> pv -> macro -> macro_name;
                vrefs =  curr_cline -> pv -> macro -> vrefs;
                macro_var_vals = NULL;
                z_var_new = malloc(sizeof(prog_vars_vals));
                z_var_new -> var_name = "z";
                z_var_new -> value =  1;
                HASH_ADD_STR(macro_var_vals, var_name, z_var_new);
                for(prog_vars_refs* vars = vrefs; vars != NULL; vars = vars->hh.next) 
                {
                    HASH_FIND_STR(curr_var_vals, vars -> ref, var_v);
                    to_add_m = malloc(sizeof(prog_vars_vals));
                    to_add_m -> value = var_v-> value;
                    to_add_m -> var_name = vars-> var_name;
                    HASH_ADD_STR(macro_var_vals, var_name, to_add_m);
                }
                HASH_FIND_STR(defined_progs, macro_name, macro_proc);
                line_res = execute_code(macro_var_vals, macro_proc, false);
                if(line_res==-1)
                {
                    return -1;
                }

                curr_var_vals = var_dict_copy(curr_var_vals);
                HASH_FIND_STR(vrefs, macro_proc-> res_var, found_ref);
                HASH_FIND_STR(curr_var_vals, found_ref-> ref, var1);
                if(var1==NULL)
                    printf("AHHHHHH\n");
                HASH_DEL(curr_var_vals, var1);
                nvar1 =  malloc(sizeof(prog_vars_vals));
                nvar1 -> var_name = found_ref-> ref;
                nvar1 -> value = line_res;
                HASH_ADD_STR(curr_var_vals, var_name, nvar1);
                vars_line[i] = var_dict_copy(curr_var_vals);
                i++;
                break;

            default:
                printf("THIS SHOULDN'T HAPPEN\n");
                break;
        }

    }

    HASH_FIND_STR(curr_var_vals, macro->res_var, res);

    if(show)
    {
        var_dict_display(curr_var_vals);
    }
    return res-> value;

}


/* executes the run command
 - asks for the name of the macro to run
 - asks for the start values of different variables
 returns final value of the result variable

*/

int run(char* cmdline)
{
    char* macro_name = malloc(15*sizeof(char));
    all_macros* macro;
    prog_vars_set* vars;
    prog_vars_vals* var_vals = NULL;
    prog_vars_vals** all_var_vals;


    printf(">>> Enter name of the macro you want to run: ");
    scanf("%s", macro_name);

    HASH_FIND_STR(defined_progs, macro_name, macro);
    if(macro == NULL)
    {
        printf("Macro %s not found\n", macro_name);
        return -1;
    }
    int n = macro -> line_nums;
    all_var_vals =  malloc((n+1)* sizeof(prog_vars_vals*));
    char* display = malloc(128*sizeof(char));
    printf("\n%s", DISPLAY_MACRO_MSG);
    scanf("%s", display);
    char* m_display  = extract_first_word(display);
    m_display[1] = '\0';
    if(m_display[0]=='y')
    {
        display_code(macro->code, n);
    }

    printf("\nEnter variables\n");
    prog_vars_vals* z_var = malloc(sizeof(prog_vars_vals));
    z_var -> var_name = "z";
    z_var -> value =  1;
    HASH_ADD_STR(var_vals, var_name, z_var);
    for(vars = macro->all_vars; vars != NULL; ) 
    {
        if(!strcmp(vars-> var_name, "z"))
        {
            vars = vars->hh.next;
            continue;
        }
        prog_vars_vals* new_var = malloc(sizeof(prog_vars_vals));
        new_var -> var_name = vars -> var_name;
        printf("%s: ", vars->var_name);
        new_var-> var_name = vars -> var_name;
        char* val = malloc(129*sizeof(char));
        scanf("%s", val);
        extract_num* e= extract_num_from_string(val);
        if(! e -> is_number)
        {
            err_print(INVALID_NUMBER_ERR);
            free(val);
            free(new_var);
            continue;
        }

        new_var->value =  e -> number;
        free(val);
        HASH_ADD_STR(var_vals, var_name, new_var);
        vars = vars->hh.next;
    }
    return execute_code(var_vals, macro, true);
}

/************************************************************************************************
 *                                     Processes macro defined in a file                        *
*************************************************************************************************/


int process_file_macros(char* file_name)
{
    char* command ;
    int size; 
    char* prog_name = malloc(16*sizeof(char));
    FILE* file = fopen(file_name, "r");
    if(file == NULL)
    {
        err_print("^ No such macro found\n");
        return -1;
    }
    prog_vars_set* res_vars = malloc(sizeof(prog_vars_set));
    strcpy(prog_name, file_name);
    
    char* line = malloc(128*sizeof(char));
    strcpy(line, "//");

    while(line[0] == '\0' || line[0] == '\n' || !strncmp(line, "//", 2))
    {
        line = malloc(128*sizeof(char));
        if(fgets(line, 128, file )==NULL)
        {
            err_print("\nPlease enter size\n");
            return -1;
        }
    }

    char* fsize = extract_first_word(line);
    extract_num* esize = extract_num_from_string(fsize);
    if(! esize -> is_number)
    {
        err_print(INVALID_NUMBER_ERR);
        free(esize);
        return -1;
    }
    size = esize-> number;

    code_line** code = malloc((size+1)*sizeof(code_line*));
    line = "//";
    while(line[0] == '\0' || line[0] == '\n' || !strncmp(line, "//", 2))
    {
        line = malloc(128*sizeof(char));
        if(fgets(line, 128, file )==NULL)
        {
            err_print("\nPlease enter num_vars\n");
            return -1;
        }
    }

    char* fnum_inputs = extract_first_word(line);
    extract_num* enum_inputs = extract_num_from_string(fnum_inputs);
    if(! enum_inputs -> is_number)
    {
        err_print(INVALID_NUMBER_ERR);
        free(enum_inputs);
        return -1;
    }


    int num_vars = enum_inputs-> number;
    prog_vars_set* all_vars = NULL;
    bool is_valid = true;
    char* temp;
    prog_vars_set* to_add;
    char* var;
    prog_vars_set* z_var;
    temp = malloc(16 * sizeof(char)) ;
    char* res_vname = malloc(16 * sizeof(char)) ;

    strcpy(temp, "z");
    z_var = malloc(sizeof(prog_vars_set));
    z_var -> var_name =  temp;
    HASH_ADD_STR(all_vars, var_name, z_var);

    for(int i = 0; i<num_vars; i++)
    {
        to_add = malloc(sizeof(prog_vars_set));
        var = "";
        while(var[0] == '\0' || var[0] == '\n' || !strncmp(var, "//", 2))
        {
            var = malloc(128*sizeof(char));
            if(fgets(var, 128, file )==NULL)
            {
                err_print("\nPlease enter num_vars\n");
                return -1;
            }
        }

        var = extract_first_word(var);
        if(var==NULL)
        {
            err_print("\nNot enough variables\n");
            return -1;
        }

        is_valid = check_vname_valid(var, all_vars);
        if(! is_valid)
        {
            printf("%s\n", var);
            free(var);
            return -1;
        }
        to_add->var_name = var;
        HASH_ADD_STR(all_vars, var_name, to_add);
    }

    strcpy(res_vname, "//");
    while(res_vname[0] == '\0' || res_vname[0] == '\n' || !strncmp(res_vname, "//", 2))
    {
        free(res_vname);
        res_vname = malloc(128*sizeof(char));
        if(fgets(res_vname, 128, file )==NULL)
        {
            err_print("\nPlease enter num_vars\n");
            return -1;
        }
    }

    if(res_vname ==  NULL)
    {
        err_print("\nPlease enter result variable\n");
        return -1;
    }
    res_vname = extract_first_word(res_vname);
    is_valid = check_vname_valid(res_vname, all_vars);
    if(! is_valid)
    {
        printf("%s\n", res_vname);
        free(res_vname);
        err_print("^ Please check variable nomenclature");
        return -1;
    }    

    res_vars = malloc(sizeof(prog_vars_set));
    res_vars -> var_name = res_vname;
    HASH_ADD_STR( all_vars, var_name, res_vars );
    int i = 1;
    int new;
    char* nline;
    char* indicated_num = malloc(128* sizeof(char)); // optional
    extract_num* e;
    int i_num;
    int c;
    while(i<=size)
    {
        c = 0;
        nline =  "//";
        code[i] = malloc(sizeof(code_line));
        while(nline[0] == '\0' || nline[0] == '\n' ||  !strncmp(nline, "//", 2))
        {
            nline = malloc(128*sizeof(char));
            if(fgets(nline, 128, file )==NULL)
            {
                err_print("\nPlease enter number of lines - comments = num_lines\n");
                return -1;
            }
        }

        while(nline[c]!= '\0' && nline[c] != '\n' && nline[c]!= ',' && nline[c]!= ' ')
        {
            indicated_num[c] = nline[c];
            c++;
        }
        indicated_num[c] = '\0';
        while(nline[c]!=  '\0' && nline[c] == ' '  && nline[c] != '\n')
            c++;

        if(nline[c] == ',')
        {  
            e = extract_num_from_string(indicated_num);
            if(! e -> is_number)
            {
                printf("%s\n",nline );
                printf("%s\n", INVALID_NUMBER_ERR);
                free(e);
                return -1;
            }
            i_num = e-> number;
            if(i_num != i)
            {
                printf("\t%s\n",nline );
                err_print("^ First number should be the line number\n");
                return -1;
            }

        }

        else
        {
            printf("\t%s\n", nline);
            err_print("^ Please enter line number");
            return -1;
        }
        c++;
        while(nline[c]==' ')
            c++;
        int t = c;
        int line_l = 0;
        while(nline[t]!= '\0' && nline[t] != '\n')
        {
            
            if(!strncmp(nline+t, "//", 2))
                break;
            t++;
            line_l++; 
        }
        command = malloc(128*sizeof(char));
        substr(command, nline, c, line_l) ;
        code[i] -> line = command;
        new = eval(i, command, code[i], all_vars, true, file);
        if(new==-1)
        {
            printf("Please check the format of this line\n %s\n", code[i]-> line );
            free(nline);
            free(code[i]);
            return -1;
        }

        if(new==i)
        {
            free(code[i]-> line);
            free(code[i]);           
        }
        i = new;

    }
    if(i!= size+1)
    {
        err_print("Please enter correct number of lines");
        return -1;
    }

    all_macros *macro = malloc(sizeof(all_macros));
    macro-> prog_name = prog_name;
    macro-> code = code;
    macro-> line_nums = size;
    macro-> all_vars = all_vars;
    macro-> res_var = res_vname;
    HASH_ADD_STR( defined_progs, prog_name, macro );
    return 0;
}

/************************************************************************************************
 *                                    Interpreter mode                                          *
*************************************************************************************************/


void interpreter_mode()
//TODO: safe string compare
{

    char* input =  malloc(128*sizeof(char));
    while(true)
    {
        printf("%s", PROMPT);
        scanf(" %[^\n]", input);        

        if(!strncmp(input, "new", 3))
        {
            new_program();
        }

        else if(!strncmp(input, "run", 3))
        {
            int res = run(input+3);
            printf("RESULT : %d\n", res);
        }
        
        else if(!strncmp(input, "import", 6))
        {
            process_file_macros(input+7);

        }
        else if(!strncmp(input, "exit", 4))
        {
            free(input);
            printf("Exiting interpreter_mode\n");
            return;
        }
        else
        {
            printf("%s\n", WRONG_I_OPT_MSG);
        }

    }

    
}

/************************************************************************************************
 *                                     Main function                                            *
*************************************************************************************************/


void help()
{
    printf("%s\n", ARG_H);
    printf("%s\n", ARG_N);
    printf("%s\n", ARG_I);
    printf("%s\n", ARG_F);
}

int main(int argc, char **argv)
{
    int opt;
	while ((opt = getopt(argc, argv, "hi")) != -1) 
	{
        switch (opt) 
        {
        	case 'h': 
        	    help();
        		exit(0);
                break;

        	case 'i': 
        		interpreter_mode();
        		break;

            default:
                printf("No argument %c\n",  opt);
                exit(-1);

        }
    }
}