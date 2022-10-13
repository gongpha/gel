#include "gelc_tokenizer.h"
#include <string.h>
#include <stdlib.h>

#define GELC_CONST_OPERATORS_SINGLE L"+-*/^&%!:=.,<>"
#define GELC_CONST_OPERATORS {L"=>", L"<=", L">=", L"==", L"!=", L"..."}

void gelc_tokenizer_create(gelc_tokenizer* tokenizer, const wchar_t* source) {
	tokenizer->source = source; // keep the source !
	tokenizer->cursor = source;
	tokenizer->line_cursor_start = source;
	tokenizer->scope = TT_NONE;
	tokenizer->parsing_ident = 0;
	tokenizer->line = 1;
	tokenizer->tune = 0;
	tokenizer->paren_level = 0;
	tokenizer->error = ERROR_OK;
	tokenizer->indents.size = 0;
	tokenizer->indents.list = NULL;
	tokenizer->indents.alloc = 0;
}

void gelc_tokenizer_destroy(const gelc_tokenizer* tokenizer) {
	if (tokenizer->indents.list) free(tokenizer->indents.list);
}

void subsource(gelc_token_subsource* ss, const wchar_t* begin, const wchar_t* end) {
	ss->source = begin;
	ss->size = end - begin;
}

int terminate_ident(gelc_tokenizer* t) {
	if (t->parsing_ident == 0) return 0;

	t->result.type = TT_IDENT;
	subsource(&t->result.data.subsource, t->begin, t->cursor);

	t->parsing_ident = 0;
	return 1;
}

int is_whitespace(wchar_t ch) {
	return (ch == L' ' || ch == L'\t' || ch == 0);
}

void alloc_num(gelc_tokenizer* t) {
	t->result.type = t->scope;
	subsource(&t->result.data.subsource, t->begin, t->cursor);
}

void advance(gelc_tokenizer* t) {
	if (*t->cursor != 0) t->cursor++;
}

void advance_big(gelc_tokenizer* t, size_t len) {
	if (*t->cursor != 0) t->cursor += len;
}

int is_indent(const wchar_t ch) {
	return ch == L' ' || ch == L'\t';
}

int is_numeric(const wchar_t ch) {
	return ch >= L'0' && ch <= L'9';
}

