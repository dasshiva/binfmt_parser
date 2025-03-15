#include "lexer.h"
#include <membuf.h>

/*
 * program = statement, {statement} |  "" 
 * A program represents a collection of zero or more statements 
 * It is the toplevel entity within the source and everything in
 * one file is part of one program structure
 *
 * To make it easy to move back and forth through a program, it is
 * made into a doubly linked list. This also means you can add or
 * remove "Statement"-s with respect to a given statement at any 
 * point
 */

struct Statement;
typedef struct Program {
	struct Statement* this;
	struct Statement* prev;
	struct Statement* next;
} Program;

/*
 * statement = '<' ident : type '>' | '<' constant : type '>'
 * A statement represents either a definition or an entity of the 
 * program from which code may or may not be generates
 * Note that the ident/constant is called the subject of the statement
 * while the type declaration is called the predicate of the statement
 */

struct Type;
typedef struct Statement {
	struct Token* subject;
	struct Type*  pred;
} Statement;

/* 
 * type = "i8" | "i16" | "i32" | "i64" | "ileb128" "<" number ">" |
 * 	  "u8" | "u16" | "u32" | "u64" | "uleb128" "<" number ">" |
 * 	  "skip" | vecdecl | typedecl | optiondecl
 */

struct Vector;
struct Type;
struct Option;
typedef struct Type {
	union {
		struct Token*  bltin;
		struct Vector* vec;
		struct Type*   type;
		struct Option* opt;
	}

	uint8_t ty;
} Type;

/* 
 * vecdecl  = "vec" "(" contents, {contents} ")"
 * contents = (ident | constant) ":" type
 * Again in the contents declaration, ident | constant is the subject
 * and type is the predicate
 */

struct Content;
typedef struct Vector {
	struct Contents* conts;
	uint32_t len;
} Vector;

typedef struct Content {
	struct Token* subject;
	struct Type*  type;
} Content;


