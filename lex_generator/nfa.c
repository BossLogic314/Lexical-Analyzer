#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "nfa.h"
#include "prototypes.h"
#include "globals.h"
#pragma warning (disable : 4996)

// Maps all ASCII characters to tokens
TOKEN token_map[] = {
	L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
	L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
	L, L, L, L, L, L, AT_EOL, L, L, L,
	OPEN_PAREN, CLOSE_PAREN, CLOSURE, PLUS_CLOSE, L, DASH, ANY,
	L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
	L, OPTIONAL,
	L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
	L, L, L, L, L, L, L, L, L, L, L, L,
	CCL_START, L, CCL_END, AT_BOL,
	L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
	L, L, L, L, L, L, L, L, L, L, L, L, L,
	OPEN_CURLY, OR, CLOSE_CURLY, L
};

// Returns pointer to an NFA structure
NFA* new_NFA() {

	NFA* pnfa = (NFA*)malloc(sizeof(NFA));

	// Check if there is enough memory
	if (!pnfa)
		print_error_message("Not enough memory to allocate for an NFA.\n");

	pnfa->index = -1;
	pnfa->is_start_state = false;
	pnfa->edge = EMPTY;
	pnfa->pset = NULL;
	pnfa->next = NULL;
	pnfa->next2 = NULL;
	pnfa->accept = NULL;
	pnfa->anchor = NONE;

	// Adding the new NFA pointer to the list
	nfa_data.list_of_NFAs[nfa_data.num_of_nfas++] = pnfa;

	return pnfa;
}

// Discards the given NFA
void discard_NFA(NFA* pnfa) {

	NFA** pp_ref_nfa = nfa_data.list_of_NFAs;
	NFA** pp_last_nfa = nfa_data.list_of_NFAs + nfa_data.num_of_nfas - 1;

	// Looping through the list to find the NFA to delete
	while (pp_ref_nfa != pp_last_nfa) {

		if (*pp_ref_nfa != pnfa) {
			pp_ref_nfa++;
			continue;
		}

		// Free-ing the allocated NFA pointer if found
		free(*pp_ref_nfa);

		if (--nfa_data.num_of_nfas)
			*pp_ref_nfa = *pp_last_nfa;

		return;
	}

	// If the NFA to free is the last in the list
	free(*pp_last_nfa);
	nfa_data.num_of_nfas--;
}

char get_special_lexeme() {

	// If the '\' is absent
	if (*input_buffer_ref != '\\')
		return *input_buffer_ref++;

	// If the '\' is present
	input_buffer_ref++;

	return *input_buffer_ref++;
}

void advance() {

	static bool in_quotes = false;

	if (current_token == EOS) {

		if (in_quotes)
			print_error_with_line_number();

		getline();

		// If the input is done
		if (input_buffer[0] == '\0') {
			lexeme = '\0';
			current_token = END_OF_INPUT;
			return;
		}

		input_buffer_ref = input_buffer;

		while (isspace(*input_buffer_ref))
			input_buffer_ref++;

		// Keep reading lines until a valid regular expression is read
		while (!*input_buffer_ref) {

			getline();

			// If the input is done
			if (input_buffer[0] == '\0') {
				lexeme = '\0';
				current_token = END_OF_INPUT;
				return;
			}

			input_buffer_ref = input_buffer;

			while (isspace(*input_buffer_ref))
				input_buffer_ref++;
		}
	}

	if (*input_buffer_ref == '"') {

		in_quotes = !in_quotes;
		input_buffer_ref++;
	}

	// Keep popping the previously stored strings on the stack
	while (*input_buffer_ref == '\0') {

		// If the stack is empty
		if (stack_top == stack - 1) {
			lexeme = '\0';
			current_token = EOS;
			return;
		}

		input_buffer_ref = *stack_top--;

		// Moving to the characters after '}'
		while (*input_buffer_ref != '}')
			++input_buffer_ref;

		++input_buffer_ref;
	}

	if (!in_quotes) {

		// If a new macro is going to appear
		while (*input_buffer_ref == '{') {

			*++stack_top = input_buffer_ref;
			input_buffer_ref = get_macro_text(input_buffer_ref);
		}
	}

	bool is_escape = (*input_buffer_ref == '\\');

	if (!in_quotes) {

		// If a space is seen, that is the end of the regular expression
		if (isspace(*input_buffer_ref)) {
			lexeme = '\0';
			current_token = EOS;
			return;
		}

		lexeme = get_special_lexeme();
	}
	else {

		if (is_escape && *(input_buffer_ref + 1) == '"') {
			lexeme = '"';
			input_buffer_ref += 2;
		}
		else
			lexeme = *input_buffer_ref++;
	}

	current_token = (in_quotes || is_escape) ? L : token_map[lexeme];
}