int is_hex(const wchar_t ch) {
	return (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F');
}

int is_bin(const wchar_t ch) {
	return ch == L'0' || ch == L'1';
}

int gelc_tokenizer_read(gelc_tokenizer* t) {
	for (;;) {
		const wchar_t ch = *t->cursor;
		if (ch == 0)
			if (t->scope == TT_NONE && t->parsing_ident == 0) return 0;
		int exit = 0;
		switch (t->scope) {
		case TT_NONE:
			switch (ch) {
			case L'"':
				if (terminate_ident(t)) return 1;
				t->scope = TT_STRING;
				t->scope_data.string.str_escaping = 0;
				advance(t);
				t->begin = t->cursor;
				exit = 1;
				break;
			case L'\n':
				if (terminate_ident(t)) return 1;

				t->result.type = TT_NEWLINE;
				advance(t);
				t->line++;
				t->line_cursor_start = t->cursor;
				//if (t->tune & TUNE_INDENT)
				if (t->paren_level == 0) {
					t->scope = TT_INDENT;
					t->scope_data.indent.indent_begin = t->cursor;
					t->scope_data.indent.current_item = 0;
					t->scope_data.indent.last_level = t->indents.size;
					t->scope_data.indent.dedenting = 0;
					t->begin = t->cursor;
				}
				return 1;
			case L'/':
				if (terminate_ident(t)) return 1;
				if (*(t->cursor + 1) == L'/') {
					t->scope = TT_COMMENT;
					advance_big(t, 2);
					t->begin = t->cursor;
					exit = 1;
				}
				else if (*(t->cursor + 1) + 1 == L'*') {
					t->scope = TT_COMMENT_BLOCK;
					advance_big(t, 2);
					t->begin = t->cursor;
					exit = 1;
				}
				break;
			case L'#':
				if (terminate_ident(t)) return 1;
				t->scope = TT_FREEZE;
				advance(t);
				t->begin = t->cursor;
				exit = 1;
				break;
			case L'@':
				if (terminate_ident(t)) return 1;
				t->scope = TT_STABLE_LINK;
				advance(t);
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
				advance(t);
				return 1;
			}
			else if (ch == L'[' || ch == L']') {
				if (terminate_ident(t)) return 1;
				t->result.type = TT_PAREN_BL;
				t->result.data.integer = ch == L']';
				t->paren_level += (t->result.data.integer == 0) ? 1 : -1;
				advance(t);
				return 1;
			}
			else if (ch == L'{' || ch == L'}') {
				if (terminate_ident(t)) return 1;
				t->result.type = TT_PAREN_BR;
				t->result.data.integer = ch == L'}';
				t->paren_level += (t->result.data.integer == 0) ? 1 : -1;
				advance(t);
				return 1;
			}

			int is_period = ch == L'.';
			int is_num = ch >= L'0' && ch <= L'9';

			if (is_num || is_period) {
				gelc_token_type tt = TT_INT_DEC;
				if (ch == L'0') {
					if (*(t->cursor + 1) == L'x') {
						tt = TT_INT_HEX;
					}
					else if (*(t->cursor + 1) == L'b') {
						tt = TT_INT_BIN;
					}
				}
				else if (is_period) {
					tt = TT_REAL;
				}
				if (terminate_ident(t)) return 1;
				t->scope = tt;
				t->begin = t->cursor;
				if (tt == TT_INT_HEX || tt == TT_INT_BIN) {
					advance_big(t, 2);
				} else advance(t);
				
				continue;
			}

			const wchar_t* operators[] = GELC_CONST_OPERATORS;
			const wchar_t operatorss[] = GELC_CONST_OPERATORS_SINGLE;

			for (size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {
				const size_t len = wcslen(operators[i]);
				if (wcsncmp(operators[i], t->cursor, len) == 0) {
					t->result.type = TT_OPERATOR;
					t->result.data.subsource.source = t->cursor;
					t->result.data.subsource.size = len;
					advance_big(t, len);
					return 1;
				}
			}
			for (size_t i = 0; i <= sizeof(operatorss) / sizeof(operatorss[0]); i++) {
				if (operatorss[i] == ch) {
					if (terminate_ident(t)) return 1;
					t->result.type = TT_OPERATOR;
					t->result.data.subsource.source = t->cursor;
					t->result.data.subsource.size = 1;
					advance(t);
					return 1;
				}
			}

			if (is_whitespace(ch)) {
				if (!terminate_ident(t)) {
					// not ident
					advance(t);
					continue;
				}
				advance(t);
				return 1;
			}
			if (!t->parsing_ident) {
				t->begin = t->cursor;
			}
			t->parsing_ident = 1;
			advance(t);

			break;
		case TT_STRING:
			if (t->scope_data.string.str_escaping) t->scope_data.string.str_escaping = 0;
			else if (ch == L'\\') t->scope_data.string.str_escaping = 1;
			else if (ch == L'"') {
				t->result.type = TT_STRING;
				subsource(&t->result.data.subsource, t->begin, t->cursor);
				t->scope = TT_NONE;
				advance(t);
				return 1;
			}
			advance(t);
			break;
		case TT_REAL:
		case TT_INT_DEC:
		case TT_INT_HEX:
		case TT_INT_BIN:
			if (is_whitespace(ch) || ch == L'\n') {
				// k
				alloc_num(t);
				t->scope = TT_NONE;
				advance(t);
				return 1;
			}

			switch (t->scope) {
			case TT_REAL:
				if (!(is_numeric(ch) || ch == L'_')) {
					t->error = ERROR_INVALID_REAL;
					return 1;
				}
			case TT_INT_DEC:
				if (ch == L'.') {
					t->scope = TT_REAL;
				}
				else if (!(is_numeric(ch) || ch == L'_')) {
					t->error = ERROR_INVALID_DEC;
					return 1;
				}
				break;
			case TT_INT_HEX:
				if (!(is_numeric(ch) || is_hex(ch) || ch == L'_')) {
					t->error = ERROR_INVALID_HEX;
					return 1;
				}
				break;
			case TT_INT_BIN:
				if (!(is_bin(ch) || ch == L'_')) {
					t->error = ERROR_INVALID_BIN;
					return 1;
				}
				break;
			}

			advance(t);
			break;

		case TT_STABLE_LINK:
			if (is_whitespace(ch) || ch == L'\n') {
				alloc_num(t);
				t->scope = TT_NONE;
				advance(t);
				return 1;
			}
			advance(t);
			break;

		case TT_FREEZE:
		case TT_COMMENT:
			if (ch == L'\n') {
				t->result.type = t->scope;
				subsource(&t->result.data.subsource, t->begin, t->cursor);
				t->scope = TT_NONE;
				return 1;
			}
			advance(t);
			break;

		case TT_COMMENT_BLOCK:
			if (ch == L'*') {
				if (*(t->cursor + 1) == L'/') {
					// kthx
					t->result.type = TT_COMMENT_BLOCK;
					subsource(&t->result.data.subsource, t->begin, t->cursor);
					t->scope = TT_NONE;
					advance_big(t, 2);
					return 1;
				}
			}
			advance(t);
			break;
		case TT_INDENT:
			if (t->scope_data.indent.dedenting > 0) {
				t->scope_data.indent.dedenting--;
				t->result.type = TT_DEDENT;
				if (t->scope_data.indent.dedenting == 0) t->scope = TT_NONE;
				return 1;
			}
			if (t->indents.size > t->scope_data.indent.current_item) {
				gelc_token_subsource* curr = &t->indents.list[t->scope_data.indent.current_item];
				size_t pos = t->cursor - t->scope_data.indent.indent_begin;

				if (!is_indent(ch)) {
					if (pos > 0) {
						// finding other character while checking inside indent characters ? no
						t->error = ERROR_WRONG_INDENT;
						return 1;
					}
					// non-indent characters and still have pending items ?
					// DEDENT
					t->scope_data.indent.dedenting = t->scope_data.indent.last_level - t->scope_data.indent.current_item;
					continue;
				}

				// cehck each indent characters are same as the source
				if (curr->source[pos] != ch) {
					// wrong indent bruh
					t->error = ERROR_WRONG_INDENT;
					return 1;
				}
				advance(t);
				if (++pos >= curr->size) {
					// next
					t->scope_data.indent.current_item++;
					t->scope_data.indent.indent_begin = t->cursor;
				}
				continue;
			}



			if (is_indent(ch)) {
				t->scope_data.indent.indent_begin = t->cursor;
				advance(t);
			}
			else {
				if (t->scope_data.indent.indent_begin < t->cursor) {
					// add to stack
					if (t->indents.size == t->indents.alloc) {
						t->indents.alloc = (t->indents.size <= 0 ? 1 : t->indents.size * 2);
						t->indents.list = realloc(t->indents.list, t->indents.alloc * sizeof(gelc_token_subsource));
						if (!t->indents.list) {
							t->error = ERROR_OUT_OF_MEM;
							return 1;
						}
					}
					gelc_token_subsource* dyn = &t->indents.list[t->indents.alloc - 1];
					subsource(dyn, t->scope_data.indent.indent_begin, t->cursor);
					t->indents.size++;
				}
				t->scope = TT_NONE;
				if (t->scope_data.indent.last_level != t->indents.size) {
					t->result.type = TT_INDENT;
					return 1;
				}
				else {
					continue;
				}


			}
		}
	}
	return 0;
}