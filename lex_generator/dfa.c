#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfa.h"
#include "prototypes.h"
#include "globals.h"
#pragma warning (disable : 4996)

DFA* new_DFA() {

	DFA* pdfa = malloc(sizeof(DFA));

	// Check if there is enough memory
	if (!pdfa)
		print_error_message("Not enough memory to allocate for an NFA.\n");

	pdfa->accept[0] = '\0';
	pdfa->anchor = 0;
	pdfa->marked = false;
	pdfa->pset = NULL;

	return pdfa;
}

void add_to_dfa_states(SET* pset, char* accept, int anchor) {

	// Check on the number of DFAs
	if (dfa_data.num_of_dfas == MAX_DFAS)
		print_error_message("Too many number of DFA states to allocate.\n");

	DFA* pdfa = new_DFA();

	if (accept)
		strcpy(pdfa->accept, accept);

	pdfa->anchor = anchor;
	pdfa->pset = pset;

	dfa_data.list_of_DFAs[dfa_data.num_of_dfas++] = pdfa;
}

DFA* get_unmarked() {

	while (dfa_data.unmarked_index < dfa_data.num_of_dfas) {

		DFA* ref_dfa = dfa_data.list_of_DFAs[dfa_data.unmarked_index];

		if (!ref_dfa->marked)
			return ref_dfa;

		// If the dfa is marked
		dfa_data.unmarked_index++;
	}

	return NULL;
}

// Returns if a set is already present in the list of DFAs
int is_set_present(SET* pset) {

	int len = dfa_data.num_of_dfas;

	for (int i = 0; i < len; ++i) {

		DFA* dfa_ref = dfa_data.list_of_DFAs[i];
		SET* set_ref = dfa_ref->pset;

		if (IS_EQUIVALENT(pset, set_ref))
			return i;
	}

	return -1;
}

void free_all_dfa_sets() {

	int len = dfa_data.num_of_dfas;

	for (int i = 0; i < len; ++i) {

		DFA* ref_dfa = dfa_data.list_of_DFAs[i];

		delset(ref_dfa->pset);
	}
}

void free_all_dfas() {

	int len = dfa_data.num_of_dfas;

	for (int i = 0; i < len; ++i) {

		DFA* ref_dfa = dfa_data.list_of_DFAs[i];

		free(ref_dfa);
	}
}

void free_transition_table() {

	for (int i = 0; i < MAX_DFAS; ++i)
		free(transition_table[i]);
}

void print_transition_table() {

	printf("Transition Table -\n");
	int len = dfa_data.num_of_dfas;

	for (int i = 0; i < len; ++i) {

		printf("DFA state %d ->", i);

		int* row = transition_table[i];

		for (int j = 0; j < MAX_CHARS; ++j)
			printf(" %d", *(row + j));

		printf("\n\n");
	}
}

void print_dfas() {

	int len = dfa_data.num_of_dfas;

	printf("Number of states in the DFA is %d\n", len);

	next_member(NULL);

	for (int i = 0; i < len; ++i) {

		// Printing all the NFA states grouped together to form this DFA state
		printf("DFA state %d -> NFA states {", i);

		SET* pset = dfa_data.list_of_DFAs[i]->pset;

		int next_state = -1;
		while ((next_state = next_member(pset)) != -1)
			printf(" %d", next_state);

		printf(" }\n");
	}

	printf("\n");
	next_member(NULL);
}

void make_transition(int start_state) {

	// Allocating for each row of the transition table
	for (int i = 0; i < MAX_DFAS; ++i) {
		transition_table[i] = malloc(MAX_CHARS * sizeof(int));

		for (int j = 0; j < MAX_CHARS; ++j)
			*(transition_table[i] + j) = -1;
	}

	// Adding the start state
	SET* current_set = newset();
	ADD(current_set, start_state);

	char* accept = NULL;
	int anchor = 0;

	// Finding the e-closure of the start state
	e_closure(current_set, accept, &anchor);

	// Adding the start state of the DFA
	add_to_dfa_states(current_set, accept, anchor);

	DFA* dfa_ref = NULL;

	// Keep going until an unvisited state is seen
	while (dfa_ref = get_unmarked()) {

		dfa_ref->marked = true;
		current_set = dfa_ref->pset;

		// Check for all the input characters
		for (int c = 0; c < MAX_CHARS; ++c) {

			SET* next_set = move(current_set, c);

			// If there is no outgoing transition on that character
			if (!next_set)
				continue;

			char* next_accept = NULL;
			int next_anchor = 0;

			// Finding the e-closure of the set
			e_closure(next_set, &next_accept, &next_anchor);

			// If the set is already present
			int dfa_present_index = -1;
			if ((dfa_present_index = is_set_present(next_set)) != -1) {

				// Marking in the transition table
				*(transition_table[dfa_data.unmarked_index] + c) = dfa_present_index;

				delset(next_set);
				continue;
			}

			// If the set makes up a new DFA state

			// Marking in the transition table
			*(transition_table[dfa_data.unmarked_index] + c) = dfa_data.num_of_dfas;

			add_to_dfa_states(next_set, next_accept, next_anchor);
		}
	}
}