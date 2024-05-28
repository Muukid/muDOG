#include "file.h"

const char* default_output_name = "README.md";

int main(int argc, char* argv[]) {
	char* chosen_output_name = (char*)default_output_name;

	if (argc == 1) {
		printf("[muDOG] Too few arguments supplied; exiting.\n");
		return -1;
	}

	/* Create output file */

		File of = file_open(default_output_name, "wb");
		if (of.f == NULL) {
			printf("[muDOG] Failed to create file; exiting.\n");
			return -1;
		}

	/* Process each file */

		for (int i = 1; i < argc; i++) {
			if (argv[i][0] == '-') {
				// -... command
				switch (argv[i][1]) {

					default: {
						printf("[muDOG] Unrecognized flag '-%c'; exiting.\n", argv[i][1]);
						file_close(of);
						return -1;
					} break;

					case 'o': {
						// New output filename
						if (i+1 >= argc) {
							printf("[muDOG] Expected another argument after '-o'; exiting.\n");
							file_close(of);
							return -1;
						}
						chosen_output_name = argv[i+1];
						i++;
					} break;
				}
			} else {
				// File to process
				File in_file = file_open(argv[i], "rb");
				if (in_file.f == NULL) {
					printf("[muDOG] Failed to open file \"%s\"; exiting.\n", argv[i]);
					return -1;
				}

				if (file_process(in_file, of) != 0) {
					return -1;
				}

				file_close(in_file);
			}
		}

	/* Close everything */

		file_close(of);

	/* Rename output file to be what the user wanted */

		remove(chosen_output_name);
		rename(default_output_name, chosen_output_name);

	return 0;
}
