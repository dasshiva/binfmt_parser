#include <membuf.h>
#include <stdio.h>

#define next_char(buf) ((char)(readU8(buf));

// All the tokentypes returned by the lexer
// Notice that carriage return, tabs and spaces do not have corresponding
// return types. This is because the lexer simply skips over such tokens
// pretending that they do not exist
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
	TYPE_I64             // "i64"
	TYPE_ILEB128         // "ileb128"
	TYPE_NAME,           // Name of a variable declared in a type(), skip, vec 
			     // or generic variable declaration. For ex in 
			     // "<example:u64>", this type represents the string 
			     // "example"
	COLON    = ':',      // value = 58
	COMMA    = ',',      // value = 44
	SEMI_COLON = ";",    // value = 59
	NEWLINE  = 126,      // '\n'
	PAR_OPEN = '(',      // value = 40
	PAR_CLOSE = ')',     // value = 41
	END_OF_FILE = 127    // Returned when the lexer reaches the end of file
};

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


	mclose(buf);
	return 0;
}
