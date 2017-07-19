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


//OJO ESTO PUEDE DAR LUGAR A UN ERROR: NO HACER MALLOCS FUERA de un init
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
else if (channels==3){

yuv2rgb(orig_down_Y,orig_down_U,orig_down_V,3,width_down_Y,height_down_Y, data);


}
int i = stbi_write_bmp(filename, width, height, channels, data);
if (DEBUG) printf ("resultado save file = %d \n",i);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void rgb2yuv(unsigned char *rgb,  int rgb_channels)
{
/// this function transform an input image stored in *rgb into YUV image stored in 3 arrays
if (DEBUG) printf ("ENTER in rgb2yuv()..--.\n");

//memory allocation ESTO ES UN POTENCIAL BUG, NO DEBEMOS HACER MALLOCS FUERA DE INIT
//------------------


//memory allocation for yuv storage (de momento solo la Y)
//---------------------------------------------------------
orig_Y=malloc(height_orig_Y*sizeof (unsigned char *));
orig_U=malloc(height_orig_Y*sizeof (unsigned char *));
orig_V=malloc(height_orig_Y*sizeof (unsigned char *));

for (int i=0;i<height_orig_Y;i++)
{
orig_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
orig_U[i]=malloc(width_orig_Y* sizeof (unsigned char));
orig_V[i]=malloc(width_orig_Y* sizeof (unsigned char));

}

//data conversion
//------------------
int r=0;
int g=0;
int b=0;
/*Y =  0.299R + 0.587G + 0.114B
   U = -0.147R - 0.289G + 0.436B
   V =  0.615R - 0.515G - 0.100B
   */
for (int line=0;line<height_orig_Y;line++)
{
 int k=0;
 for (int j=0;j<width_orig_Y*rgb_channels;j+=rgb_channels)
 {
    r=rgb[line*width_orig_Y*rgb_channels+j];
    g=rgb[line*width_orig_Y*rgb_channels+j+1];
    b=rgb[line*width_orig_Y*rgb_channels+j+2];

   orig_Y[line][k]=(r*299+g*587+b*114)/1000;
   orig_U[line][k]=128+ (-168*r - 331*g + b*500)/1000;
   orig_V[line][k]=128+ (500*r - 418*g - b*81)/1000;

   k++;//next pixel


 }
 //printf("   \n");
}

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data) {

//if (DEBUG)
printf ("ENTER in yuv2rgb()...\n");


//memory allocation ESTO ES UN POTENCIAL BUG, NO DEBEMOS HACER MALLOCS FUERA DE INIT
//------------------
//free (data);
//data=malloc(height*width*channels*sizeof (unsigned char));

//imagenes de solo componente Y
// -----------------------------
if (channels==1){
for (int line=0;line<height;line++)
{
  for (int x=0;x<width;x++)
  {
  data[line*width+x]=y[line][x];
  }
}
return;
}//endif channels==1

//imagenes con crominancia
//------------------------
if (channels ==3){
printf (" 3 channels... \n");
for (int line=0;line<height;line++)
{
/*
   R = Y + 1.140V
   G = Y - 0.395U - 0.581V
   B = Y + 2.032U
*/
int pix=0;
for (int x=0;x<width*3;x+=channels)
  {
  int yp=y[line][pix];
  int up=u[line/2][pix/2];
  int vp=v[line/2][pix/2];
  pix++;
  int r=(1000*yp+1140*vp)/1000;
  int g=(1000*yp-395*up- 581*vp)/1000;
  int b=(1000*yp+2032*up)/1000;
  //printf ("r:%d, g:%d, b:%d \n",r,g,b);

  data[line*width*3+x]=(unsigned char)r;
  data[line*width*3+x+1]=(unsigned char)g;
  data[line*width*3+x+2]=(unsigned char)b;

  }

}
printf(" 3 channels done \n");
}


}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
double get_PSNR_Y()
{
double total=0;
double dif=0;
for (int y=0;y<height_down_Y;y++){
  for (int x=0;x<width_down_Y;x++){

  dif=orig_down_Y[y][x]- result_Y[y][x];
  total+=dif*dif;
  }
  }
  double mse=total/(double)(height_down_Y*width_down_Y);
  //printf ("mse=%f \n",mse);
double psnr=10.0*log10((255.0*255.0)/mse);
 //printf ("psnr=%f \n",psnr);
return psnr;



}