// Returns the pointer to the start NFA of the machine
NFA* machine() {

	advance();
	NFA* start, * p;

	p = start = new_NFA();
	p->edge = EPSILON;
	p->next = rule();

	while (current_token != END_OF_INPUT) {

		p->next2 = new_NFA();
		p = p->next2;
		p->edge = EPSILON;

		p->next = rule();
	}

	start->is_start_state = true;
	return start;
}

NFA* rule() {

	NFA* start, * end;

	if (current_token == AT_BOL) {

		start = new_NFA();
		start->edge = '\n';
		start->anchor |= START;

		advance();
		expr(&(start->next), &end);
	}
	else
		expr(&start, &end);

	if (current_token == AT_EOL) {

		end->next = new_NFA();
		end->edge = '\n';
		end->anchor |= END;
		end = end->next;

		advance();
	}

	// Ignoring all the space characters
	while (isspace(*input_buffer_ref))
		input_buffer_ref++;

	// To allocate to the accept string
	int len = (strlen(input_buffer_ref) + 1);

	end->accept = (char*)malloc(len * sizeof(char));
	strcpy(end->accept, input_buffer_ref);

	advance();

	return start;
}

void expr(NFA** ppstart, NFA** ppend) {

	cat_expr(ppstart, ppend);

	// Continuously call cat_expr if 'or' is encountered
	while (current_token == OR) {

		advance();

		NFA* cat_start, * cat_end;
		cat_expr(&cat_start, &cat_end);

		NFA* new_start, * new_end;
		new_start = new_NFA();
		new_end = new_NFA();

		new_start->edge = EPSILON;
		(*ppend)->edge = EPSILON;
		cat_end->edge = EPSILON;

		// Left link
		new_start->next = *ppstart;
		new_start->next2 = cat_start;

		// Right link
		(*ppend)->next = new_end;
		cat_end->next = new_end;

		// Updating pointers
		*ppstart = new_start;
		*ppend = new_end;
	}
}

// Determines if the current token can be concatenated to the previous one
bool first_in_cat() {

	switch (current_token)
	{
	case EOS: case AT_EOL: case OR: case CLOSE_PAREN:
		return false;

	case CLOSURE: case PLUS_CLOSE: case OPTIONAL:
		print_error_with_line_number();
		return false;

	case AT_BOL:
		print_error_with_line_number();
		return false;

	case CCL_END:
		print_error_with_line_number();
		return false;

	default:
		return true;
	}
}

void cat_expr(NFA** ppstart, NFA** ppend) {

	if (first_in_cat())
		factor(ppstart, ppend);

	while (first_in_cat()) {

		NFA* factor_start, * factor_end;
		factor(&factor_start, &factor_end);

		// Concatenating
		memcpy(*ppend, factor_start, sizeof(NFA));
		*ppend = factor_end;

		discard_NFA(factor_start);
	}
}

void factor(NFA** ppstart, NFA** ppend) {

	term(ppstart, ppend);

	if (!(current_token == CLOSURE || current_token == PLUS_CLOSE || current_token == OPTIONAL))
		return;

	NFA* new_start = new_NFA();
	NFA* new_end = new_NFA();

	new_start->edge = EPSILON;
	(*ppend)->edge = EPSILON;

	switch (current_token)
	{
	case CLOSURE:
		new_start->next = *ppstart;
		new_start->next2 = new_end;
		(*ppend)->next = *ppstart;
		(*ppend)->next2 = new_end;
		advance();
		break;

	case PLUS_CLOSE:
		new_start->next = *ppstart;
		(*ppend)->next = *ppstart;
		(*ppend)->next2 = new_end;
		advance();
		break;

	case OPTIONAL:
		new_start->next = *ppstart;
		new_start->next2 = new_end;
		(*ppend)->next = new_end;
		advance();
		break;

	default:
		break;
	}

	*ppstart = new_start;
	*ppend = new_end;
}

