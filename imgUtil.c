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
#include "include/globals.h"
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "stb_image/stb_image_write.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
/*
http://aras-p.info/blog/2007/05/28/now-thats-what-i-call-a-good-api-stb_image/

https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
*/

void init_imageUtil()
{


}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void load_frame(char const* filename)
{
if (DEBUG) printf("ENTER in load frame...\n");
//orig_Y=stbi_load("../LHE_Pi/img/lena.bmp", &width_orig, &height_orig, &channels, 0);
rgb=stbi_load(filename, &width_orig_Y, &height_orig_Y, &rgb_channels, 0);

/* esto es nuevo*/
//width_orig_UV=width_orig_Y;
//height_orig_UV=height_orig_Y;


//width_orig_Y=640;
//height_orig_Y=480;
//rgb_channels=3;

if (DEBUG) printf(" image loaded. width=%d, height=%d",width_orig_Y,height_orig_Y);

//int i = stbi_write_bmp("../LHE_Pi/img/kk.bmp", width_orig, height_orig, channels, orig_Y);

//printf("%d  , %d\n",i, channels);
//stbi_image_free( orig_Y );
printf("exit from load_frame...\n");
}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void save_frame(char const* filename, int width, int height, int channels, unsigned char **Y,unsigned char **U, unsigned char **V, int yuvmodel)
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

