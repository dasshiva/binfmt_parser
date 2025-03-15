#ifndef __LEXER_H__
#define __LEXER_H__

#include <stdint.h>
#include <membuf.h>
// All the tokentypes returned by the lexer
// Notice that carriage return, tabs and spaces do not have corresponding
// enum types. This is because the lexer simply skips over such tokens
// pretending that they do not exist.
// However new lines are recognised because they are used to find where
// single line comments end

enum TokenType {
	OPEN_BRACKET  = '<', // value = 60
	CLOSE_BRACKET = '>', // value = 63
	TYPE_U8       =  0,  // "u8"
	TYPE_U16,            // "u16"
	TYPE_U32,            // "u32"
	TYPE_U64,            // "u64"
	TYPE_ULEB128,        // "uleb128"
	TYPE_I8,             // "i8"
	TYPE_I16,            // "i16"
	TYPE_I32,            // "i32"
	TYPE_I64,            // "i64"
	TYPE_ILEB128,        // "ileb128"
	TYPE_OPTIONAL,       // "optional"
	TYPE_VEC,            // "vec"
	TYPE_SKIP,           // "skip"
	TYPE_DECL,           // "type"
	TYPE_NAME,           // Name of a variable declared in a type(), skip, vec 
			                 // or generic variable declaration. For ex in 
			                 // "<example:u64>", this type represents the string 
			                 // "example"
  LITERAL_DEFAULT,     // "default"
  LITERAL_NONE,        // "none"
	TYPE_CONSTANT,       // A numeric constant such as 0xCAFEBABE
			     // always treated as 64-bit unsigned integer
	COLON    = ':',      // value = 58
	COMMA    = ',',      // value = 44
	SEMI_COLON = ';',    // value = 59
	NEWLINE  = 126,      // '\n'
	PAR_OPEN = '(',      // value = 40
	PAR_CLOSE = ')',     // value = 41
	MINUS    = '-',      // value = 45
	END_OF_FILE = 127    // Returned when the lexer reaches the end of file
};

typedef struct Token {
	union {
		char*    name;
	    	uint64_t u64;	
	};

	enum TokenType type;
	uint32_t  line, col;
} Token;

struct Lexer;
struct Lexer* newLexer(MemBuf*);
// Make sure to free the token claimed from here by calling freeToken()
struct Token* next(struct Lexer* lexer);
void dumpToken(struct Token* token);
void destroyLexer(struct Lexer* lexer);
void freeToken(struct Token* token);

#endif
