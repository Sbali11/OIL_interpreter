// Defining Types

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>

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



typedef struct lines_referred_by_t
{
    int i;                      /* key: line number that refers to the curr_line */
    UT_hash_handle hh;         /* makes this structure hashable */
} lines_referred_by;
