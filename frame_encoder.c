#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include "include/globals.h"
#include "include/imgUtil.h"
#include "include/downsampler.h"
#include "include/quantizer.h"
#include "include/frame_encoder.h"
#include "include/entropic_enc.h"

double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_framecoder(int width, int height,int px, int py)
{
    downsampler_initialized=false;
    quantizer_initialized=false;
    pppx = px;
    pppy = py;
    width_orig_Y=width;
    height_orig_Y=height;

    //modelo YUV 444 inicialmente
    //----------------
    //height_orig_UV=width_orig_Y;
    //width_orig_UV=width_orig_Y;

    init_downsampler();
    init_quantizer();
    init_entropic_enc();
}

void *quantize_pair() {
    for (int line=0; line < height_down_Y/2; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
	entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=0; line < height_down_UV/2; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
	entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
	entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

void *quantize_impair() {
    for (int line=height_down_Y/2; line < height_down_Y; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
	entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=height_down_UV/2; line < height_down_UV; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
	entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

void *quantize_one() {
    for (int line=0; line < height_down_Y/4; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=0; line < height_down_UV/4; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
        entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

void *quantize_two() {
    for (int line=0; line < height_down_Y/2; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=0; line < height_down_UV/2; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
        entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

void *quantize_three() {
    for (int line=0; line < 3*height_down_Y/4; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=0; line < 3*height_down_UV/4; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
        entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

void *quantize_four() {
    for (int line=0; line < height_down_Y; line++){
        quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
        entropic_enc(hops_Y, bits_Y, line, width_down_Y);
    }
    for (int line=0; line < height_down_UV; line++){
        quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
        entropic_enc(hops_U, bits_U, line, width_down_UV);
        quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
        entropic_enc(hops_V, bits_V, line, width_down_UV);
    }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void quantize_frame()
{
 if (DEBUG) printf ("ENTER in quantizeframe... \n");

    //luminance components
    //--------------------
	for (int line=0;line<height_down_Y;line++){
      quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);
	}
	//chrominance components
	//-----------------------
	for (int line=0;line<height_down_UV;line++){
      quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
      quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
	}



}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void encode_frame_fromfile()
{
//DEBUG=true;
if (DEBUG) printf("ENTER in encode_frame... \n");
struct timeval t_ini, t_fin;
double secs;


//load_frame("../LHE_Pi/img/lena.bmp");
//load_frame("../LHE_Pi/img/baboon.bmp");
load_frame("../LHE_Pi/img/cascada.bmp");
printf("frame loaded  \n");

pppx=1;
pppy=1;
init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int rc1, rc2, rc3, rc4;
pthread_t thread1, thread2, thread3, thread4;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
printf ("init ok");
rgb2yuv(rgb,rgb_channels);

//ahora esta en YUV444


yuv2rgbX(orig_Y,orig_U,orig_V,3,width_orig_Y,height_orig_Y, rgb);

int i = stbi_write_bmp("../LHE_Pi/img/orig_RGB_YUV_RGB.bmp", width_orig_Y, height_orig_Y, 3, rgb);


printf("rgb2yuv done \n");

downsample_frame(pppx,pppy);


printf("down done\n");
/*
gettimeofday(&t_ini, NULL);
int veces=100;
for (int i=0 ;i<veces;i++)
quantize_frame();
gettimeofday(&t_fin, NULL);
printf("quantization done\n");
secs = timeval_diff(&t_fin, &t_ini)/veces;
*/
/*
gettimeofday(&t_ini, NULL);
int veces=100;
for (int i=0 ;i<veces;i++){
if ((rc1=pthread_create(&thread1, NULL, &quantize_pair, NULL))){
    printf("Thread creation failed.");
}
if ((rc2=pthread_create(&thread2, NULL, &quantize_impair, NULL))){
    printf("Thread creation failed.");
}
//pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
}
gettimeofday(&t_fin, NULL);
printf("quantization done\n");
secs = timeval_diff(&t_fin, &t_ini)/veces;
*/

gettimeofday(&t_ini, NULL);
int veces=100;
for (int i=0 ;i<veces;i++){
if ((rc1=pthread_create(&thread1, NULL, &quantize_one, NULL))){
    printf("Thread creation failed.");
}
if ((rc2=pthread_create(&thread2, NULL, &quantize_two, NULL))){
    printf("Thread creation failed.");
}
if ((rc3=pthread_create(&thread3, NULL, &quantize_three, NULL))){
    printf("Thread creation failed.");
}
if ((rc4=pthread_create(&thread4, NULL, &quantize_four, NULL))){
    printf("Thread creation failed.");
}

//pthread_join(thread1, NULL);
//pthread_join(thread2, NULL);
//pthread_join(thread3, NULL);
pthread_join(thread4, NULL);
}
gettimeofday(&t_fin, NULL);
printf("quantization done\n");
secs = timeval_diff(&t_fin, &t_ini)/veces;







printf("quantization in %.16g ms\n", secs * 1000.0);

gettimeofday(&t_ini, NULL);
for (int i=0 ;i<veces;i++){
for (int line=0;line<height_down_Y;line++) {
    entropic_enc(hops_Y, bits_Y, line, width_down_Y);
	//tam_bytes_Y[line]=entropic_enc(hops_Y, bits_Y, line, width_down_Y); //Consultar: Esta instruccion es para que el streamer sepa cuantos bits ocupa cada linea
}
for (int line=0;line<height_down_UV;line++) {
    entropic_enc(hops_U, bits_U, line, width_down_UV);
    entropic_enc(hops_V, bits_V, line, width_down_UV);
}
}
gettimeofday(&t_fin, NULL);
printf("entropic coding done\n");
secs = timeval_diff(&t_fin, &t_ini)/veces;











printf("entropic coding in %.16g ms\n", secs * 1000.0);

/*Para probar el streamer
//Streamer
gettimeofday(&t_ini, NULL);
sendData();
gettimeofday(&t_fin, NULL);
printf("streamer done\n");
secs = timeval_diff(&t_fin, &t_ini);

printf("streamer in %.16g ms\n", secs * 1000.0);*/

//char *data;
//yuv2rgb(orig_down_Y,orig_down_U,orig_down_V,1,width_down_Y,height_down_Y, data);

save_frame("../LHE_Pi/img/orig_Y.bmp", width_orig_Y, height_orig_Y, 1, orig_Y,orig_down_U,orig_down_V);
save_frame("../LHE_Pi/img/orig_U.bmp", width_orig_Y, height_orig_Y, 1, orig_U,orig_down_U,orig_down_V);
save_frame("../LHE_Pi/img/orig_V.bmp", width_orig_Y, height_orig_Y, 1, orig_V,orig_down_U,orig_down_V);

save_frame("../LHE_Pi/img/orig_down_Y.bmp", width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V);
save_frame("../LHE_Pi/img/orig_down_U.bmp", width_down_UV, height_down_UV, 1, orig_down_U,orig_down_U,orig_down_V);
save_frame("../LHE_Pi/img/orig_down_V.bmp", width_down_UV, height_down_UV, 1, orig_down_V,orig_down_U,orig_down_V);


save_frame("../LHE_Pi/img/LHE_Y.bmp", width_down_Y, height_down_Y, 1, result_Y,result_U,result_V);
save_frame("../LHE_Pi/img/LHE_YUV.bmp", width_down_Y, height_down_Y, 3, result_Y,result_U,result_V);
save_frame("../LHE_Pi/img/orig_down_YUV.bmp", width_down_Y, height_down_Y, 3, orig_down_Y,orig_down_U,orig_down_V);

printf("save done \n");

double psnr=(float) get_PSNR_Y();
printf("psnr: %2.2f dB\n ",psnr);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void downsample_frame( int pppx,  int pppy)
{
/// this function downsample a frame scanline by scanline, in the same order than quantizer
/// scanlines are processed in order module 8
//if (DEBUG)
printf("ENTER in downsample_frame...\n");

// downsampler initialization, if needed
//---------------------------------------
if (downsampler_initialized==false) init_downsampler();


//downsampling by scanlines
//--------------------------
//esto debe ser coregido para que recorra las scanlines salteadas modulo 8
printf ("downsampling...");
// component Y
// ------------
//si pppy==2 entonces solo se downsamplean la mitad de las lineas, logicamente
for (int line=0;line<height_orig_Y;line+=pppy){
	down_avg_horiz(orig_Y,width_orig_Y,orig_down_Y,line,pppx,pppy);
	}



// components U, V
// ----------------
// si pppy=2 se downsamplean una de cada 4 lineas
int ratio_height_YUV=height_orig_Y/height_orig_UV;
int ratio_width_YUV=width_orig_Y/width_orig_UV;
int pppyUV=2*pppy/ratio_height_YUV;
int pppxUV=2*pppx/ratio_width_YUV;
printf ("pppx:%d , pppy:%d, pppxUV:%d, pppyUV:%d \n",pppx,pppy,pppxUV,pppyUV);
for (int line=0;line<height_orig_UV;line+=pppyUV){
	down_avg_horiz(orig_U,width_orig_UV,orig_down_U,line,pppxUV,pppyUV);
	down_avg_horiz(orig_V,width_orig_UV,orig_down_V,line,pppxUV,pppyUV);
	}


}

