




#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include "globals.h"
#include "imgUtil.h"


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void encode_frame()
{
if (DEBUG) printf("ENTER in encode_frame... \n");

load_frame("../LHE_Pi/img/lena.bmp");
printf("frame loaded  \n");

rgb2yuv(rgb,rgb_channels);
printf("rgb2yuv done \n");

pppx=2;pppy=2;
downsample_frame(pppx,pppy);


printf("down done\n");


//char *data;
//yuv2rgb(orig_down_Y,orig_down_U,orig_down_V,1,width_down_Y,height_down_Y, data);

save_frame("../LHE_Pi/img/kk.bmp", width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V);
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
int line=0;


//esto debe ser coregido para que recorra las scanlines salteadas modulo 8
for (int line=0;line<height_orig;line+=pppy)
	{
	//printf( "downsampling line %d \n",line);
	down_avg_horiz(orig_Y,orig_down_Y,line,pppx,pppy);
	//line =((line + (MODULE) ) % height_orig) ;
	//line++;
	}


}

