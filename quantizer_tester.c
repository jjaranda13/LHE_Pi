#include <stdbool.h>
#include "globals.h"
//#include "camera_reader.h"
#include "downsampler.h"
#include "entropic_enc.h"
#include "quantizer.h"
//#include "streamer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "imgUtil.h"

/*Función auxiliar para medir tiempos de ejecución*/
/*
double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}
*/



int main(int argc, char* argv[]) {

	struct timeval t_ini, t_fin;
	double secs;

    DEBUG=true;
    downsampler_initialized=false;
    quantizer_initialized=false;

    printf("hello world\n");
/*
	gettimeofday(&t_ini, NULL);
	pppx=2;
	pppy=2;
	framecoder_init(640, 480,pppx,pppy);
	gettimeofday(&t_fin, NULL);
	secs = timeval_diff(&t_fin, &t_ini);
	printf("%.16g ms\n", secs * 1000.0);
	*/
/*
    gettimeofday(&t_ini, NULL);

    int width=640/pppx;
    int height=480/pppy;
    int module=8;
    int line=0;
    for (int j=0;j<1;j++)
	for (int i=0;i<height;i++)
	{
	quantize_scanline( orig_Y,  line, width, hops_Y,result_Y);
	line +=module;
	line = line % height; //Actualmente recorre siemre el mismo bloque 0,8,16, etc...
	//line = ((i+1) % (height/module) == 0) ? ((line % height)+1) : (line % height);// Esta linea hace que se recorran bien los slices
	}

	gettimeofday(&t_fin, NULL);

	secs = timeval_diff(&t_fin, &t_ini);
	secs=secs/1000;
	printf("%.16g ms\n", secs * 1.0);
*/
    printf ("encoding frame \n");
//    debug=true;
    DEBUG=false;
    encode_frame_fromfile();
    printf("Creado fichero ");

	}
