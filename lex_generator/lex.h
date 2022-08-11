#ifndef _LEX_

#define _LEX_

#include <TOOLS/HASH.H>

#define MAX_LEN 256

enum COMMENT_STATE { NO_COMMENT, INLINE_COMMENT, BLOCK_COMMENT };

typedef struct macro {

	char name[MAX_LEN];
	char text[MAX_LEN];

}MACRO;

int line_number;
HASH_TAB* p_hash_table;

#endif