//yuv2rgb(orig_down_Y,orig_down_U,orig_down_V,3,width_down_Y,height_down_Y, data);
//yuv2rgb(Y,U,V,3,width_down_Y,height_down_Y, data,yuvmodel);
yuv2rgb(Y,U,V,3,width,height, data,yuvmodel);

}
int i = stbi_write_bmp(filename, width, height, channels, data);
if (DEBUG) printf ("resultado save file = %d \n",i);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void rgb2yuv(unsigned char *rgb,  int rgb_channels)
{
/// this function transform an input image stored in *rgb into YUV image stored in 3 arrays
if (DEBUG) printf ("ENTER in rgb2yuv()..--.\n");

//las componentes UV van a medir lo mismo
//---------------------------------------
width_orig_UV=width_orig_Y;
height_orig_UV=height_orig_Y;

//memory allocation ESTO ES UN POTENCIAL BUG, NO DEBEMOS HACER MALLOCS FUERA DE INIT
//------------------


//memory allocation for yuv storage (de momento solo la Y)
//---------------------------------------------------------
orig_Y=malloc(height_orig_Y*sizeof (unsigned char *));
orig_U=malloc(height_orig_UV*sizeof (unsigned char *));
orig_V=malloc(height_orig_UV*sizeof (unsigned char *));



scaled_Y=malloc(height_orig_Y*sizeof (unsigned char *));
scaled_U=malloc(height_orig_UV*sizeof (unsigned char *));
scaled_V=malloc(height_orig_UV*sizeof (unsigned char *));


for (int i=0;i<height_orig_Y;i++)
{
orig_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
orig_U[i]=malloc(width_orig_UV* sizeof (unsigned char));
orig_V[i]=malloc(width_orig_UV* sizeof (unsigned char));



scaled_Y[i]=malloc(width_orig_Y* sizeof (unsigned char));
scaled_U[i]=malloc(width_orig_UV* sizeof (unsigned char));
scaled_V[i]=malloc(width_orig_UV* sizeof (unsigned char));


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


// podriamos optimizar haciendo un subsampling de tipo YUV420 para las componentes
// de crominancia iniciales, en lugar de tener que downsamplear despues

for (int line=0;line<height_orig_Y;line++)
{
 int k=0;
 for (int j=0;j<width_orig_Y*rgb_channels;j+=rgb_channels)
 {
    r=rgb[line*width_orig_Y*rgb_channels+j];
    g=rgb[line*width_orig_Y*rgb_channels+j+1];
    b=rgb[line*width_orig_Y*rgb_channels+j+2];

 // YCbCr equations
  int y=(r*299+g*587+b*114)/1000;//luma (not luminance)
  int u=128+ (-169*r - 331*g + b*500)/1000;//Cb
  int v=128+ (500*r - 418*g - b*81)/1000;//Cr

  //joseja equations
  //int y=(r*299+g*587+b*114)/1000;//luma (not luminance)
  //int u=128+ (-147*r - 289*g + b*436)/1000;//Cb
  //int v=128+ (615*r - 515*g - b*10)/1000;//Cr


 //int u=128+ (-147*r - 289*g + b*436)/1000;//Cb
 //int v=128+ (615*r - 515*g - b*10)/1000;//Cr



// supuestamente estas ecuaciones son validas, aunque diferentes
// segun https://www.pcmag.com/encyclopedia/term/55166/yuv-rgb-conversion-formulas
// y tambien: http://www.compression.ru/download/articles/color_space/ch03.pdf
// dan mejor resultado al volver luego a rgb,
//  Y =  0.299R + 0.587G + 0.114B
//   U = -0.147R - 0.289G + 0.436B
//   V =  0.615R - 0.515G - 0.100B
 // int u= 128 +(492*(b-y))/1000;
 // int v= 128 +(877*(r-y))/1000;
  /*
  Y  =      (0.257 * R) + (0.504 * G) + (0.098 * B) + 16

Cr = V =  (0.439 * R) - (0.368 * G) - (0.071 * B) + 128

Cb = U = -(0.148 * R) - (0.291 * G) + (0.439 * B) + 128
  */
 // y=((257 * r) + (504 * g) + (98 * b))/1000 + 16;
 /*
 U'= (B-Y)*0.565

V'= (R-Y)*0.713
*/
  //v=((439*r)-(368*g)-71*b)/1000 +128;
  //u=(-148*r-291*g+439*b)/1000+128;
//u=(b-y)*565/1000+128;
//v=(r-y)*713/1000+128;

  if (y<0) y=0;else if (y>255) y=255;
  if (u<0) u=0;else if (u>255) u=255;
  if (v<0) v=0;else if (v>255) v=255;
  //if (y==0) {u=0;v=0;}
  //if (y==255) {u=255;v=255;}

   orig_Y[line][k]=(unsigned char ) y;//r*299+g*587+b*114)/1000;
   orig_U[line][k]=(unsigned char ) u;//128+ (-169*r - 331*g + b*500)/1000;
   orig_V[line][k]=(unsigned char ) v;//128+ (500*r - 418*g - b*81)/1000;

   k++;//next pixel


 }
 //printf("   \n");
}

}
void scale_epx_H2(unsigned char **channel, int height, int down_width, unsigned char **epx,int umbral)
{
int e1,e2,p,a,b,c,d,count;

int t=umbral;
  for (int y=0;y<height;y++)
    {
    for (int x=0;x<down_width;x++)
      {
       p=channel[y][x];
       e1=p;
       e2=p;

       if (y>0 && x>0 && x<down_width-1 && y<height-1)
        {
        a=channel[y-1][x];
        b=channel[y][x+1];
        c=channel[y][x-1];
        d=channel[y+1][x];
        count=0;
        if ((abs(c-a)<t) && (abs(c-d)<t)) {e1=(c+p)/2;count++;}
        else if (abs(c-a)<t)  {e1=(a+p+c)/3;count++;}
        else if (abs(c-d)<t)  {e1=(d+p+c)/3;count++;}

        if ((abs(b-a)<t) && (abs(b-d)<t)) {e2=(b+p)/2;count++;}
        else if (abs(b-a)<t)  {e2=(a+p+b)/3;count++;}
        else if (abs(b-d)<t)  {e2=(d+p+b)/3;count++;}


        //salva el disparo
        if (count==2) {
         e1=p;e2=p;
         }

        }


       epx[y][x*2]=e1;
       epx[y][x*2+1]=e2;


      }
    }

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void scale_epx(unsigned char **channel, int c_height, int c_width, unsigned char **epx,int umbral)
{

int a,b,c,d,t,e1,e2,e3,e4,p,count;
t=umbral;
printf ("pppx %d pppy %d", pppx,pppy);


int pppx=2;
int pppy=2;

for (int y=0;y<c_height;y++)
{
  for (int x=0;x<c_width;x++)
  {
   p=channel[y][x];
   e1=p;
   e2=p;
   e3=p;
   e4=p;


   if (y>0 && x>0 && x<c_width-1 && y<c_height-1)
   {
   a=channel[y-1][x];
   b=channel[y][x+1];
   c=channel[y][x-1];
   d=channel[y+1][x];

   count=0;
/*
   if (abs(c-a)<t) {e1=(a+c)>>1;count++;}
   if (abs(b-a)<t) {e2=(a+b)>>1;count++;}
   if (abs(d-c)<t) {e3=(d+c)>>1;count++;}
   if (abs(b-d)<t) {e4=(b+d)>>1;count++;}
*/


 if (abs(c-a)<t) {e1=(a+c+p)/3;count++;}
   if (abs(b-a)<t) {e2=(a+b+p)/3;count++;}
   if (abs(d-c)<t) {e3=(d+c+p)/3;count++;}
   if (abs(b-d)<t) {e4=(b+d+p)/3;count++;}

   //salva el disparo!
   if (count>=3) {

   e1=p;e2=p;e3=p;e4=p;}
    }
   epx[y*pppy][x*pppx]=e1;
   epx[y*pppy][x*pppx+1]=e2;
   epx[y*pppy+1][x*pppx]=e3;
   epx[y*pppy+1][x*pppx+1]=e4;




  }
}


}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data,int yuvmodel) {

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

https://en.wikipedia.org/wiki/YCbCr
   R = Y + 1.140V
   G = Y - 0.395U - 0.581V
   B = Y + 2.032U
*/
int pix=0;

int divisorUV=2;
if (yuvmodel==444) {
divisorUV=1;
}
else if (yuvmodel==420) {
divisorUV=2;
}
else{
printf ("yuv2rgb() :model YUV incorrect  %d", yuvmodel);
exit(0);
}
for (int x=0;x<width*3;x+=3)//channels)
  {
  int yp=y[line][pix];
  int up=u[line/divisorUV][pix/divisorUV];
  int vp=v[line/divisorUV][pix/divisorUV];

  //up=(up-128)*2+128;
  //vp=(vp-128)*2+128;
  // if (up<0) up=0;else if (up>255) up=255;
  //if (vp<0) vp=0;else if (vp>255) vp=255;


  pix++;
  // el downsampling ha hecho que algunas tuplas yuv sean incoherentes pues
  // y,u,v ( usamos YCbCr) no son independientes. primero hay que arreglar
  // el triplete
  //most of the YUV space is in fact unused !!!!!
  //in YCbCr, the Y is the brightness (luma), Cb is blue minus luma (B-Y) and Cr is red minus luma (R-Y)


  /*
     Unlike R, G, and B, the Y, Cb and Cr values are not independent;
     choosing YCbCr values arbitrarily may lead to one or more of the RGB values
     that are out of gamut, i.e. greater than 1.0 or less than 0.0.
    */


  //if (y<0) y=0;else if (y>255) y=255;
  //if (up<20) up=20;else if (up>105) up=105;
  //if (vp<20) vp=20;else if (vp>105) vp=105;


  int r=(1000*yp+1402*(vp-128))/1000;
  int g=(1000*yp-344*(up-128)- 714*(vp-128))/1000;
  int b=(1000*yp+1772*(up-128))/1000;

/*
 R = Y + 1.140V
   G = Y - 0.395U - 0.581V
   B = Y + 2.032U
*/

/*
  int r=(1000*yp+1140*(vp-128))/1000;
  int g=(1000*yp-395*(up-128)- 581*(vp-128))/1000;
  int b=(1000*yp+2032*(up-128))/1000;
*/

/*
B = 1.164(Y - 16)                   + 2.018(U - 128)

G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)

R = 1.164(Y - 16) + 1.596(V - 128)
*/
//   b=(1164*(yp-16))/1000;
//   g=(1164*(yp-16)-813*(vp-128)-391*(up-128))/1000;
//   r=(1164*(yp-16)+1596*(vp-128))/1000;

  // if (b<0 || b>255) printf("%d \n",r);

  if (r<0 )r=0;else if (r>255) r=255;
  if (g<0 )g=0;else if (g>255) g=255;
  if (b<0 )b=0;else if (b>255) b=255;

  //r=max (0,r);r=min(255,r);
  //g=max (0,g);r=min(255,g);
  //b=max (0,b);r=min(255,b);
  //printf ("r:%d, g:%d, b:%d \n",r,g,b);

  data[line*width*3+x]=(unsigned char)r;
  data[line*width*3+x+1]=(unsigned char)g;
  data[line*width*3+x+2]=(unsigned char)b;

  }

}
printf(" 3 channels done \n");
}


}




