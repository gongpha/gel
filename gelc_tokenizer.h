#ifndef GELC_TOKENIZER_H
#define GELC_TOKENIZER_H

#define GELC_CONST_OPERATORS_SINGLE "+-*/^&%!:=.,<>"
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
	TT_STRING, // Unescaped !!!
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
		int integer;
		const wchar_t* operator;
	} data;
} gelc_token;

typedef enum {
	TUNE_INDENT = 0x00000001,

} gelc_tokenizer_tune;

typedef enum {
	PHINT_DEC,
	PHINT_HEX
} gelc_tokenizer_hint;

typedef enum {
	ERROR_OK,

	ERROR_INVALID_REAL,
} gelc_tokenizer_error;


typedef struct {
	const wchar_t* source;
	wchar_t* cursor;
	gelc_token_type scope;

	int parsing_ident;
	unsigned int line;
	wchar_t* begin;
	wchar_t* line_cursor_start;
	int paren_level;
	int str_escaping;
	gelc_tokenizer_hint parsing_hint;

	//
	gelc_tokenizer_error error;
	gelc_token result;

	// tune
	int tune;

} gelc_tokenizer;

void gelc_tokenizer_create(gelc_tokenizer* tokenizer, const wchar_t* source);
int gelc_tokenizer_read(gelc_tokenizer* tokenizer);

#endif