void dodash(SET* pset) {

	while (current_token != CCL_END) {
		
		int start = lexeme;
		ADD(pset, start);
		advance();

		// If a dash is seen
		if (current_token == DASH) {
			advance();

			for (int i = start; i <= lexeme; ++i)
				ADD(pset, i);

			advance();
		}
	}

	advance();
}

void term(NFA** ppstart, NFA** ppend) {

	if (current_token == OPEN_PAREN) {
		advance();
		expr(ppstart, ppend);

		if (current_token != CLOSE_PAREN)
			print_error_with_line_number();

		advance();
		return;
	}

	*ppstart = new_NFA();
	*ppend = new_NFA();

	if (current_token != ANY && current_token != CCL_START) {

		(*ppstart)->next = *ppend;
		(*ppstart)->edge = lexeme;

		advance();

		return;
	}

	// Allocating the set
	if (((*ppstart)->pset = newset()) == NULL)
		print_error_message("Not enough memory to allocate a set.\n");

	// Lexeme is '.'
	if (current_token == ANY) {

		(*ppstart)->next = *ppend;
		(*ppstart)->edge = CCL;

		ADD((*ppstart)->pset, '\n');
		COMPLEMENT((*ppstart)->pset);

		advance();

		return;
	}

	// This is a CCL
	advance();

	if (current_token == AT_BOL) {

		COMPLEMENT((*ppstart)->pset);
		ADD((*ppstart)->pset, '\n');

		advance();
	}

	// This is the case of [], which should accept only empty spaces
	if (current_token == CCL_END){

		ADD((*ppstart)->pset, ' ');
		ADD((*ppstart)->pset, '\t');

		(*ppstart)->next = *ppend;
		(*ppstart)->edge = CCL;

		advance();
		return;
	}

	dodash((*ppstart)->pset);
	(*ppstart)->next = *ppend;
	(*ppstart)->edge = CCL;
}

void assign_indices() {

	int len = nfa_data.num_of_nfas;

	// Assigning an index to each NFA
	for (int i = 0; i < len; ++i)
		nfa_data.list_of_NFAs[i]->index = i;
}

// Printing all the NFAs
void print_nfas() {

	int len = nfa_data.num_of_nfas;

	printf("Number of states in the NFA is %d\n", len);

	assign_indices();

	for (int i = 0; i < len; ++i) {

		NFA* ref_nfa = nfa_data.list_of_NFAs[i];
		
		printf("NFA state number %d ", ref_nfa->index);

		// Printing the nfa in 'next'
		ref_nfa->next == NULL ? printf("-> NULL ") : printf("-> state number %d ", ref_nfa->next->index);

		// Printing the nfa in 'next2'
		ref_nfa->next2 == NULL ? printf("( -- ) ") : printf("( %d ) ", ref_nfa->next2->index);

		// Printing all the elements on the edge
		if (ref_nfa->edge == EMPTY)
			printf("[Terminal]");
		else if (ref_nfa->edge == EPSILON)
			printf("on EPSILON");
		else if (ref_nfa->edge != CCL)
			printf("on %c", ref_nfa->edge);
		else {
			printf("on [ ");

			int ret;
			while ((ret = next_member(ref_nfa->pset)) != -1)
				printf("%c ", ret);

			printf("]");
		}

		if (ref_nfa->is_start_state)
			printf(" <Start state>");

		if (ref_nfa->accept != NULL)
			printf(" %s", ref_nfa->accept);

		printf("\n");
	}
}

void free_all_nfas() {

	int len = nfa_data.num_of_nfas;

	for (int i = 0; i < len; ++i) {

		NFA* ref_nfa = nfa_data.list_of_NFAs[i];

		if (ref_nfa->accept != NULL)
			free(ref_nfa->accept);

		if (ref_nfa->pset != NULL)
			delset(ref_nfa->pset);

		free(ref_nfa);
	}
}