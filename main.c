#include "gelc_tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

void print_token(const gelc_tokenizer* t) {
	switch (t->result.type) {
	case TT_COMMENT:		printf("> COMMENT   : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_FREEZE:			printf("> FREEZE    : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_DEDENT:			printf("> DEDENT\n"); break;
	case TT_IDENT:			printf("> IDENT     : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_INDENT:			printf("> INDENT\n"); break;
	case TT_INT_DEC:		printf("> INT (dec) : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_INT_HEX:		printf("> INT (hex) : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_INT_BIN:		printf("> INT (bin) : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_NEWLINE:		printf("> NEWLINE\n"); break;
	case TT_OPERATOR:		printf("> OPERATOR  : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_PAREN:			printf(t->result.data.integer == 0 ? "> '('\n" : "> ')'\n"); break;
	case TT_PAREN_BL:		printf(t->result.data.integer == 0 ? "> '['\n" : "> ']'\n"); break;
	case TT_PAREN_BR:		printf(t->result.data.integer == 0 ? "> '{'\n" : "> '}'\n"); break;
	case TT_REAL:			printf("> REAL      : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_STABLE_LINK:	printf("> STABLE    : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	case TT_STRING:			printf("> STRING    : %.*ls\n", t->result.data.subsource.size, t->result.data.subsource.source); break;
	}
}

int main() {
	FILE* f = fopen("../../../sample/001.gel", "r");
	fseek(f, 0, SEEK_END);
	size_t fs = ftell(f);
	fseek(f, 0, SEEK_SET);

	wchar_t* source = malloc(sizeof(wchar_t) * (fs + 1));
	fgetws(source, fs + 1, f);
	fclose(f);
	
	gelc_tokenizer t;
	gelc_tokenizer_create(&t, source);
	while (gelc_tokenizer_read(&t)) {
		if (t.error != ERROR_OK) {
			printf("k no\n");
			return -1;
		}
		print_token(&t);
	}
	gelc_tokenizer_destroy(&t);
	return 0;
}