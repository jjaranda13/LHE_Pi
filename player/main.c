#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decoder.h"
#include "entropic_decoder_test.h"

typedef struct player_options {
	char *filename;
	bool fullscreen;
	char * output_f;
} player_options;

int parse_cmd(int argc, char *argv[], player_options * options)
{
    int option_index = 0, c;

	options->filename = "stdin";
	options->fullscreen = false;
	options->output_f = "NULL";
	
    static struct option long_options[] =
    {
        {"fullscreen", no_argument, NULL, 'f'},
        {"input", required_argument, NULL, 'i'},
        {"stdin", no_argument, NULL, 's'},
        {"output", required_argument, NULL, 'o'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long (argc, argv, "fi:so:", long_options, &option_index)) != -1)
    {

        switch (c)
        {
        case 'f':
            options->fullscreen = true;
            break;

        case 'i':
            options->filename = optarg;
            break;

        case 's':
            options->filename = "stdin";
            break;

        case 'o':
            options->output_f = optarg;
            break;
            
        case '?':
            /* getopt_long already printed an error message. */
            return -1;
            break;

        default:
            return -1;
            break;
        }
    }

	return 0;
}

int main(int argc, char *argv[]) {

	player_options options;
	int state;

	if (parse_cmd(argc, argv, &options) != 0) {
		return -1;
	}
	if (strcmp(options.filename, "stdin") == 0) {

		state = decode_stream_stdin(options.fullscreen, options.output_f);
	}
	else {
		state = decode_stream_file(options.fullscreen, options.filename, options.output_f);
	}
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return state;
}


