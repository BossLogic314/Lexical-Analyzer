#include <stdio.h>
#include <TOOLS/SET.H>
#include "globals.h"
#pragma warning (disable : 4996)

void e_closure(SET* p_input_set, char** paccept, int* panchor) {

	// If the input set points to NULL
	if (!p_input_set)
		return;

	int stack[STACK_SIZE];
	int* stack_top = stack - 1;

	int anchor = *panchor;

	// Getting the first input state of the NFA
	int current_state = next_member(p_input_set);

	// If the input set is empty
	if (current_state == -1)
		return;

	// Pushing all the input states onto the stack
	while (current_state != -1) {

		*++stack_top = current_state;

		current_state = next_member(p_input_set);
	}

	// Going on until the stack is empty
	while (stack_top != (stack - 1)) {

		// Popping the top-most state
		current_state = *stack_top--;

		// Adding the current state to the set
		ADD(p_input_set, current_state);

		NFA* ref_nfa = nfa_data.list_of_NFAs[current_state];

		// If the current state is an accepting state
		if (ref_nfa->accept != NULL) {
			*paccept = ref_nfa->accept;
			anchor = ref_nfa->anchor;
		}

		// If the state does not have an EPSILON transition
		if (ref_nfa->edge != EPSILON)
			continue;

		// Pushing the next states onto the stack
		if (ref_nfa->next != NULL) {

			if (!MEMBER(p_input_set, ref_nfa->next->index))
				*++stack_top = (ref_nfa->next)->index;
		}

		if (ref_nfa->next2 != NULL) {

			if (!MEMBER(p_input_set, ref_nfa->next2->index))
				*++stack_top = (ref_nfa->next2)->index;
		}
	}
}

SET* move(SET* p_input_set, int c) {

	if (!p_input_set)
		return NULL;

	SET* p_output_set = NULL;

	int current_state;

	// Resetting the current set
	next_member(NULL);

	while ((current_state = next_member(p_input_set)) != -1) {

		NFA* ref_nfa = nfa_data.list_of_NFAs[current_state];

		if (ref_nfa->edge == c || (ref_nfa->pset != NULL && MEMBER(ref_nfa->pset, c))) {

			if (!p_output_set)
				p_output_set = newset();

			ADD(p_output_set, (ref_nfa->next)->index);
		}
	}

	return p_output_set;
}