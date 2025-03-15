#include "lexer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define next_char(buf, var) readU8(buf, var);

// they are so arranged such that their indexes
// equal the TYPE_* enum values
static const char* builtinTypes[] = {
  "u8", "u16", "u32", "u64", "uleb128", // the unsigned types
  "i8", "i16", "i32", "i64", "ileb128", // the signed types
  "optional", "vec", "skip", "type"     // the composite types
};

static int findIfBuiltin(const char* s) {
  // FIXME: This is inefficient
  for (int i = 0; i < 14; i++) {
    if (strcmp(builtinTypes[i], s) == 0) 
      return i;
  }

  return -1;
}

static int willSkip(char c) {
  switch (c) {
  case '\r':
  case ' ':
  // We do need newlines to know if a comment has ended
  // but stray newlines do not really matter
  case '\n':
    return 1;
  default:
    return 0;
  }
}

enum {
  LEXER_EOF = 1 << 0,        // Lexer has hit the end of file
  LEXER_IN_COMMENT = 1 << 1, // We are processing a comment
  LEXER_IN_ERROR = 1 << 2    // We have encountered some kind of error
};

struct Lexer {
  uint32_t line;
  uint32_t col;
  MemBuf *buf;
  uint32_t flags;
};

void lexerError(struct Lexer *lex, const char *fmt, ...) {
  lex->flags |= LEXER_IN_ERROR;
  va_list ap;
  va_start(ap, fmt);
  printf("Error at line %u, column %u : ", lex->line, lex->col);
  vprintf(fmt, ap);
  printf("\nAborting now\n");
  va_end(ap);
}

static inline int isDigit(char c) { return (c >= '0') && (c <= '9'); }

