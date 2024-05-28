#include <stdio.h>

struct File {
	FILE* f;
	size_t length;
};
typedef struct File File;

/* Basic file operations */

	extern File file_open(const char* filename, const char* format);

	extern char file_next_char(File file);

	extern void file_write_char(File file, char c);

	extern void file_close(File file);

/* File processing */

	extern int file_process(File input, File output);