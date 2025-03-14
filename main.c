#include <membuf.h>
#include <stdio.h>
#include "lexer.h"

int main(int argc, const char** argv) {
	if (argc < 2) {
		printf("Usage: %s [FILENAME].syntax\n", argv[0]);
		return 1;
	}

	MemBuf* buf = mopenFromFile(argv[1]);
	if (!buf) {
		printf("Could not open syntax file %s", argv[1]);
		return 1;
	}

	struct Lexer* lexer = newLexer(buf);
	struct Token* t = next(lexer);
	int errno = 0;
	while (1) {
		// Syntax error detected
		// We can differentiate between EOF's and errors
		// because for syntax errors, the lexer first returns NULL 
		// without any previous indication
		// For EOF, the lexer first returns a Token with type = 
		// END_OF_FILE and THEN returns NULL
		if (!t) {
			errno = 1;
			break;
		}

		if (t->type == END_OF_FILE)
			break;
		dumpToken(t);
    freeToken(t);
		t = next(lexer);
	}

	freeToken(t);
	destroyLexer(lexer);
	return errno;
}