//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int get_PSNR_Y(unsigned char **result_Y, unsigned char ** orig_Y, int height , int width)
{
double total=0;
double dif=0;
for (int y=0;y<height;y++){
  for (int x=0;x<width;x++){

  dif=(double)orig_Y[y][x]- (double)result_Y[y][x];
  total+=dif*dif;
  }
  }
  double  mse=total/((double)height*(double)width);
  //printf ("mse=%f \n",mse);
double psnr=10.0*log10((255.0*255.0)/mse);
  //printf ("psnr dentro=%f \n",psnr);
 //printf ("mse dentro=%f \n",mse);
 int k = (int) (psnr*100);
return k;



}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void yuv444_torgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data) {

for (int line=0;line<height;line++)
{
/*

https://en.wikipedia.org/wiki/YCbCr
   R = Y + 1.140V
   G = Y - 0.395U - 0.581V
   B = Y + 2.032U
*/
int pix=0;
for (int x=0;x<width*3;x+=3)//channels)
  {
  int yp=y[line][pix];
  int up=u[line][pix];
  int vp=v[line][pix];

  //up=(up-128)*2+128;
  //vp=(vp-128)*2+128;
  // if (up<0) up=0;else if (up>255) up=255;
  //if (vp<0) vp=0;else if (vp>255) vp=255;


  pix++;
  // el downsampling ha hecho que algunas tuplas yuv sean incoherentes pues
  // y,u,v ( usamos YCbCr) no son independientes. primero hay que arreglar
  // el triplete
  //most of the YUV space is in fact unused !!!!!
  //in YCbCr, the Y is the brightness (luma), Cb is blue minus luma (B-Y) and Cr is red minus luma (R-Y)


  /*
     Unlike R, G, and B, the Y, Cb and Cr values are not independent;
     choosing YCbCr values arbitrarily may lead to one or more of the RGB values
     that are out of gamut, i.e. greater than 1.0 or less than 0.0.
    */


  //if (y<0) y=0;else if (y>255) y=255;
  //if (up<20) up=20;else if (up>105) up=105;
  //if (vp<20) vp=20;else if (vp>105) vp=105;



  int r=(1000*yp+1402*(vp-128))/1000;
  int g=(1000*yp-344*(up-128)- 714*(vp-128))/1000;
  int b=(1000*yp+1772*(up-128))/1000;

 // int r=(1000*yp+1370*(vp-128))/1000;
 // int g=(1000*yp-337*(up-128)- 698*(vp-128))/1000;
 // int b=(1000*yp+1732*(up-128))/1000;



  //int r=(1000*yp+1140*(vp-128))/1000;
  //int g=(1000*yp-395*(up-128)- 581*(vp-128))/1000;
  //int b=(1000*yp+2032*(up-128))/1000;


/*
B = 1.164(Y - 16)                   + 2.018(U - 128)

G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)

R = 1.164(Y - 16) + 1.596(V - 128)
*/
//   b=(1164*(yp-16))/1000;
//   g=(1164*(yp-16)-813*(vp-128)-391*(up-128))/1000;
//   r=(1164*(yp-16)+1596*(vp-128))/1000;

   //if (b<0 || b>255) printf("%d \n",r);

  if (r<0 )r=0;else if (r>255) r=255;
  if (g<0 )g=0;else if (g>255) g=255;
  if (b<0 )b=0;else if (b>255) b=255;

  //r=max (0,r);r=min(255,r);
  //g=max (0,g);r=min(255,g);
  //b=max (0,b);r=min(255,b);
  //printf ("r:%d, g:%d, b:%d \n",r,g,b);

  data[line*width*3+x]=(unsigned char)r;
  data[line*width*3+x+1]=(unsigned char)g;
  data[line*width*3+x+2]=(unsigned char)b;

  }

}
printf(" 3 channels done \n");
}






