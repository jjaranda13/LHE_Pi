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


struct thread_info
{
int start;
int separation;
int num_threads;
};

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

    for (int i = 0; i < 9; i++)
        hops_type[i] = 0;

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
//         quantize_subframe
//         -----------------
// esta funcion procesa un grupo de lineas separadas entre si
// comienza en start_line, despues start_line+8, despues start_line+16 etc
// y asi hasta que llega al final de la imagen. en ese momento se detiene,
// la formula general es line= (start_line+N*separacion)
// en caso de tener varios threads, la separacion debe ser separacion * num_threads. de este modo
// el th1 ejecutara la linea 0, luego la 8*3=24, luego la 48, etc
// el th2 ejeutara la linea 8 , luego la 32, luego la 56 etc
// el th3 ejecutara la linea 16, luego la 40, luego la 64 etc
//------------------------------------------------------------
void quantize_subframe(int start_line,int separacion)
{
int line=start_line;
int n=0;

// primeramente procesamos todas las lineas de luminancia
// -----------------------------------------------------
// empezamos por la start_line

while (line<height_down_Y)
{
  //componentes luminancia
  if (DEBUG)  printf("line %d \n",line);
  quantize_scanline( orig_down_Y,  line, width_down_Y, hops_Y,result_Y);

  n++;
  line=(start_line+n*separacion);

}


// tras la luminancia, procesamos las de crominancia. son menos
// ------------------------------------------------------------
line=start_line;
n=0;

line=(start_line+n*8)% height_down_UV;
while (line<height_down_UV)
{
  if (DEBUG) printf("line UV %d \n",line);
  quantize_scanline( orig_down_U,  line, width_down_UV, hops_U,result_U);
  quantize_scanline( orig_down_V,  line, width_down_UV, hops_V,result_V);
  n++;
  line=(start_line+n*separacion) ;
}


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//esta funcion es un ejemplo muy sencillo, solo valido para un thread
//si tenemos 2 threads y queremos una separacion de lineas 8
// th1 quantize_subframe(0,16) ejecutaria 0,16,32 etc
// th2 quantize_subframe(8,16) ejecutaria 8,24,48 etc
// de este modo las primeras lineas procesadas seran la 0,8,16,24,32,48 etc
// el th1 y el th2 despues tendrian que invocarse con
// th1 quantize_subframe(1,16) ejecutaria 1,17,33 etc
// th2 quantize_subframe(9,16) ejecutaria 9,25,49 etc
// por lo tanto tendriamos la 1,9,17,25,33,49 etc
// debemos hacer estas invocaciones hasta el final. como hemos cogido separacion 16 tendremos que las dos
//ultimas invocaciones serian
// th1 quantize_subframe(14,16) ejecutaria 14,30,46 etc
// th2 quantize_subframe(15,16) ejecutaria 15,31,47 etc
void quantize_frame()
{
for (int i=0;i<8;i++)
{
if (DEBUG) printf ("processing lines starting at %d \n",i);
quantize_subframe(i, 8);
}

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//aunque esta funcion no la vamos a usar, la he querido dejar por si la queremos para pruebas
//cuantiza una imagen siguiendo un orden secuencial en las lineas
void quantize_frame_normal()

{
 if (DEBUG) printf ("ENTER in quantizeframe()... \n");

    //luminance
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


/*
     int total_bits=0;
    for (int i = 0;i <9; i++){
        printf("hop %d: %d \n", i, hops_type[i]);

        }*/
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//esta funcion es la que debe ejecutar cualquier thread
void *mytask(void *arg)
{
struct thread_info *tinfo = arg;
//(int start, int separation, int num_threads)
int start=tinfo->start;
int separation=tinfo->separation;
int num_threads=tinfo->num_threads;

int invocaciones=separation/num_threads;

for (int i=start; i< start+invocaciones;i++){
  quantize_subframe(i,separation);
}

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void encode_frame_fromfile()
{
//DEBUG=true;
if (DEBUG) printf("ENTER in encode_frame... \n");
struct timeval t_ini, t_fin;
double secs;


load_frame("../LHE_Pi/img/lena.bmp");
//load_frame("../LHE_Pi/img/mario.bmp");
//load_frame("../LHE_Pi/img/baboon.bmp");
//load_frame("../LHE_Pi/img/mountain.bmp");
printf("frame loaded  \n");

pppx=2;
pppy=2;
init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int rc1, rc2, rc3, rc4;
pthread_t thread1, thread2, thread3, thread4;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
printf ("init ok");
rgb2yuv(rgb,rgb_channels);

//ahora esta en YUV444


//yuv444_torgb(orig_Y,orig_U,orig_V,3,width_orig_Y,height_orig_Y, rgb);
yuv2rgb(orig_Y,orig_U,orig_V,3,width_orig_Y,height_orig_Y, rgb,444);

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
printf("quantizing...\n");
gettimeofday(&t_ini, NULL);
int veces=1;

// la funcion quantize_frame no usa threads
for (int i=0;i<veces;i++)
//quantize_frame();
quantize_frame_normal();
/*
if ((rc1=pthread_create(&thread1, NULL, &mytask(0,16,2), NULL))){
    printf("Thread creation failed.");
if ((rc2=pthread_create(&thread2, NULL, &mytask(8,16,2), NULL))){
    printf("Thread creation failed.");
*/

/*
struct thread_info *tinfo;
int num_threads=2;
tinfo = calloc(num_threads, sizeof(struct thread_info));


tinfo[0].start=0;
tinfo[0].separation=16;
tinfo[0].num_threads=2;
tinfo[1].start=8;
tinfo[1].separation=16;
tinfo[1].num_threads=2;

if ((rc1=pthread_create(&thread1, NULL, &mytask, &tinfo[0]))){
    printf("Thread creation failed.");
    }

if ((rc2=pthread_create(&thread2, NULL, &mytask, &tinfo[1]))){
    printf("Thread creation failed.");
    }

pthread_join(thread1, NULL);
pthread_join(thread2, NULL);

*/


/*
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

pthread_join(thread1, NULL);
pthread_join(thread2, NULL);
pthread_join(thread3, NULL);
pthread_join(thread4, NULL);



}
*/
gettimeofday(&t_fin, NULL);
printf("quantization done !\n");
secs = timeval_diff(&t_fin, &t_ini)/veces;







printf("quantization in %.16g ms\n", secs * 1000.0);
int umbral=38;//56;//56;
/*
0  -> 29.23 db
28 -> 29.4 db
48 -> 29.51 db
56 -> 29.53
64 -> 29.46
80 -> 29.38 db
128 -> 29.29 db
255 -> 29.23 db
*/

if (pppx==2 && pppy==2){

printf("expanding EPX...\n");
scale_epx(result_Y,height_down_Y,width_down_Y,scaled_Y,umbral);
scale_epx(result_U,height_down_UV,width_down_UV,scaled_U,umbral);
scale_epx(result_V,height_down_UV,width_down_UV,scaled_V,umbral);

/*
scale_epx(scaled_Y2,height_down_Y*2,width_down_Y*2,scaled_Y,umbral);
scale_epx(scaled_U2,height_down_UV*2,width_down_UV*2,scaled_U,umbral);
scale_epx(scaled_V2,height_down_UV*2,width_down_UV*2,scaled_V,umbral);
*/

save_frame("../LHE_Pi/img/LHE_scaled_BN.bmp", width_orig_Y, height_orig_Y, 1, scaled_Y,scaled_U,scaled_V,420);
save_frame("../LHE_Pi/img/LHE_scaled_color.bmp", width_orig_Y, height_orig_Y, 3, scaled_Y,scaled_U,scaled_V,420);
}

int bits = 0;
int pixels = 512*512;
gettimeofday(&t_ini, NULL);
for (int i=0 ;i<veces;i++){
for (int line=0;line<height_down_Y;line++) {
    bits += entropic_enc(hops_Y, bits_Y, line, width_down_Y);
	//tam_bytes_Y[line]=entropic_enc(hops_Y, bits_Y, line, width_down_Y); //Consultar: Esta instruccion es para que el streamer sepa cuantos bits ocupa cada linea
}
for (int line=0;line<height_down_UV;line++) {
    bits+=entropic_enc(hops_U, bits_U, line, width_down_UV);
    bits+=entropic_enc(hops_V, bits_V, line, width_down_UV);
}
}
printf("Bits: %d\n", bits);
float bpp = (float)bits/(float)pixels;
printf("bpp: %f\n", bpp);
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

save_frame("../LHE_Pi/img/orig_Y.bmp", width_orig_Y, height_orig_Y, 1, orig_Y,orig_down_U,orig_down_V,420);
save_frame("../LHE_Pi/img/orig_U.bmp", width_orig_Y, height_orig_Y, 1, orig_U,orig_down_U,orig_down_V,420);
save_frame("../LHE_Pi/img/orig_V.bmp", width_orig_Y, height_orig_Y, 1, orig_V,orig_down_U,orig_down_V,420);

save_frame("../LHE_Pi/img/orig_down_Y.bmp", width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V,420);
save_frame("../LHE_Pi/img/orig_down_U.bmp", width_down_UV, height_down_UV, 1, orig_down_U,orig_down_U,orig_down_V,420);
save_frame("../LHE_Pi/img/orig_down_V.bmp", width_down_UV, height_down_UV, 1, orig_down_V,orig_down_U,orig_down_V,420);


save_frame("../LHE_Pi/img/LHE_Y.bmp", width_down_Y, height_down_Y, 1, result_Y,result_U,result_V,420);
save_frame("../LHE_Pi/img/LHE_YUV.bmp", width_down_Y, height_down_Y, 3, result_Y,result_U,result_V,420);
save_frame("../LHE_Pi/img/orig_down_YUV.bmp", width_down_Y, height_down_Y, 3, orig_down_Y,orig_down_U,orig_down_V,420);

printf("save done \n");

double psnr= get_PSNR_Y(result_Y,orig_down_Y,height_down_Y,width_down_Y);
printf("psnr down: %2.2f dB\n ",psnr);

//if (pppx==2 && pppy==2)
{
double psnr2= get_PSNR_Y(scaled_Y,orig_Y, height_orig_Y,width_orig_Y);
printf("psnr scaled: %2.2f dB\n ",(float)psnr2);
}


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

