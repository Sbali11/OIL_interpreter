#include <stdio.h>
#include <stdlib.h>
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
*/
// Defining Types
typedef enum 
{
    EXIT,
    NEW,
    RESTART,
    REF_LINE,
    TO_EVAL,
    COMMENT,
    CHANGE,
    CHANGE_SIZE,
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



// DEFINING ALL STRUCTS
typedef struct prog_vars_t 
{
    char* var_name;                    /* key */
    UT_hash_handle hh;         /* makes this structure hashable */

} prog_vars_set;


typedef struct prog_vars_refs_t
{
    char* var_name;                    /* key */
    char* ref;
    UT_hash_handle hh;         /* makes this structure hashable */

} prog_vars_refs;

typedef struct var_line_t
{
    char* vars[2];
    int i;

} var_line;

typedef struct macro_line_t
{
    char* macro_name;
    prog_vars_refs*  vrefs; // DICT
    //TODO
} macro_line;

union parsed_vals
{

    var_line* var;
    macro_line* macro;
};

typedef struct code_t
{
    char* line;
    cmdline_type linet;
    union parsed_vals* pv;

} code_line;



typedef struct macros_t 
{
    char* prog_name;                    /* key */
    int line_nums;
    code_line** code;
    prog_vars_set* all_vars;
    char* res_var;
    UT_hash_handle hh;         /* makes this structure hashable */

} all_macros;

typedef struct first_key_t
{
    char* val;
    bool intended_var;
    int c;

} first_key;

typedef struct extract_number_t
{
    bool is_number;
    int number;

} extract_num;


typedef struct prog_vars_vals_t
{
    char* var_name;                    /* key */
    int value;
    UT_hash_handle hh;         /* makes this structure hashable */

} prog_vars_vals;



typedef struct line_res_t
{
    int i;
    prog_vars_vals* vars_curr_val;
    
} line_res;


//Defining constants
all_macros* defined_progs = NULL;

// GLOBAL variables

int n = -1;

void interpreter_mode();
void new_program();
int eval(int i, char* cmdline, code_line* code_line, prog_vars_set* all_vars);

char* set_program_name()
{
    char* name = malloc(20* sizeof(char));
    printf("%s", P_NAME_MSG);
    scanf("%s", name);
    return name;
}

bool check_char(char c)
{
    if(c>='a' && c<='z')
        return true;
    if(c>='A' && c<='Z')
        return true;
    return false;

}

// https://stackoverflow.com/questions/19482941/isdigit-to-include-checking-spaces
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
        printf("^ %s\n", VNAME_START_ERR);
        printf("\033[0m");
        return false;
    }

    int c = 1;
    char err_msg[strlen(var_name)];
    strcpy(err_msg, " ");

    while(var_name[c]!= '\0')
    {
        if(!(check_char(var_name[c]) || isdigit(var_name[c])))
        {
            printf("\033[1;31m");
            printf("\nVAR_NAME_ERROR:");
            printf("\033[0m");
            printf("\t%s\n", var_name);

            printf("\t%s^ Variable names can only have letters/numbers \n", err_msg);


            return false;
        }
        
        strcat(err_msg, " ");
        c++;
    }

    prog_vars_set* var;

    HASH_FIND_STR(all_vars, var_name, var);

    if(var != NULL)
    {
        printf("\033[1;31m");
        printf("%s\n", VAR_NAME_EXTS_ERR);
        printf("\033[0m");
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

prog_vars_set* create_vars()
{

    prog_vars_set* all_vars = NULL;
    int num_vars = 0;
    bool is_valid = true;
    char* temp;
    prog_vars_set* to_add;
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

    printf("%s\n", P_VAR_NAMES);
    
    for(int i = 0; i<num_vars; i++)
    {
        //Size of vars
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

parse_type exec(char *cmdline)
{
    //CHECK IF UPPER ARROW KEY

    if(!strncmp(cmdline, "exit", 4))
        return EXIT;

    if(!strncmp(cmdline, "change", 6))
        return CHANGE;


    else if (!strncmp(cmdline, "new", 3))
        return RESTART;

    else if (!strncmp(cmdline, "*", 1))
        return REF_LINE;

    else if(!strncmp(cmdline, "//", 2))
        return COMMENT;

    return TO_EVAL;
}




void free_code(code_line** code, int n)
{  
    int i= 0;
    var_line* var;
    macro_line* macro;
    prog_vars_refs* vref;
    prog_vars_refs* tmp;

    while(i < n && code[i]!= NULL)
    {
        if(code[i]-> line != NULL)
            free(code[i] -> line);

        switch(code[i]-> linet)
        {
            case VAR:
                var = code[i] -> pv -> var;
                if(var!=NULL)
                {
                    free(var -> vars[0]);
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
                        free(vref);            /* optional- if you want to free  */
                    } 

                }
                break;

            default:

                break;
        }
        i++;

    }
}

void process_program(int n, prog_vars_set* all_vars, char* prog_name, char* res_vname)
{
    int i = 0;
    code_line** code = malloc(n*sizeof(code_line*));
    int line_no;
    
    extract_num* line_no_ext;
    //char* line;
    printf(">>> Start Program %s\n", prog_name);
    int new;
    int new_l;

    code_line* new_code_line;

    while(i<n)
    {
        code[i] = malloc(sizeof(code_line));
        code[i] -> line = malloc(129* sizeof(char));
        char save_opt = 'k';
        printf("... %d, ", i);
        scanf(" %[^\n]", code[i] -> line);
        switch(exec(code[i] -> line))
        {
            case EXIT:
                printf("EXIT\n");
                free_code(code, n);
                exit(0);

            case TO_EVAL:
                //TODO 
                new = eval(i, code[i] -> line, code[i], all_vars);
                if(new==i)
                {
                    free(code[i]-> line);
                    free(code[i]);
                }

                i = new;
                break;
            
            case RESTART: 
                printf("RESTART\n");
                while(save_opt != 'y' || save_opt != 'n')
                {
                    printf("%s\n", HALTING_MSG);
                    scanf("%c", &save_opt);
                    if(save_opt == 'y')
                        printf("To Save");
                        //save_program(prog_name, code);
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

                new_l = eval(line_no, new_code_line-> line, new_code_line, all_vars);
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
            
                printf("WUT");
                break;

            default:
                printf("NOT YET IMPLEMENTED");
                break;

        }
    }
    all_macros *var = malloc(sizeof(all_macros));
    var-> prog_name = prog_name;
    var-> code = code;
    var-> line_nums = n;
    var-> all_vars = all_vars;
    var-> res_var = res_vname;
    HASH_ADD_STR( defined_progs, prog_name, var );
    printf("\n");

}


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
        printf("Enter name of result variable: ");
        scanf("%s", res_vname);
        is_valid_vname = check_vname_valid(res_vname, all_vars);
    }
    res_vars -> var_name = res_vname;
    HASH_ADD_STR( all_vars, var_name, res_vars );
    process_program(size, all_vars, name, res_vname);
}


//TODO 

void save_program()
{
    //

}

char *substr(char *dst, char *src, size_t offset, size_t length) 
{
    memcpy(dst, src + offset, length);
    dst[length] = '\0';
    return dst;
}

void display_code(code_line** code, int line_nums)
{
    for(int i = 0; i< line_nums; i++)
    {
        printf("\n... %d, %s", i, code[i]->line);
    }

    return;
}



int process_macro_line(int i, code_line* cline, char* macros_name, char* remaining_str, prog_vars_set* all_vars)
{
    all_macros *macro;
    prog_vars_set* var;
    prog_vars_refs* prefs = NULL;
    char* variable_vals = malloc(15*sizeof(char));
    HASH_FIND_STR(defined_progs, macros_name, macro);
    if(macro == NULL)
    {
        printf("^\nNo macro named %s found\n", macros_name);
        return i;
    }

    char display;
    printf("%s\n", DISPLAY_MACRO_MSG);
    scanf("%c", &display);

    if(display=='y')
        display_code(macro->code, macro->line_nums);

    for(var = macro->all_vars; var != NULL; ) 
    {
        printf("%s:  ", var-> var_name);
        scanf("%s", variable_vals);
        prog_vars_set* var;

        HASH_FIND_STR(all_vars, variable_vals, var);

        if(!var)
            printf("No variable named %s defined\n", variable_vals);
        else
        {
            prog_vars_refs* to_add =  malloc(sizeof(prog_vars_refs));
            to_add-> var_name = var-> var_name;
            to_add-> ref = variable_vals;
            var = var->hh.next;

        }

            
    }

    cline -> linet = MACRO;
    cline -> pv = malloc(sizeof(union parsed_vals));
    cline -> pv -> macro = malloc(sizeof(macro_line));
    cline -> pv -> macro -> macro_name =  macros_name;
    cline -> pv -> macro -> vrefs =  prefs;

    return i+1;


}

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


    res -> intended_var = var;
    res-> c = c;
    return res;

}



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
        printf("OOPS! %s\n", EXPECTED_FORMAT);
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



