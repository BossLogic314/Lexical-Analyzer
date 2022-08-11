#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define _GLOBALS_DEF_

#include "nfa.h"
#include "dfa.h"
#include "lex.h"
#include "prototypes.h"
#include "globals.h"
#pragma warning (disable : 4996)

char* get_macro_text(char* macro_name) {

	if (*macro_name++ != '{')
		return NULL;

	char text[MAX_COUNT];
	char* text_ref = text;

	while (*macro_name != '}')
		*text_ref++ = *macro_name++;

	*text_ref = '\0';

	MACRO* pmacro =  findsym(p_hash_table, text);
	return pmacro->text;
}

void getline() {

	char ref_buffer[MAX_COUNT];

	line_number++;

	if (fgets(input_buffer, MAX_COUNT, input_fp) == NULL)
		print_error_with_line_number();

	// If all the regular expressions are done reading
	if (input_buffer[0] == '%') {
		input_buffer[0] = '\0';
		return;
	}

	// The line cannot start with a space character
	if (input_buffer[0] == ' ' || input_buffer[0] == '\t' || input_buffer[0] == '\n')
		print_error_with_line_number();

	char c = 'A';
	while (c = fgetc(input_fp)) {

		// Un-reading the character back to the input file
		ungetc(c, input_fp);

		// If a space character is seen in the next line
		if (c == ' ' || c == '\t' || c == '\n') {

			line_number++;

			if (fgets(ref_buffer, MAX_COUNT, input_fp) == NULL)
				print_error_with_line_number();

			strcat(input_buffer, ref_buffer);

			continue;
		}

		break;
	}
}

void print_error_with_line_number() {

	fprintf(stderr, "Error in line number %d\n", line_number);
	fclose(input_fp);
	fclose(output_fp);
	exit(1);
}

void print_macros(MACRO* pmacro) {
	printf("%s -> %s\n", pmacro->name, pmacro->text);
}

void print_error_message(char* message) {

	fprintf(stderr, message);
	exit(1);
}

char* strip_comments(char* string, int* p_in_comment) {

	char* c = string, *ref = NULL;

	// Looping every character
	while (*c != '\0' && *c != '\n') {

		// If the text is in in-line comments
		if (*p_in_comment == INLINE_COMMENT) {
			*c++ = ' ';
			continue;
		}

		// If the text is in block comments
		if (*p_in_comment == BLOCK_COMMENT) {

			// Block comments end
			if (*c == '*' && *(c + 1) == '/') {
				*c = ' ';
				*(c + 1) = ' ';
				c += 2;

				*p_in_comment = NO_COMMENT;
				continue;
			}

			*c++ = ' ';
			continue;
		}

		// In-line comments start
		if (*c == '/' && *(c + 1) == '/') {
			*c = ' ';
			*(c + 1) = ' ';

			*p_in_comment = INLINE_COMMENT;
			c += 2;
			continue;
		}

		// Block comments start
		if (*c == '/' && *(c + 1) == '*') {
			*c = ' ';
			*(c + 1) = ' ';

			*p_in_comment = BLOCK_COMMENT;
			c += 2;
			continue;
		}

		if (ref == NULL)
			ref = c;

		c++;
	}

	// Removing the flag that an in-line comment is present
	if (*p_in_comment == INLINE_COMMENT)
		*p_in_comment = NO_COMMENT;

	return ref;
}

void head_parsing() {

	bool before_head = true, is_head = false;

	int in_comment = 0;

	// Keep reading lines from the '.lex' file
	while (true) {

		line_number++;

		if (fgets(input_buffer, MAX_COUNT, input_fp) == NULL)
			print_error_with_line_number();

		// Getting the first non-comment character
		char* ref = strip_comments(input_buffer, &in_comment);

		if (before_head) {

			// If the line is a full comment
			if (ref == NULL)
				continue;

			// If the head template is not started
			if (*ref != '%' || *(ref + 1) != '{')
				print_error_with_line_number();

			before_head = false;
			is_head = true;

			continue;
		}

		// In the 'head' section
		if (is_head) {

			// If the line is a full comment
			if (ref == NULL)
				continue;

			// Ending the 'head' section
			if (*ref == '%' && *(ref + 1) == '}')
				return;

			fputs(input_buffer, output_fp);
			continue;
		}

		fputs("\n", output_fp);
		return;
	}
}

void macro_processing() {

	int in_comment = 0;

	bool first_macro = true;

	while (true) {

		line_number++;

		if (fgets(input_buffer, MAX_COUNT, input_fp) == NULL)
			print_error_with_line_number();

		// Getting the first non-comment character
		char* macro_name_start = strip_comments(input_buffer, &in_comment);

		// If the line is empty
		if (!macro_name_start || *macro_name_start == '\0')
			continue;

		// If the end of the macros is seen
		if (*macro_name_start == '%' && *(macro_name_start + 1) == '%')
			break;

		// Finding the end of the macro name
		char* macro_name_end = macro_name_start;
		while (*macro_name_end != ' ' && *macro_name_end != '\t' && *macro_name_end != '\n')
			macro_name_end++;

		if (first_macro) {
			p_hash_table = maketab(256, hash_add, strcmp);
			first_macro = false;
		}

		// Finding the start of the macro text
		char* macro_text_start = macro_name_end;
		while (*macro_text_start == ' ' || *macro_text_start == '\t')
			macro_text_start++;

		// Finding the end of the macro text
		char* macro_text_end = macro_text_start;
		while (*macro_text_end != ' ' && *macro_text_end != '\t' && *macro_text_end != '\n')
			macro_text_end++;

		// Inserting the macro into the hash table
		MACRO* macro = newsym(sizeof(MACRO));
		strncpy(macro->name, macro_name_start, macro_name_end - macro_name_start);
		strncpy(macro->text, macro_text_start, macro_text_end - macro_text_start);
		addsym(p_hash_table, macro);
	}

	printf("The macros provided are -\n");
	ptab(p_hash_table, print_macros, NULL, 1);
	printf("\n");
}

