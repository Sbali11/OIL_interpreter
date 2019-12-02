#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "uthash.h"
/*
MACROS cannot be numbers 
CHECK if variable is a macro

*/
// Defining Types
typedef enum 
{
    EXIT,
    NEW,
    REF_LINE,
    TO_EVAL,
    SAVE

} parse_type;


typedef enum 
{
    INFINITE_LOOP, 
    SYNTAX_ERROR,
    EVALUATED

} end_program;

typedef enum
{
    VAR,
    MACRO,
    UNDEF
} cmdline_type;

typedef struct my_struct 
{
    char* var_name;                    /* key */
    int value;
    UT_hash_handle hh;         /* makes this structure hashable */

} prog_vars;

typedef struct prog_vars_t
{
    long num_vars;
    char** vars;

} prog_vars;

typedef struct macros_t 
{
    char* macro_name;                    /* key */
    char** code;
    prog_vars* macro_vars;
    UT_hash_handle hh;         /* makes this structure hashable */

} all_macros;




//Defining constants
char* WELCOME_MESSAGE = "*****************************************\n\t\t Welcome to  OIL interpreter_mode\n*****************************************\n\n\n";
char* prompt = "OIL>";
all_macros defined_progs = NULL;

// GLOBAL variables
char** all_commands =  malloc(size*sizeof(char*));
int n = -1;


parse_type exec(const char *cmdline)
{
    //CHECK IF UPPER ARROW KEY

    if(strncmp(cmdline, "exit"))
        return EXIT;

    else if (strncmp(cmdline, "new"))
        return RESTART;

    else if (strncmp(cmdline, "*"))
        return REF_LINE;

    else if(strncmp(cmdline, "//"))
        return COMMENT

    return TO_EVAL;
}



prog_vars* init_all_vars()
{

    prog_vars** all_vars;
    long num_vars = 0;
    
    printf("%s\n", "Enter num of variables\n");
    scanf("%ld", num_vars);

    if(num_vars<0)
    {
        init_all_vars();
    }

    if(num_vars == 0)
        return all_vars;
    
    all_vars =  malloc(num_vars*sizeof(char*));

    for(int i = 0; i<num_vars; i++)
    {
        printf("%ld) ", i+1);
        scanf("%s", all_vars[i]);
        
    }

    all_vars->vars = all_vars;
    all_vars->num_vars = num_vars;

    return all_vars;


}
//TODO 

void save_program()
{
    return

}

void set_program_name()
{
    char name[128];
    printf("%s", "Enter name of your program(<128 chars): ");
    scanf("%s", name);
    return name;
}

void set_program_length()
{
    long size;
    printf("%s\n", "Enter the length of the program:" );
    scanf("%ld", size);
    return size;
}

char *substr(char *dst, char *src, size_t offset, size_t length) {
    memcpy(dst, src + offset, length);
    dst[length] = '\0';
    return dst;
}

void check_val(char* curr_str, char* remaining_str, prog_vars* all_vars)
{
    all_macros *macro;
    HASH_FIND_STR(defined_progs, remaining_str, macro);
    if(macro)
    {
        for(int i = 0; i<macro->macro_vars.num_vars; i++)
        {
            printf("%s:  ", macro->macro_vars.vars[i]);
        
        }

    }

}


long eval(long i, const char* cmdline, prog_vars* all_vars)
{
    int start = 0;
    size_t length = 0; 
    int c = 0;
    arg_num = 0;
    total_num_vars =  -1;
    cmdline_type curr_line_type = UNDEF;
    char* dst;
    
    while(cmdline[c]!='\0')
    {
        while(cmdline[c] != ' ')
            c++;

        if(cmdline[c]==',')
        {
            dst  =  malloc(sizeof(char)*(length+1));
            substr(dst, cmdline, 0, length);
            check_val(dst, cmdline[c+1], all_vars);
        } 
        c++;
        length++;

    }

}

end_program process_program(long n, prog_vars* all_vars, char* prog_name)
{
    long i;
    char** code = malloc(n*sizeof(char*));

    while(true)
    {
        printf("%s %ld, ", prompt, i);
        scanf("%s", line);

        switch(exec(line)):
        {
            case EXIT:
                exit(0);

            case TO_EVAL:
                i = eval(i, line, all_vars);
                break;
            
            case RESTART: 
                return;
                char save = 'k';
                while(save != 'y' || save != 'n')
                {
                    printf("%s\n", "HALTING... Do you want to save this program in your directory(y/n)?");
                    scanf("%c", save)
                    if(save=='y')
                        save_program(program_name, program);
                }

                all_macros *var = malloc(sizeof(all_macros));
                var-> prog_name = prog_name;
                var-> code = code;
                var-> all_vars = all_vars;
                HASH_ADD_STR( defined_progs, prog_name, var );

                new_program()
                break;

            case CHANGE:
                //Change line code

            
            case CHANGE_SIZE:
                printf("%s\n", "Enter the length of the program:" );
                scanf("%ld", size);
                break;

            case REF_LINE:
                print_rel_commands(i);
                break;

        }
    }
}

void new_program()
{

    int* all_vars = NULL;
    long max_var = -1;
    long size; 
    char name[128];
    name = set_program_name();
    size = set_program_length();
    prog_vars = init_all_vars();
    process_program(size, prog_vars, name);


}

void display_start_message()
{
    printf(WELCOME_MESSAGE);
    printf("%s\n", "To enter variables that you will be using: <var_name> <start_val>");
    printf("%s\n", "If you want to start a new program, type new");
    printf("%s\n", "If you want to exit the interpreter, type exit");
    printf("%s\n", "To see all the commands that refer to the current line number, enter *");
    printf("%s\n", "To change the length of the program, change size/-");
    printf("%s\n", "To change a line previously defined, type change <line_number>");
    return;
}

void interpreter_mode()
{
    display_start_message();
    new_program();
}

int main(int argc, char **argv)
{
    int opt;
    all_macros = "";


	while ((opt = getopt(argc, argv, "hn:if:")) != -1) 
	{
        switch (opt) 
        {
        	case 'h': 
        	//	help();
        		exit(0);

            case 'n':
                n = atoi(optarg);
                break;

            case 'l':
                n = 500;
                break;

        	case 'i':
                if(n<0)
                {
                     help();
                     exit(-1);
                } 
                           
        		interpreter_mode();
        		break;

        }
    }

}

