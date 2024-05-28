#include "file.h"

/* Basic file operations */

	extern File file_open(const char* filename, const char* format) {
		File file;
		file.f = fopen(filename, format);
		if (file.f == NULL) {
			return file;
		}

		fseek(file.f, 0L, SEEK_END);
		file.length = ftell(file.f);
		fseek(file.f, 0L, SEEK_SET);

		return file;
	}

	extern char file_next_char(File file) {
		return (char)fgetc(file.f);
	}

	extern void file_write_char(File file, char c) {
		fputc(c, file.f);
	}

	extern void file_close(File file) {
		fclose(file.f);
	}

	long file_get_index(File file) {
		return ftell(file.f);
	}

	void file_set_index(File file, long index) {
		fseek(file.f, index, SEEK_SET);
	}

/* File processing */

	/* Command identification */

		#define COMMAND_COUNT 8
		const char* commandTypes[COMMAND_COUNT] = {
			#define DOCBEGIN 0
			"DOCBEGIN",
			#define DOCEND 1
			"DOCEND",
			#define NL 2
			"NL",
			#define NLNT 3
			"NLNT",
			#define NLFT 4
			"NLFT",
			#define DOCLINE 5
			"DOCLINE",
			#define IGNORE 6
			"IGNORE",
			#define ATTENTION 7
			"ATTENTION",
		};

		int any_valid(int* valid, int len) {
			for (int i = 0; i < len; i++) {
				if (valid[i] == 0) {
					return 1;
				}
			}
			return 0;
		}
		#define is_char(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z') || c == '_' || (c >= '0' && c <= '9'))
		int identify_command(File f) {
			int valid[COMMAND_COUNT] = {0};
			int i = 0;

			char c = file_next_char(f);
			while (is_char(c) && any_valid(valid, COMMAND_COUNT)) {
				for (int j = 0; j < COMMAND_COUNT; j++) {
					if (valid[j] == 0) {
						if (commandTypes[j][i] == '\0') {
							valid[j] = 1;
						} else if (commandTypes[j][i] != c) {
							valid[j] = 1;
						}
					}
				}

				c = file_next_char(f);
				i++;
			}

			for (int j = 0; j < COMMAND_COUNT; j++) {
				if (valid[j] == 0 && commandTypes[j][i] == '\0') {
					return j;
				}
			}

			return -1;
		}

	/* Command processing */

		struct processingInfo {
			int ignore_state;
			size_t ignore_count;
			size_t attention_count;
		};
		typedef struct processingInfo processingInfo;

		void process_command(int command, File input, File output, processingInfo* pi) {
			switch (command) {
				default: break;

				case DOCBEGIN: case DOCLINE: {
					if (pi->attention_count < pi->ignore_count) return;

					if (command == DOCLINE) {
						file_write_char(output, '\n');
					}

					int in_command = -1;
					char prev_c = 1;
					while (in_command != DOCEND && prev_c != '\0') {
						char c = file_next_char(input);

						if (c == '\n' && command == DOCLINE) {
							file_write_char(output, '\n');
							return;
						}

						if (c == '@') {

							in_command = identify_command(input);
							if (in_command != -1) {
								process_command(in_command, input, output, pi);
							}

						} else {
							// (Prevention for unneccessary beginning tab characters)
							if (prev_c == '\n') {
								pi->ignore_state = 1;
							}
							if (pi->ignore_state && c != '\t') {
								pi->ignore_state = 0;
							}

							if (!pi->ignore_state) {
								file_write_char(output, c);
							}
						}

						prev_c = c;
					}
				} break;

				case DOCEND: {
					if (pi->attention_count < pi->ignore_count) return;
					// ...
				} break;

				case NL: case NLNT: {
					if (pi->attention_count < pi->ignore_count) return;

					file_write_char(output, '\n');
					file_write_char(output, '\n');
					for (int i = 0; i < 3; i++) {
						file_write_char(output, '`');
					}
					file_write_char(output, 'c');
					file_write_char(output, '\n');

					long index = file_get_index(input);
					char c = file_next_char(input);

					while (c != '\n') {
						c = file_next_char(input);
					}
					c = file_next_char(input);

					if (command == NLNT) {
						while (c == '\t') {
							c = file_next_char(input);
						}
					}

					while (c != '\n') {
						file_write_char(output, c);
						c = file_next_char(input);
					}

					file_write_char(output, '\n');
					for (int i = 0; i < 3; i++) {
						file_write_char(output, '`');
					}
					file_write_char(output, '\n');

					file_set_index(input, index-1);
				} break;

				case NLFT: {
					if (pi->attention_count < pi->ignore_count) return;

					long index = file_get_index(input);
					char c = file_next_char(input);

					while (c != '\n') {
						c = file_next_char(input);
					}
					c = file_next_char(input);

					while (c == '\t' || c == ' ') {
						c = file_next_char(input);
					}

					if (is_char(c)) {
						while (is_char(c)) {
							file_write_char(output, c);
							c = file_next_char(input);
						}
					} else {
						file_write_char(output, c);
					}

					file_set_index(input, index-1);
				} break;

				case IGNORE: {
					pi->ignore_count += 1;
				} break;

				case ATTENTION: {
					pi->attention_count += 1;
				} break;
			}
		}

	/* Actual processing */

		extern int file_process(File input, File output) {
			processingInfo pi;
			pi.ignore_state = 0;
			pi.ignore_count = 0;
			pi.attention_count = 0;

			for (size_t i = 0; i < input.length; i++) {
				char c = file_next_char(input);
				switch (c) {
					default: break;
					case '@': {
						int command = identify_command(input);
						if (command != -1) {
							process_command(command, input, output, &pi);
						}
					} break;
				}
			}
			return 0;
		}