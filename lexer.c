#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define next_char(buf, var) readU8(buf, var);

static int willSkip(char c) {
	switch (c) {
		case '\r':
		case ' ' :
		// We do need newlines to know if a comment has ended
		// but stray newlines do not really matter
		case '\n':
			return 1;
		default:
			return 0;
	}
}

enum {
	LEXER_EOF = 1 << 0,          // Lexer has hit the end of file
	LEXER_IN_COMMENT = 1 << 1,   // We are processing a comment
	LEXER_IN_ERROR = 1 << 2     // We have encountered some kind of error
};

struct Lexer {
	uint32_t line;
	uint32_t col;
	MemBuf*  buf;
	uint32_t flags;
};

void lexerError(struct Lexer* lex, const char* fmt, ...) {
	lex->flags |= LEXER_IN_ERROR;
	va_list ap;
	va_start(ap, fmt);
	printf("Error at line %u, column %u : ", lex->line, lex->col);
	vprintf(fmt, ap);
	printf("\nAborting now\n");
	va_end(ap);
}

static inline int isDigit(char c) {
	return (c >= '0') && (c <= '9');
}

static inline int isLetter(char c) {
	return ((c >= 'a') && (c <= 'z')) ||
		((c >= 'A') && (c <= 'Z'));
}

static struct Token* makeToken(struct Lexer* lexer, enum TokenType ty) {
	struct Token* token = malloc(sizeof(struct Token));
	if (!token) {
		lexerError(lexer, "Out of memory");
		return NULL;
	}

	token->line = lexer->line;
	token->col  = lexer->col;
	token->type = ty;
	return token;
}

int lexName(struct Lexer* lex, char* name) {
	// DO NOT touch name[0], it is already initialised
	char c = 0;
	uint64_t status = 0;
	for (int i = 1; i < 20; i++) {
		status = next_char(lex->buf, &c);
		if (status == UINT64_MAX) {
			lex->flags |= LEXER_EOF;
			return 0;
		}

		if (isDigit(c) || isLetter(c))
			name[i] = c;
		else {
			// Put back character into stream
			mseek(lex->buf, MEMBUF_CURRENT, -1);
			return 1;
		}

		lex->col++;
	}

	return 1;
}

struct Token* next(struct Lexer* lex) {
	if (!lex)
		return NULL;
	
	// Prevent running the lexer if we are done
	if (lex->flags & LEXER_EOF) 		
		return NULL;

	// We have encountered a lexing error, don't process any more tokens
	if (lex->flags & LEXER_IN_ERROR)
		return NULL;

	// TODO: Implement lexer
	struct Token* ret = NULL;
	while (1) {
		char c = 0;
		uint64_t status = next_char(lex->buf, &c);
		if (status == UINT64_MAX) {
			lex->flags |= LEXER_EOF;
			break;
		}

		// detect newline before willSkip() jumps over it
		if (c == '\n') {
			lex->line++;
			lex->col = 0;
		}

		if (lex->flags & LEXER_IN_COMMENT) {
			if (c == '\n') { // Found newline start lexing now
				lex->flags &= ~LEXER_IN_COMMENT;
				continue;
			}
			else { // Keep skipping if in comment till newline
			        lex->col++;
				continue;
			}
		}

		if (willSkip(c)) {
			lex->col++;
			continue;
		}

		if (c == SEMI_COLON) {
			lex->col++;
			lex->flags |= LEXER_IN_COMMENT;
			continue;
		}

		// If we came here, it means that 'c' is a character
		// of some meaning i.e it is of semantic importance
		// and is something that has to be passed back to
		// our caller

		switch (c) {
			case COLON:
			case PAR_OPEN:
			case OPEN_BRACKET:
			case CLOSE_BRACKET:
			case PAR_CLOSE:
			case MINUS: {
				ret = makeToken(lex, c);
				lex->col++;
				break;
			}

			default:
				lexerError(lex, "Unimplemented support for %c", c);
				break;
		};
	}
		/*	if (c == '<') {
				lex->flags |= LEXER_EXPECT_ENTITY;
				lex->flags &= ~LEXER_EXPECT_OPEN_BRACKET;
				ret = makeToken(lex, OPEN_BRACKET);
				lex->col++;
				return ret;
			}

			else { 
				lexerError(lex, "Expected '<' but got '%c'", c);
				return NULL;
			}

		if (lex->flags & LEXER_EXPECT_ENTITY) {
			if (isLetter(c)) {
				ret = makeToken(lex, TYPE_NAME);
				if (!ret)
					return NULL;

				// Allocate a fixed size (maybe more than
				// needed) as identifiers cannot be 
				// longer than 20 bytes

				ret->name = calloc(20, sizeof(char));
				if (!ret->name) {
					free(ret);
					lexerError(lex, "Out of memory");
					return NULL;
				}

				ret->name[0] = c;
				lex->col++; // We picked up 'c'

				// lexName can pick up a maximum of 19
				// characters only as one has already been
				// picked up by us

				// If lexName failed it encountered an 
				// unexpected end of file
				if (!lexName(lex, ret->name)) {
					lexerError(lex, "Unexpected end of file");
					free(ret->name);
					free(ret);
					return NULL;
				}

				return ret;
			}

			else if (isDigit(c)) {
				// Pick up a constant
			}

			else {
				// Whatever 'c' is, it cannot be a part of an 
				// identifier or a constant 
				lexerError(lex, "Expected identifier or constant but '%c'" 
						" cannot be in an identifier or constant", c);
				return NULL;
			}
		}

		lex->col++;
	} */

