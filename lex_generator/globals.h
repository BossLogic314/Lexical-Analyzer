#ifndef _GLOBALS_

#define _GLOBALS_

#include "nfa.h"
#include "dfa.h"

#ifdef _GLOBALS_DEF_
#define PREFIX
#else
#define PREFIX extern
#endif

#define STACK_SIZE 1024
#define MAX_COUNT 1000
#define MAX_CHARS 128

PREFIX char input_buffer[MAX_COUNT];
PREFIX char* input_buffer_ref;

PREFIX int lexeme;
PREFIX int current_token;

PREFIX char* stack[STACK_SIZE];

// The variable should be initialized without extern
#ifdef _GLOBALS_DEF_
PREFIX char** stack_top = stack - 1;
#else
PREFIX char** stack_top;
#endif

PREFIX NFA_DATA nfa_data;

PREFIX DFA_DATA dfa_data;

PREFIX int* transition_table[MAX_DFAS];

// Input and output file pointers
PREFIX FILE* input_fp;
PREFIX FILE* output_fp;

#endif