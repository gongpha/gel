#include "gelc_tokenizer.h"
#include <stdio.h>
int main() {
	const wchar_t* source = L"#abc = 123\ndef = 456\n// hello venus\nhello 123 \"setyor\" omega 0.1 0x99210000 {gayland xd 108 (hoho)} // xd\n\tmama\n\tpapa\nok boomer";
	gelc_tokenizer t;
	gelc_tokenizer_create(&t, source);
	while (gelc_tokenizer_read(&t)) {
		if (t.error != ERROR_OK) {
			printf("k no\n");
			return -1;
		}
		switch (t.result.type) {
		case TT_COMMENT: printf("> COMMENT   : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_FREEZE:  printf("> FREEZE    : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_DEDENT:  printf("> DEDENT\n"); break;
		case TT_IDENT:   printf("> IDENT     : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_INDENT:  printf("> INDENT\n"); break;
		case TT_INT:     printf("> INT       : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_INT_HEX: printf("> INT (hex) : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_NEWLINE: printf("> NEWLINE\n"); break;
		case TT_OPERATOR:printf("> OPERATOR  : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		case TT_PAREN   :printf(t.result.data.integer == 0 ? "> '('\n" : "> ')'\n"); break;
		case TT_PAREN_BL:printf(t.result.data.integer == 0 ? "> '['\n" : "> ']'\n"); break;
		case TT_PAREN_BR:printf(t.result.data.integer == 0 ? "> '{'\n" : "> '}'\n"); break;
		case TT_REAL    :printf("> REAL      : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		//case TT_STABLE_LINK
		case TT_STRING  :printf("> STRING    : %.*ls\n", t.result.data.subsource.size, t.result.data.subsource.source); break;
		}
	}
	gelc_tokenizer_destroy(&t);
	return 0;
}