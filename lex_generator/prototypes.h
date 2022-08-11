#ifndef _PROTOTYPES_

#define _PROTOTYPES_

#include "nfa.h"
#include "dfa.h"
#include "lex.h"

// lex.c
char* get_macro_text(char* macro_name);
void getline();
void print_error_with_line_number();
void print_error_message(char* message);
char* strip_comments(char* string, int* p_in_comment);
void head_parsing();
void macro_processing();
void print_macros(MACRO* pmacro);
void write_switch_statement_to_c_file();
void write_transition_table_to_c_file();
void lex();

// nfa.c
NFA* new_NFA();
void discard_NFA(NFA* pnfa);
char get_special_lexeme();
void advance();

NFA* machine();
NFA* rule();
void expr(NFA** ppstart, NFA** ppend);
void cat_expr(NFA** ppstart, NFA** ppend);
void factor(NFA** ppstart, NFA** ppend);
void term(NFA** ppstart, NFA** ppend);
bool first_in_cat();
void dodash(SET* pset);

void assign_indices();
void print_nfas();
void free_all_nfas();

// interpret.c
void e_closure(SET* p_input_set, char** paccept, int* panchor);
SET* move(SET* p_input_set, int c);

// dfa.c
DFA* new_DFA();
void add_to_dfa_states(SET* pset, char* accept, int anchor);
DFA* get_unmarked();
int is_set_present(SET* pset);
void free_all_dfa_sets();
void free_all_dfas();
void free_transition_table();
void print_transition_table();
void print_dfas();
void make_transition(int start_state);

#endif