#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>

const char* ARG_H = "-h: help";
const char* ARG_N = "-n <num_lines>: number of lines in the program";
const char* ARG_I = "-i: Interpreter Mode";
const char* ARG_F = "-f(OPTIONAL): File containing name of macros";

const char** ARGS_HELP = {ARG_H, ARG_N, ARG_I, ARG_F}