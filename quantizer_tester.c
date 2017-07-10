#include <stdbool.h>
#include "main.h"
//#include "camera_reader.h"
//#include "downsampler.h"
//#include "entropic_enc.h"
//#include "quantizer.h"
//#include "streamer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>


static int coder_init(int width, int height) {

    pppx = 2;
    pppy = 2;
	scanlines = calloc((height/pppy),sizeof(char *));
	for (int i=0; i < height/pppy; i++) {
        scanlines[i] = calloc (width/pppx,sizeof(char));
    }

    return 0;
}

int main(int argc, char* argv[]) {

	struct timeval t_ini, t_fin;
	//double secs;

DEBUG=true;

    printf("hello world");
	gettimeofday(&t_ini, NULL);
	coder_init(640, 480);
	gettimeofday(&t_fin, NULL);
	}
