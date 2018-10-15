#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "video_encoder.h"

int parse_cmd(int argc, char *argv[]);

int main(int argc, char* argv[])
{
    int status;

    status = parse_cmd(argc, argv);
    if (status != 0)
    {
        return -1;
    }
    VideoSimulation();
	return 0;
}

int parse_cmd(int argc, char *argv[])
{

    is_rtp = false;
    pppx = 2;
    pppy = 2;

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
	}
	fprintf(stderr,"rtp=%d pppx=%d pppy=%d",is_rtp, pppx, pppy);
	return 0;
}
