




#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include "globals.h"
#include "imgUtil.h"
#include "downsampler.h"
#include "quantizer.h"
#include "frame_encoder.h"

double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void framecoder_init(int width, int height,int px, int py)
{
    downsampler_initialized=false;
    quantizer_initialized=false;
    pppx = px;
    pppy = py;
    width_orig=width;
    height_orig=height;

    init_downsampler();
    init_quantizer();

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void quantize_frame()
{
 if (DEBUG) printf ("ENTER in quantizeframe... \n");

    //int module=8;
    //int line=0;

	for (int line=0;line<height_down_Y;line++)
	{
	quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
	//line +=module;
	//line = line % height_down_Y; //Actualmente recorre siemre el mismo bloque 0,8,16, etc...
	//line = ((i+1) % (height/module) == 0) ? ((line % height)+1) : (line % height);// Esta linea hace que se recorran bien los slices
	}



}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void encode_frame_fromfile()
{
if (DEBUG) printf("ENTER in encode_frame... \n");
struct timeval t_ini, t_fin;
double secs;


load_frame("../LHE_Pi/img/lena.bmp");
printf("frame loaded  \n");

pppx=2;
pppy=2;
framecoder_init(width_orig,height_orig,pppx,pppy);

rgb2yuv(rgb,rgb_channels);
printf("rgb2yuv done \n");




pppx=2;pppy=2;
downsample_frame(pppx,pppy);


printf("down done\n");

gettimeofday(&t_ini, NULL);
quantize_frame();
gettimeofday(&t_fin, NULL);
printf("quantization done\n");
secs = timeval_diff(&t_fin, &t_ini);

printf("%.16g ms\n", secs * 1000.0);
//char *data;
//yuv2rgb(orig_down_Y,orig_down_U,orig_down_V,1,width_down_Y,height_down_Y, data);

//save_frame("../LHE_Pi/img/kk.bmp", width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V);

//hago esto como prueba. no se donde esta el error. si en save frame pongo orig_down_Y funciona bien
printf ("width_down_Y %d heigh down %d\n", width_down_Y,height_down_Y);
for (int y=0;y<height_down_Y;y++)
{
for (int x=0;x<width_down_Y;x++)
  {
 // result_Y[y][x]=orig_down_Y[y][x];
}
}
//result_Y=orig_down_Y;

save_frame("../LHE_Pi/img/kk.bmp", width_down_Y, height_down_Y, 1, result_Y,result_U,result_V);
printf("save done \n");


}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void downsample_frame( int pppx,  int pppy)
{
/// this function downsample a frame scanline by scanline, in the same order than quantizer
/// scanlines are processed in order module 8
if (DEBUG) printf("ENTER in downsample_frame...\n");

// downsampler initialization, if needed
//---------------------------------------
if (downsampler_initialized==false) init_downsampler();


//downsampling by scanlines
//--------------------------
//int line=0;


//esto debe ser coregido para que recorra las scanlines salteadas modulo 8
for (int line=0;line<height_orig;line+=pppy)
	{
	//printf( "downsampling line %d \n",line);
	down_avg_horiz(orig_Y,orig_down_Y,line,pppx,pppy);
	//line =((line + (MODULE) ) % height_orig) ;
	//line++;
	}


}
