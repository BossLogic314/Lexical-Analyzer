#ifndef _DFA_

#define _DFA_

#include <stdbool.h>
#include <TOOLS/SET.H>

#define MAX_DFAS 4096
#define MAX_COUNT 1000

typedef struct dfa {

	bool marked;

	// String, if the DFA is an accept state
	char accept[MAX_COUNT];

	int anchor;

	// The set of NFA states that it represents
	SET* pset;

} DFA;

typedef struct DFA_data {

	int num_of_dfas;
	DFA* list_of_DFAs[MAX_DFAS];

	int unmarked_index;

} DFA_DATA;

#endif