#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "frame_encoder.h"
#include "globals.h"
#include "video_encoder.h"

int parse_cmd(int argc, char *argv[], char ** filename);

int main(int argc, char* argv[])
{
    int status;
    char * filename;

    status = parse_cmd(argc, argv, &filename);

    if (status == 0)
    {
        video_recording();
    }
    else if (status == 1)
    {
        encode_file(filename);
    }
    else if (status == 2)
    {
        encode_video_from_file_sequence(filename, sequence_length);
    }
	return 0;
}

int parse_cmd(int argc, char *argv[], char ** filename)
{
    char * delimiter = NULL;
    int sequence_last = 0;

    is_rtp = false;
    sequence_length = 0;
    sequence_init = 0;
    pppx = 2;
    pppy = 2;
    *filename = "NULL";

	for (int i = 0; i < argc; i++)
	{
        if (strcmp(argv[i], "-rtp") == 0)
        {
            is_rtp = true;
        }
        else if (strcmp(argv[i], "-pppx") == 0)
        {
            pppx = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-pppy") == 0)
        {
            pppy = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0 )
        {
            *filename = argv[i + 1];
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sequence") == 0 )
        {
            delimiter = strtok( argv[i + 1], ":" );
            if (delimiter == NULL)
            {
                return -1;
            }
            sequence_init = atoi(delimiter);
            delimiter = strtok( NULL, ":" );
            if (delimiter == NULL)
            {
                return -1;
            }
            sequence_length = atoi(delimiter);
        }
	}

	if (strcmp(*filename, "NULL") == 0)
	{
        return 0;
	}
	else if (sequence_length == 0)
	{
        return 1;
    }
    else
    {
        return 2;
    }
}
