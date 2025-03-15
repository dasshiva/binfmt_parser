#ifndef __PARSER_H__
#define __PARSER_H__

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
	struct Statement* cur;
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
struct TypeDecl;
struct Option;
typedef struct Type {
	union {
		struct Token*    bltin;
		struct Vector*   vec;
		struct TypeDecl* type;
		struct Option*   opt;
	}

	uint8_t ty;
} Type;

/* 
 * typedecl  = "type" "(" contents, {contents} ")"
 * contents = (ident | constant) ":" type
 * Again in the contents declaration, ident | constant is the subject
 * and type is the predicate
 */

struct Content;
typedef struct TypeDecl {
	struct Contents* conts;
	uint32_t len;
} Vector;

typedef struct Content {
	struct Token* subject;
	struct Type*  type;
} Content;

/* 
 * vecdecl = "vec" "(" size ":" type ")"
 * size = ident | constant
 */
typedef struct Vector {
  struct Token* subject;
  struct Type*  type;
};

/* 
 * optiondecl  = "optional" "(" contents, {contents} ")"
 * contents = (ident | constant) ":" type | "default" ":" type | 
 *  (ident | constant) ":" "none" | "default" : "none"
 */

struct Options;
typedef struct Option {
  struct Options* opts;
  uint32_t len;
}

typedef struct Options {
  struct Token* subject; // may be an identifier, constant or "default"
  struct Token* pred;    // may be a type or "none"
} Options;

#endif
