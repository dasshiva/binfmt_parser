#include <membuf.h>
#include <stdio.h>

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
