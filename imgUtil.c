/**
 * @file quantizer.c
 * @author JJGA, MAG
 * @date July 2017
 * @brief image utilities.
 *
 * This module reads images in rgb (BMP format) and convert into YUV. also saves images in BMP format
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include "globals.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

/*
http://aras-p.info/blog/2007/05/28/now-thats-what-i-call-a-good-api-stb_image/

https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
*/

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void load_frame(char const* filename)
{
if (DEBUG) printf("ENTER in load frame...\n");
//orig_Y=stbi_load("../LHE_Pi/img/lena.bmp", &width_orig, &height_orig, &channels, 0);
rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);

if (DEBUG) printf(" image loaded. width=%d, height=%d",width_orig_Y,height_orig_Y);

//int i = stbi_write_bmp("../LHE_Pi/img/kk.bmp", width_orig, height_orig, channels, orig_Y);

//printf("%d  , %d\n",i, channels);
//stbi_image_free( orig_Y );
printf("exit from load_frame...\n");
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void save_frame(char const* filename, int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V)
{
if (DEBUG) printf("ENTER in save frame...\n");
//int i = stbi_write_bmp("../LHE_Pi/img/kk.bmp", width_orig, height_orig, channels, orig_Y);

if (DEBUG) printf (" saving frame width=%d, height=%d channels=%d \n",width, height, channels);
//cargamos el array
unsigned char *data=malloc (channels*width*height);



if (channels==1)
{
int pix=0;

for (int line=0;line < height;line++)
{
 for (int x=0;x<width;x++)
 {
 data[line*width+x]=Y[line][x];
 }
}


}
int i = stbi_write_bmp(filename, width, height, channels, data);
if (DEBUG) printf ("resultado save file = %d \n",i);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void rgb2yuv(unsigned char *rgb,  int rgb_channels)
{
/// this function transform an input image stored in *rgb into YUV image stored in 3 arrays
if (DEBUG) printf ("ENTER in rgb2yuv()...\n");


//memory allocation for yuv storage (de momento solo la Y)
//----------------------------------
orig_Y=malloc(height_orig_Y*sizeof (unsigned char *));
orig_U=malloc(height_orig_Y*sizeof (unsigned char *));
orig_V=malloc(height_orig_Y*sizeof (unsigned char *));

for (int i=0;i<height_orig_Y;i++)
{
orig_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
orig_U[i]=malloc(width_orig_Y* sizeof (unsigned char));
orig_V[i]=malloc(width_orig_Y* sizeof (unsigned char));

}

//data conversion (de momento es fake, pilla canal R y listo)
//-----------------------------------------
int r=0;
int g=0;
int b=0;

for (int line=0;line<height_orig_Y;line++)
{
 int k=0;
 for (int j=0;j<width_orig_Y*rgb_channels;j+=rgb_channels)
 {
    r=rgb[line*width_orig_Y*rgb_channels+j];
    g=rgb[line*width_orig_Y*rgb_channels+j+1];
    b=rgb[line*width_orig_Y*rgb_channels+j+2];

   orig_Y[line][k]=(r*299+g*587+b*114)/1000;
   orig_U[line][k]=128+ ((-r*168+g*331+b*500)/1000);
   orig_V[line][k]=128+ ((r*500-g*418-b*81)/1000);

   k++;//next pixel


 }
 //printf("   \n");
}

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data) {

/// de momento esta funcion no la usamos

if (DEBUG) printf ("ENTER in yuv2rgb()...\n");


//memory allocation
//------------------
data=malloc(height*width*channels*sizeof (unsigned char));
for (int line=0;line<height;line++)
{
  for (int x=0;x<width;x++)
  {
  data[line*width+x]=y[line][x];
  }
}


}
