#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"
#include "entropic_decoder_test.h"

typedef struct player_options {
	int width;
	int height;
	char *filename;
	bool fullscreen;
} player_options;

int parse_cmd(int argc, char *argv[], player_options * options) {

	options->filename = "NULL";
	options->height = -1;
	options->width = -1;
	options->fullscreen = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-w") == 0) {
			options->width = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--height") == 0 || strcmp(argv[i], "-h") == 0) {
			options->height = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--input") == 0 || strcmp(argv[i], "-i") == 0) {
			options->filename = argv[i + 1];

		}
		else if (strcmp(argv[i], "--stdin") == 0 || strcmp(argv[i], "-s") == 0) {
			options->filename = "stdin";

		}
        else if (strcmp(argv[i], "--fullscreen") == 0 || strcmp(argv[i], "-f") == 0) {
			options->fullscreen = true;

		}
	}
	if (strcmp(options->filename, "NULL") == 0) {
		printf("Supply a filename or stdin.\n");
		return -1;
	}

	if (options->width == -1) {
		printf("Width of the representation must be supplied\n");
		return -1;
	}
	if (options->height == -1) {
		printf("Height of the representation must be supplied\n");
		return -1;
	}
	return 0;
}

int main(int argc, char *argv[]) {

	player_options options;
	int state;

	if (parse_cmd(argc, argv, &options) != 0) {
		printf("Press Enter to Continue");
		while (getchar() != '\n');
		return -1;

	}
	if (strcmp(options.filename, "stdin") == 0) {
		state = decode_stream_stdin(options.width, options.height, options.fullscreen);
	}
	else {
		state = decode_stream_file(options.width, options.height, options.fullscreen, options.filename);
	}
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return state;
}


