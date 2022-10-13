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
	//fread(source, sizeof(wchar_t), fs, f);
	fgetws(source, fs + 1, f);
	fclose(f);

	//const wchar_t* source = L"#abc = 123\ndef = 456\n// hello venus\nhello 123 \"setyor\" omega 0.1 0x99210000 0b10010110 3.1415 .0001 000.100 60_000_000 4.00_00 0b1000_1111_0000 @2a2569300065 {gayland xd 108 (hoho)} // xd\n\tmama\n\tpapa\n\t\tHOHO\nok boomer";
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