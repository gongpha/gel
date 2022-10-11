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
	tokenizer->paren_level = 0;
	tokenizer->str_escaping = 0;
	tokenizer->parsing_hint = 0;
	tokenizer->error = ERROR_OK;
}

int terminate_ident(gelc_tokenizer* t) {
	if (t->parsing_ident == 0) return 0;

	t->result.type = TT_IDENT;
	substring(&t->result.data.literal, t->begin, t->cursor);

	t->parsing_ident = 0;
	return 1;
}

void substring(gelc_token_literal* lit, const wchar_t* begin, const wchar_t* end) {
	lit->source = begin;
	lit->size = end - begin;
}

int gelc_tokenizer_read(gelc_tokenizer* t)
{
	for (;;) {
		const wchar_t ch = *t->cursor;
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
				//if (t->tune & TUNE_INDENT)
				if (t->paren_level == 0)
					t->scope = TT_INDENT;

				t->scope = TT_STRING;
				t->cursor++;
				t->begin = t->cursor;
				return 1;
			case L'#':
				if (terminate_ident(t)) return 1;
				t->scope = TT_COMMENT;
				t->cursor++;
				t->begin = t->cursor;
				exit = 1;
				break;
			}
			if (exit) continue;

			if (ch == L'(' || ch == L')') {
				if (terminate_ident(t)) return 1;
				t->result.type = TT_PAREN;
				t->result.data.integer = ch == L')';
				t->paren_level += (t->result.data.integer == 0) ? 1 : -1;
				t->cursor++;
				return 1;
			}
			else if (ch == L'[' || ch == L']') {
				if (terminate_ident(t)) return 1;
				t->result.type = TT_PAREN_BL;
				t->result.data.integer = ch == L']';
				t->paren_level += (t->result.data.integer == 0) ? 1 : -1;
				t->cursor++;
				return 1;
			}
			else if (ch == L'{' || ch == L'}') {
				if (terminate_ident(t)) return 1;
				t->result.type = TT_PAREN_BL;
				t->result.data.integer = ch == L']';
				t->paren_level += (t->result.data.integer == 0) ? 1 : -1;
				t->cursor++;
				return 1;
			}

			int is_period = ch == L'.';
			int is_num = ch >= L'0' && ch <= L'9';
			if (is_period || is_num) {
				if ((is_num && !t->parsing_ident) || is_period) {
					if (terminate_ident(t)) return 1;
					t->scope = (is_period) ? TT_REAL : TT_INT;
					t->begin = t->cursor;
					t->cursor++;
					continue;
				}
			}


			const wchar_t* operators[] = GELC_CONST_OPERATORS;
			const wchar_t* operatorss = GELC_CONST_OPERATORS_SINGLE;

			for (size_t i = 0; i <= sizeof(operators) / sizeof(operators[0]); i++) {
				const size_t len = wcslen(operators[i]);
				if (wcsncmp(operators[i], t->cursor, len) == 0) {
					t->result.type = TT_OPERATOR;
					t->result.data.operator = operators[i];
					t->cursor += len;
					return 1;
				}
			}
			for (size_t i = 0; i <= sizeof(operatorss) / sizeof(operatorss[0]); i++) {
				if (operatorss[i] == ch) {
					if (terminate_ident(t)) return 1;
					t->result.type = TT_OPERATOR;
					t->result.data.operator = operatorss[i];
					t->cursor++;
					return 1;
				}
			}

			if (ch == L' ' || ch == L'\t') {
				if (!terminate_ident(t)) {
					// not ident
					t->cursor++;
					continue;
				}
				t->cursor++;
				return 1;
			}
			if (!t->parsing_ident) {
				t->begin = t->cursor;
			}
			t->parsing_ident = 1;
			t->cursor++;

			break;
		case TT_STRING:
			if (t->str_escaping) t->str_escaping = 0;
			else if (ch == L'\\') t->str_escaping = 1;
			else if (ch == L'"') {
				t->result.type = TT_STRING;
				substring(&t->result.data.literal, t->begin, t->cursor);
				t->scope = TT_NONE;
				t->cursor++;
				return 1;
			}
			break;
		case TT_REAL:
		case TT_INT:
			if (ch >= L'0' && ch <= L'9') t->cursor++;
			else if (
				t->parsing_hint == PHINT_HEX &&
				(
					(
						ch >= L'a' && ch <= L'f'
						)
					|| (
						ch >= L'A' && ch <= L'F'
						)
					)
				) t->cursor++;
			else if (ch == L'.') {
				if (t->scope == TT_REAL) {
					// passing float. BUT WHY AGAIN
					t->error = ERROR_INVALID_REAL;
					return 1;
				}
				t->scope = TT_REAL;
				t->cursor++;
			}
			break;
		}
	}
	return 0;
}
#undef ALLOC