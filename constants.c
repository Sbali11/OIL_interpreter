#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include "uthash.h"


const char* PROMPT = "OIL> ";
const char* ARG_H = "-h: help";
const char* ARG_N = "-n <num_lines>: number of lines in the program";
const char* ARG_I = "-i: Interpreter Mode";
const char* ARG_F = "-f(OPTIONAL): File containing name of macros";
const char* WELCOME_MSG = "*****************************************\t\t Welcome to  OIL interpreter_mode\t\t *****************************************";
const char* I_NEW = "To start a new program, type new";
const char* I_RUN = "To run a program, type run <prog_name>, you will be asked to give values for variables if needed. z is automatically initialized to 1.";
const char* I_EXIT = "To exit the interpreter, type exit";
const char* P_VAR = "To enter variables that you will be using: <var_name> <start_val>";
const char* P_STAR = "To see all the commands that refer to the current line number, enter *";
const char* P_SIZE= "To change the length of the program, change size/-";
const char* P_CHANGE= "To change a line previously defined, type change <line_number>";
const char* HALTING_MSG = "HALTING... Do you want to save this program in your directory(y/n)?";
const char* LINE_CHANGE_MSG = ">>> Enter line to be changed(write -1 to exit line change): ";
const char* INVALID_LINE_MSG = "Invalid line number";
const char* NEW_LINE_INPUT_MSG = ">>> Enter new line";
const char* LINE_CHANGE_SUCCESS_MSG = "Line successfully changed!";
const char* P_SIZE_MSG = ">>> Enter the length of the program: ";
const char* NUM_VARS_MSG = ">>> Enter num of input variables: ";
const char* ERROR_NUM_VARS = "Number of vars can't be <0";
const char* P_NAME_MSG = ">>> Enter name of your program(<128 chars): ";
const char* P_VAR_NAMES = ">>> Enter names of input variables: ";
const char* DISPLAY_MACRO_MSG = "Do you want to display the code for macro(y for yes)?";
const char* WRONG_I_OPT_MSG = "run/new are the only valid options";
const char* NO_VAR_ENTERED = "Please enter var name";
const char* VNAME_START_ERR = "Please start var_name with a character";
const char* VNAME_MACRO_ERR = "Variable names cannot be macro-names";
const char* VAR_NAME_EXTS_ERR = "Variable name already exists, please choose another name";
const char* EXPECTED_FORMAT = "Expected format: <var1_name>, <var2_name>, <line_no>";
const char* CHANGE_EXPECTED_FORMAT = "Expected format: change <line_number>";
const char* INVALID_NUMBER_ERR = "\t> Please enter valid natural number";
