#ifndef _NFA_

#define _NFA_

#include <stdbool.h>
#include <TOOLS/SET.H>

#define MAX_NFAS 1024

enum edge_values { EMPTY = -3, CCL, EPSILON };
enum anchor_values { NONE, START, END, BOTH };

typedef enum token {
	EOS = 1,
	ANY,			// .
	AT_BOL,			// ^
	AT_EOL,			// $
	CCL_END,		// ]
	CCL_START,		// [
	CLOSE_CURLY,	// }
	CLOSE_PAREN,	// )
	CLOSURE,		// *
	DASH,			// -
	END_OF_INPUT,	// EOF
	L,				// Literal character
	OPEN_CURLY,		// {
	OPEN_PAREN,		// (
	OPTIONAL,		// ?
	OR,				// |
	PLUS_CLOSE		// +
} TOKEN;

typedef struct nfa {

	// An index for the nfa
	int index;

	// Stores whether the NFA is the start state
	bool is_start_state;

	// Stores whether it is a character, CCL (character class), EMPTY or EPSILON
	int edge;

	// If the edge stores a character class, to store all the characters
	SET* pset;

	struct nfa* next;
	struct nfa* next2;

	// Points to the string holding the action part of accepting states
	char* accept;

	int anchor;

} NFA;

// The structure which contains data about NFAs and their list
typedef struct NFA_data {

	int num_of_nfas;
	NFA* list_of_NFAs[MAX_NFAS];

} NFA_DATA;

#endif