void write_switch_statement_to_c_file() {

	int len = dfa_data.num_of_dfas;
	char buffer[MAX_COUNT];

	for (int i = 0; i < len; ++i) {

		DFA* dfa_ref = dfa_data.list_of_DFAs[i];

		// If not an accepting state
		if (dfa_ref->accept[0] == '\0')
			continue;

		// If it is an accepting state
		snprintf(buffer, MAX_COUNT, "case %d:\n\t%sbreak;\n", i, dfa_ref->accept);
		fputs(buffer, output_fp);
	}
}

void write_transition_table_to_c_file() {

	char buffer[MAX_COUNT];
	int num_of_dfa = dfa_data.num_of_dfas;

	// Defining the variable in the '.c' file
	snprintf(buffer, MAX_COUNT, "static unsigned char Yy_nxt[%d][%d] = {\n", num_of_dfa, MAX_CHARS);
	fputs(buffer, output_fp);

	// Initializing the values in the transition table
	for (int i = 0; i < num_of_dfa; ++i) {

		fputs("\t{\n\t", output_fp);
		for (int j = 0; j < MAX_CHARS; ++j) {

			snprintf(buffer, MAX_COUNT, "%d, ", *(transition_table[i] + j));
			fputs(buffer, output_fp);

			// Adding a '\n' for readability
			if ((j + 1) % 10 == 0)
				fputs("\n\t", output_fp);
		}
		fputs("\n\t},\n", output_fp);
	}

	fputs("};\n\n", output_fp);

	// Adding the #define to the input_buffer
	snprintf(buffer, MAX_COUNT, "#define yy_next(state, c) Yy_nxt[state][c]\n\n");
	fputs(buffer, output_fp);

	// Adding the accept states to the '.c' file
	snprintf(buffer, MAX_COUNT, "static unsigned char Yyaccept[] = {\n");
	fputs(buffer, output_fp);

	for (int i = 0; i < num_of_dfa; ++i) {

		int state_value = 0;
		DFA* dfa_ref = dfa_data.list_of_DFAs[i];

		// If it is an accepting state
		if (dfa_ref->accept[0] != '\0')
			state_value = (dfa_ref->anchor == 0) ? 4 : dfa_ref->anchor;

		snprintf(buffer, MAX_COUNT, "\t%d,\n", state_value);
		fputs(buffer, output_fp);
	}

	fputs("};\n\n", output_fp);
}

void lex() {

	char* par_file_name = "D:\\workspace\\compiler\\SAMPLES\\LEX.PAR";
	FILE* par_fp = fopen(par_file_name, "r");

	bool first_time = true;

	// Keep reading from the '.par' until EOF
	while (fgets(input_buffer, MAX_COUNT, par_fp)) {

		input_buffer_ref = input_buffer;

		// Deleting all the space characters
		if (input_buffer[0] != '\f') {

			while (isspace(*input_buffer_ref))
				input_buffer_ref++;
		}

		// If the line was empty
		if (*input_buffer_ref == '@'  || *input_buffer_ref == '\0' || *input_buffer_ref == '\n')
			continue;

		// If not the 'form feed' character
		if (input_buffer[0] != '\f') {

			fputs(input_buffer, output_fp);
			continue;
		}

		// If the 'form feed' character is seen for the first time
		if (first_time) {

			first_time = !first_time;

			// The function writes to the file
			write_transition_table_to_c_file();

			continue;
		}

		// The 'form feed' character is seen for the second time
		write_switch_statement_to_c_file();
	}

	// Keep reading the last part of the '.lex' file and paste it into the '.c' file
	while (fgets(input_buffer, MAX_COUNT, input_fp) != NULL)
		fputs(input_buffer, output_fp);
}

int main(int argc, char* argv[]) {

	// Checking if the '.lex' file is provided
	if (argc != 3)
		print_error_message("Invalid command line arguments provided.\n Please provide two arguments -> input file path followed by the output file path.\n");

	// The path of '.lex' file
	char* input_file_name = argv[1];
	char* output_file_name = argv[2];

	input_fp = fopen(input_file_name, "r");
	output_fp = fopen(output_file_name, "w");

	if (!input_fp)
		print_error_message("Invalid '.lex' file provided.\n");

	// Calling the function to process the head part
	head_parsing();

	// Calling the function to store all the macros
	macro_processing();

	current_token = EOS;
	machine();
	print_nfas();

	// Finding the NFA start state
	int start_state = -1;

	for (int i = 0; i < nfa_data.num_of_nfas; ++i) {

		if (nfa_data.list_of_NFAs[i]->is_start_state) {
			start_state = i;
			break;
		}
	}

	// To transition from an NFA to a DFA
	make_transition(start_state);

	print_dfas();
	free_all_dfa_sets();

	free_all_nfas();

	print_transition_table();

	lex();

	free_transition_table();
	free_all_dfas();

	fclose(input_fp);
	fclose(output_fp);
	return 0;
}