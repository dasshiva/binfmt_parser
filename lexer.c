#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>

#define next_char(buf, var) ((char)(readU8(buf, var)));

enum {
	LEXER_EXPECT_OPEN_BRACKET = 1 << 0,
	LEXER_EOF = 1 << 1
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
	// TODO: Implement lexer
	struct Token* ret = NULL;
	while (1) {
		char c = 0;
		uint64_t status = next_char(lex->buf, &c);
		if (status == UINT64_MAX) {
			lex->flags |= LEXER_EOF;
			break;
		}
	}
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

		case NEWLINE:
		case END_OF_FILE: { break; }
		default: {
			printf("Unrecognised token type = %u", token->type);
			break;
		}
		
	};

	printf("}\n");
}

