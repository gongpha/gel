#include "gelc_tokenizer.h"
#include <string.h>

void gelc_tokenizer_create(gelc_tokenizer* tokenizer, const wchar_t* source) {
	tokenizer->source = source;
	tokenizer->cursor = source;
	tokenizer->line_cursor_start = source;
	tokenizer->scope = TT_NONE;
	tokenizer->parsing_ident = 0;
	tokenizer->line = 1;
	tokenizer->tune = 0;
}

int terminate_ident(gelc_tokenizer* t) {
	if (t->parsing_ident == 0) return 0;

	t->result.type = TT_IDENT;
	substring(&t->result.data, t->begin, t->cursor);

	t->parsing_ident = 0;
	return 1;
}

void substring(gelc_token_literal* lit, const wchar_t* begin, const wchar_t*end) {
	lit->source = begin;
	lit->size = end - begin;
}

int gelc_tokenizer_read(gelc_tokenizer* t)
{
	for (;;) {
		wchar_t ch = *t->cursor;
		if (ch == 0) return 0;
		int exit = 0;
		switch (t->scope) {
		case TT_NONE:
			switch (ch) {
				case L'"':
					if (terminate_ident(t)) return 1;
					t->scope = TT_STRING;
					t->cursor++;
					t->begin = t->cursor;
					exit = 1;
				case L'\n':
					if (terminate_ident(t)) return 1;

					t->result.type = TT_NEWLINE;
					t->cursor++;
					t->line++;
					t->line_cursor_start = t->cursor;
					if (t->tune & TUNE_INDENT)
						t->scope = TT_INDENT;

					t->scope = TT_STRING;
					t->cursor++;
					t->begin = t->cursor;
					exit = 1;
				break;
			}
			if (exit) continue;
			break;
		}
	}
	return 0;
}
#undef ALLOC