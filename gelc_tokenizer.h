#ifndef GELC_TOKENIZER_H
#define GELC_TOKENIZER_H

#define GELC_CONST_OPERATORS_SINGLE = "+-*/^&%!:=.,<>"
#define GELC_CONST_OPERATORS {"=>", "<=", ">=", "==", "!=", "..."}

#include <stddef.h>


typedef struct {
	wchar_t* source;
	unsigned int size;
} gelc_token_literal;

typedef enum {
	TT_NONE,
	TT_COMMENT,
	TT_IDENT,
	TT_STRING,
	TT_INT,
	TT_REAL,
	TT_OPERATOR,
	TT_STABLE_LINK,
	TT_NEWLINE,
	TT_PAREN,
	TT_PAREN_BL,
	TT_PAREN_BR,
	TT_INDENT,
	TT_DEDENT
} gelc_token_type;

typedef struct {
	gelc_token_type type;
	union {
		gelc_token_literal literal;
	} data;
	unsigned int line;
	unsigned int pos;
} gelc_token;

typedef enum {
	TUNE_INDENT = 0x00000001,

} gelc_tokenizer_tune;


typedef struct {
	const wchar_t* source;
	wchar_t* cursor;
	gelc_token_type scope;

	int parsing_ident;
	unsigned int line;
	wchar_t* begin;
	wchar_t* line_cursor_start;

	//
	gelc_token result;

	// tune
	int tune;

} gelc_tokenizer;

void gelc_tokenizer_create(gelc_tokenizer* tokenizer, const wchar_t* source);
int gelc_tokenizer_read(gelc_tokenizer* tokenizer);

#endif