int eval(int i, char* cmdline, code_line* cline, prog_vars_set* all_vars)
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

    return process_macro_line(i, cline, dst, cmdline+1, all_vars);

}

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

int execute_code(prog_vars_vals* init_var_vals, all_macros* macro)
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
    vars_line = calloc(n, sizeof(prog_vars_vals*));
    int i = 0;
    code = macro -> code;
    prog_vars_vals* to_add_m;
    prog_vars_vals* var_v;
    
    while(i < n)
    {
        curr_cline = code[i];

        switch(curr_cline->linet)
        {
            case VAR:
                var1_name = curr_cline -> pv -> var -> vars[0];
                var2_name = curr_cline -> pv -> var -> vars[1];
                HASH_FIND_STR(curr_var_vals, var1_name, var1);
                HASH_FIND_STR(curr_var_vals, var2_name, var2);
                nvar1 =  malloc(sizeof(prog_vars_vals));
                nvar1 -> var_name = var1_name;
                nvar1 -> value =  var1-> value - var2 -> value;
                curr_var_vals = var_dict_copy(curr_var_vals);
                HASH_DEL(curr_var_vals, var1);
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
                macro_name = curr_cline -> pv -> macro -> macro_name;
                vrefs =  curr_cline -> pv -> macro -> vrefs;
                macro_var_vals = NULL;
                for(prog_vars_refs* vars = vrefs; vars != NULL; vars = vars->hh.next) 
                {
                    HASH_FIND_STR(curr_var_vals, vars -> ref, var_v);
                    to_add_m = malloc(sizeof(prog_vars_vals));
                    to_add_m -> value = var_v-> value;
                    to_add_m -> var_name = vars-> var_name;
                    HASH_ADD_STR(macro_var_vals, var_name, to_add_m);
                }

                i++;
                break;

            default:
                printf("THIS SHOULDN'T HAPPEN\n");
                break;
        }

    }

    HASH_FIND_STR(curr_var_vals, macro->res_var, res);
    if(res==NULL)
    {
        printf("WUT %s \n",macro->res_var);
    }
    return res-> value;

}

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
    all_var_vals =  malloc(n* sizeof(prog_vars_vals*));

    char* display = malloc(2*sizeof(char));
    printf("\n%s", DISPLAY_MACRO_MSG);
    scanf("%1s", display);
    display[1] = '\0';

    printf("Here: %c", display[0]);
    
    if(display[0]=='y')
    {
        display_code(macro->code, n);
    }

    printf("\nEnter variables\n");

    for(vars = macro->all_vars; vars != NULL; vars = vars->hh.next) 
    {
        prog_vars_vals* new_var = malloc(sizeof(prog_vars_vals));
        new_var -> var_name = vars -> var_name;
        printf("%s: ", vars->var_name);
        new_var-> var_name = vars -> var_name;
        char* val = malloc(129*sizeof(char));
        scanf("%s", val);
        extract_num* e= extract_num_from_string(val);
        if(! e -> is_number)
        {
            printf("%s\n", INVALID_NUMBER_ERR);
            free(val);
            continue;
        }

        new_var->value =  e -> number;
        free(val);
        HASH_ADD_STR(var_vals, var_name, new_var);
    }
    return execute_code(var_vals, macro);
}


void interpreter_mode()
//TODO: safe string compare
{

    char* input =  malloc(128*sizeof(char));
    int c;

    while(true)
    {
        printf("%s", PROMPT);
        scanf("%s",  input);
        c = 0;
        

        if(!strncmp(input+c, "new", 3))
        {
            new_program();
        }

        else if(!strncmp(input+c, "run", 3))
        {
            run(input+3);
        }

        else if(!strncmp(input+c, "exit", 4))
        {
            free(input);
            printf("Exiting interpreter_mode\n");
            //free_memory();
            return;
        }
        else
        {
            printf("%s\n", WRONG_I_OPT_MSG);
        }

    }

    
}

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

	while ((opt = getopt(argc, argv, "hn:if:")) != -1) 
	{
        switch (opt) 
        {
        	case 'h': 
        	    help();
        		exit(0);
                break;

            case 'n':
                n = atoi(optarg);
                break;

        	case 'i':
                if(n<0)
                {
                     help();
                     exit(-1);
                }      
        		interpreter_mode();
        		break;

            default:
                printf("No argument %c\n",  opt);
                exit(-1);

        }
    }


}