	// We come here when we reach EOF the first time 
	// Inform the caller about this
	// Next time we are called in this state, we will return NULL
	
	ret = malloc(sizeof(struct Token));
	ret->type = END_OF_FILE;
	ret->name = NULL;
	return ret;
}

struct Lexer* newLexer(MemBuf* buf) {
	if (!buf)
		return NULL;

	struct Lexer* ret = malloc(sizeof(struct Lexer));
	if (!ret)
		return NULL;

	ret->buf = buf;
	ret->line = 1;
	ret->col  = 1;
	ret->flags |= 0;
	return ret;
}

void destroyLexer(struct Lexer* lexer) {
	if (!lexer)
		return;
	mclose(lexer->buf);
	free(lexer);
}

void dumpToken(struct Token* token) {
	printf("{ Location = %u:%u ", token->line, token->col);
	switch (token->type) {
		case OPEN_BRACKET:
		case CLOSE_BRACKET:
		case COLON:
		case COMMA:
		case SEMI_COLON:
		case PAR_OPEN:
		case PAR_CLOSE:
		case MINUS: {
			printf(" Token =\'%c\' ", (char)token->type);
			break;
		}

		case TYPE_I8: {
			printf(" Type = \"i8\" ");
			break;	
		}
			      
		case TYPE_I16: {
			printf(" Type = \"i16\" ");
			break;	
		}

		case TYPE_I32: {
			printf(" Type = \"i32\" ");
			break;	
		}

		case TYPE_I64: {
			printf(" Type = \"i64\" ");
			break;	
		}

		case TYPE_ILEB128: {
			printf(" Type = \"ileb128\" ");
			break;	
		}

		case TYPE_U8: {
			printf(" Type = \"u8\" ");
			break;	
		}

		case TYPE_U16: {
			printf(" Type = \"u16\" ");
			break;
		}

		case TYPE_U32: {
			printf(" Type = \"u32\" ");
			break;	
		}

		case TYPE_U64: {
			printf(" Type = \"u64\" ");
			break;	
		}

		case TYPE_ULEB128: {
			printf(" Type = \"uleb128\" ");
			break;	
		}

		case TYPE_VEC: {
			printf(" Type = \"vec\" ");
			break;	
		}

		case TYPE_SKIP: {
			printf(" Type = \"skip\" ");
			break;	
		}

		case TYPE_DECL: {
			printf(" Declaration = \"type\" ");
			break;	
		}


		case TYPE_CONSTANT: {
			printf(" Constant = %lu ", token->u64);
			break;
		}

		case TYPE_NAME: {
			printf(" Name = %s ", token->name);
			break;
		}

		case END_OF_FILE: { break; }
		default: {
			printf("Unrecognised token type = %u", token->type);
			break;
		}
		
	};

	printf("}\n");
}

void freeToken(struct Token* token) {
	// Only free the token structure itself not token->name
	// token->name may be passed on to other structures in
	// the parser and freeing this here would mean we have to
	// strcpy() the name to a new allocated buffer which is not
	// very efficient and definitely more costly than just using
	// the same string everywhere until we can have more efficient
	// alternatives (maybe hashing the strings??)
	free(token);
}
