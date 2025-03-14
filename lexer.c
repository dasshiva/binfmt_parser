#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

#define next_char(buf, var) readU8(buf, var);

static int willSkip(char c) {
	switch (c) {
		case '\r':
		case ' ' :
		// We do need newlines to know if a comment has ended
		// but stray newlines do not really matter
		case '\n':
		case ':' :
			return 1;
		default:
			return 0;
	}
}

enum {
	LEXER_EXPECT_OPEN_BRACKET = 1 << 0,
	LEXER_EOF = 1 << 1,
	LEXER_IN_COMMENT = 1 << 2
};

struct Lexer {
	uint32_t line;
	uint32_t col;
	MemBuf*  buf;
	uint32_t flags;
};

struct Token* next(struct Lexer* lex) {
	if (!lex)
		return NULL;
	
	if (lex->flags & LEXER_EOF) // Prevent running the lexer if we are done
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
				lex->flags &= LEXER_IN_COMMENT;
				continue;
			}
			else // Keep skipping if in comment till newline
				continue;
		}

		if (willSkip(c))
			continue;

		if (c == SEMI_COLON) {
			lex->flags |= LEXER_IN_COMMENT;
			continue;
		}

		// If we came here, it means that 'c' is a character
		// of some meaning i.e it is of semantic importance
		// and is something that has to be passed back to
		// our caller

		lex->col++;

	}

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
	ret->col  = 0;
	ret->flags |= LEXER_EXPECT_OPEN_BRACKET;
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
			printf("Token =\'%c\' ", (char)token->type);
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
