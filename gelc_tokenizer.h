#ifndef GELC_TOKENIZER_H
#define GELC_TOKENIZER_H

#define GELC_CONST_OPERATORS_SINGLE L"+-*/^&%!:=.,<>"
#define GELC_CONST_OPERATORS {L"=>", L"<=", L">=", L"==", L"!=", L"..."}

#include <stddef.h>


typedef struct {
	const wchar_t* source;
	unsigned int size;
} gelc_token_subsource;

typedef enum {
	TT_NONE,
	TT_COMMENT,
	TT_IDENT,
	TT_STRING, // Unescaped !!!
	TT_INT,
	TT_INT_HEX,
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
		gelc_token_subsource subsource;
		int integer;
		double real;
	} data;
} gelc_token;

typedef enum {
	TUNE_INDENT = 0x00000001,

} gelc_tokenizer_tune;

typedef enum {
	ERROR_OK,

	ERROR_INVALID_REAL,
	ERROR_INVALID_HEX,
	ERROR_OUT_OF_MEM,
	ERROR_WRONG_INDENT,
} gelc_tokenizer_error;


typedef struct {
	const wchar_t* source;
	wchar_t* cursor;
	gelc_token_type scope;

	int parsing_ident;
	unsigned int line;
	// pos : cursor - line_cursor_start
	const wchar_t* begin;
	const wchar_t* line_cursor_start;
	int paren_level;
	int str_escaping;
	struct {
		char* list;
		size_t alloc;
		size_t size;
	} indents;
	int bypass_indent;
	int new_indent;
	unsigned int indent;

	//
	gelc_tokenizer_error error;
	gelc_token result;

	// tune
	int tune;

} gelc_tokenizer;

void gelc_tokenizer_create(gelc_tokenizer* tokenizer, const wchar_t* source);
void gelc_tokenizer_destroy(const gelc_tokenizer* tokenizer);
int gelc_tokenizer_read(gelc_tokenizer* tokenizer);

#endif