static inline int isLetter(char c) {
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

static inline int isHexDigit(char c) {
  return isDigit(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

static struct Token *makeToken(struct Lexer *lexer, enum TokenType ty) {
  struct Token *token = malloc(sizeof(struct Token));
  if (!token) {
    lexerError(lexer, "Out of memory");
    return NULL;
  }

  token->line = lexer->line;
  token->col = lexer->col;
  token->type = ty;
  return token;
}

int lexConstant(struct Lexer* lex, uint64_t* dest, int base) {
  // If base is 0, we have to figure it out ourselves
  // Otherwise just use the given base
  char c = 0;
  uint64_t status = 0;
  if (!base) {
    status = next_char(lex->buf, &c);
    lex->col++; // We picked up another character
    if (status == UINT64_MAX) {
      lexerError(lex, "Unexpected end of file while reading constant");
      return 0;
    }

    if (isDigit(c)) 
      base = 10;
    else {

      switch(c) {
        case 'b': base = 2; break;
        case 'x': case 'X': base = 16; break;
        default:
                lexerError(lex, "%c is not a digit or base specifier", c);
                return 0;
      }
    }
  }

  // Now we have the required base
  // base can have three values = 2, 10, 16
  // If base is 2, there can be at most 64 digits, if base is 10, there can
  // be at most 20 digits and if base is 16 there can be at most 16 digits

  if (base == 2) {
    char* num = calloc(64, sizeof(char));
    for (int i = 0; i < 64; i++) {
      status = next_char(lex->buf, &c);
      lex->col++;
      if (status == UINT64_MAX) {
        lexerError(lex, "Unexpected end of file while reading constant");
        return 0;
      }

      if (!isDigit(c)) {
        // Put 'c' back
        mseek(lex->buf, MEMBUF_CURRENT, -1);
        break;
      }

      if (c != '0' && c != '1') {
        lexerError(lex, "%c cannot appear in binary constant", c);
        return 0;
      }

      num[i] = c;
    }

    *dest = strtoull(num, NULL, 2);
    free(num);
  }

  else if (base == 10) {
    char* num = calloc(20, sizeof(char));
    for (int i = 0; i < 20; i++) {
      status = next_char(lex->buf, &c);
      lex->col++;
      if (status == UINT64_MAX) {
        lexerError(lex, "Unexpected end of file while reading constant");
        return 0;
      }

      if (!isDigit(c)) {
        // Put 'c' back
        mseek(lex->buf, MEMBUF_CURRENT, -1);
        break;
      }

      num[i] = c;
    }

    *dest = strtoull(num, NULL, 10);
    free(num);
  }

  else if (base == 16) {
    char* num = calloc(16, sizeof(char));
    for (int i = 0; i < 16; i++) {
      status = next_char(lex->buf, &c);
      lex->col++;
      if (status == UINT64_MAX) {
        lexerError(lex, "Unexpected end of file while reading constant");
        return 0;
      }

      if (!isHexDigit(c)) {
        // Put 'c' back
        mseek(lex->buf, MEMBUF_CURRENT, -1);
        break;
      }

      num[i] = c;
    }

    *dest = strtoull(num, NULL, 16);
    free(num);

  }

  else {
    lexerError(lex, "Internal error - Unknown base %d", base);
    return 0;
  }

  return 1;
}

int lexName(struct Lexer *lex, char *name) {
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

struct Token *next(struct Lexer *lex) {
  if (!lex)
    return NULL;

  // Prevent running the lexer if we are done
  if (lex->flags & LEXER_EOF)
    return NULL;

  // We have encountered a lexing error, don't process any more tokens
  if (lex->flags & LEXER_IN_ERROR)
    return NULL;

  // TODO: Implement lexer
  struct Token *ret = NULL;
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
      } else { // Keep skipping if in comment till newline
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

    // If we came here, it means that 'c' is a character of some meaning i.e 
    // it is of semantic importance and is something that has to be passed 
    // back to our caller

    switch (c) {
    case COLON:
    case PAR_OPEN:
    case OPEN_BRACKET:
    case CLOSE_BRACKET:
    case PAR_CLOSE:
    case MINUS: {
      ret = makeToken(lex, c);
      lex->col++;
      return ret;
    }

    default:
      if (isLetter(c)) {
        ret = makeToken(lex, TYPE_NAME);
        if (!ret)
          return NULL;
        // Allocate a fixed size (maybe more than needed) as identifiers cannot
        // be longer than 20 bytes
        ret->name = calloc(20, sizeof(char));
        if (!ret->name) {
          free(ret);
          lexerError(lex, "Out of memory");
          return NULL;
        }

        ret->name[0] = c;
        lex->col++; // We picked up 'c'

        // lexName can pick up a maximum of 19 characters only as one has 
        // already been picked up by us

        // If lexName failed it encountered an unexpected end of file
        if (!lexName(lex, ret->name)) {
          lexerError(lex, "Unexpected end of file");
          free(ret->name);
          free(ret);
          return NULL;
        }

        int bltin = findIfBuiltin(ret->name);
        if (bltin != -1) {
          ret->type = bltin;
          free(ret->name);
        }

        return ret;
      }

      else if (isDigit(c)) {
        // All constants are 64 bit unsigned integers. We support three modes
        // of specifying integers: base 2 (binary), base 10 (decimal) and
        // base 16(hexadecimal)

        // Base 2 integers are specified using the suffix '0b', base 16 
        // integers are suffixed with '0x'. Decimal integers are the 
        // default when no suffix is present

        ret = makeToken(lex, TYPE_CONSTANT);
        if (!ret)
          return NULL;

        if (c == '0') { 
          // Leading zeroes are allowed in decimal numbers
          // But since leading zeroes do not affect the value of the 
          // decimal number it is safe to skip them

          // Many different kinds of errors are possible here
          // so let lexConstant deal with the exact error
          lex->col++; // we picked up 'c'
          if (!lexConstant(lex, &ret->u64, 0)) {
            free(ret);
            return NULL;
          }
        }

        else { // Must be decimal
          // put 'c' back into the stream
          mseek(lex->buf, MEMBUF_CURRENT, -1);
          if (!lexConstant(lex, &ret->u64, 10)) {
            free(ret);
            return NULL;
          }
        }

        return ret;
      }

      else {
        lexerError(lex, "Unknown character %c in source", c);
        return NULL;
      }

      break;
    }

    lex->col++;
  }

  // We come here when we reach EOF the first time.  Inform the caller 
  // about this. Next time we are called in this state, 
  // we will return NULL

  ret = malloc(sizeof(struct Token));
  ret->type = END_OF_FILE;
  ret->name = NULL;
  return ret;
}

struct Lexer *newLexer(MemBuf *buf) {
  if (!buf)
    return NULL;

  struct Lexer *ret = malloc(sizeof(struct Lexer));
  if (!ret)
    return NULL;

  ret->buf = buf;
  ret->line = 1;
  ret->col = 1;
  ret->flags = 0;
  return ret;
}

void destroyLexer(struct Lexer *lexer) {
  if (!lexer)
    return;
  mclose(lexer->buf);
  free(lexer);
}

const char *token2string(Token *t) {
  switch (t->type) {
  case OPEN_BRACKET:
    return "<";
  case CLOSE_BRACKET:
    return ">";
  case COLON:
    return ":";
  case COMMA:
    return ",";
  case SEMI_COLON:
    return ";";
  case PAR_OPEN:
    return "(";
  case PAR_CLOSE:
    return ")";
  case MINUS:
    return "-";
  case TYPE_I8:
    return "Type(i8)";
  case TYPE_I16:
    return "TYpe(i16)";
  case TYPE_I32:
    return "Type(i32)";
  case TYPE_I64:
    return "Type(i64)";
  case TYPE_ILEB128:
    return "Type(ileb128)";
  case TYPE_U8:
    return "Type(u8)";
  case TYPE_U16:
    return "Type(u16)";
  case TYPE_U32:
    return "Type(u32)";
  case TYPE_U64:
    return "TYpe(u64)";
  case TYPE_ULEB128:
    return "Type(uleb128)";
  case TYPE_DECL:
    return "TypeDecl(type)";
  case TYPE_VEC:
    return "Type(vec)";
  case TYPE_SKIP:
    return "TYpe(skip)";
  case TYPE_CONSTANT:
    return "constant";
  case TYPE_NAME: 
    return t->name;
  case END_OF_FILE:
    return "EOF";
  default:
    return "(Unreachable)";
  }
}

void dumpToken(struct Token *token) {
  printf("{ Location = %u:%u ", token->line, token->col);
  printf("Token = %s ", token2string(token));
  printf("}\n");
}

void freeToken(struct Token *token) {
  // Only free the token structure itself not token->name token->name may be 
  // passed on to other structures in the parser and freeing this here would 
  // mean we have to strcpy() the name to a new allocated buffer which is not
  // very efficient and definitely more costly than just using the same 
  // string everywhere until we can have more efficient alternatives 
  // (maybe hashing the strings?)
  free(token);
}
