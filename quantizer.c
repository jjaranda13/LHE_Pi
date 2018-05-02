/**
 * @file quantizer.c
 * @author JJGA
 * @date July 2017
 * @brief quantize one scanline using LHE.
 *
 * This module reads the image signal (Y,U,V) and transform one scanline into hops.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>

#include "include/globals.h"
#include "include/quantizer.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))
// la cache realmente podria ser de 10KB cache_hops[255][7][6]; e incluso de la mitad (5KB) pues es simetrica
//unsigned char cache_hops[256][7][6]; //10KB cache [Y][h1][hop_number]

unsigned char cache_hops[256][7][3]; //5KB cache [Y][h1][hop_number]



//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_quantizer(){
///this function pre computes the cache
///la cache es cache[hop0][h1][hop] = 10.000 bytes = 10 KB

if (DEBUG) printf ("ENTER in init_quantizer()...\n");
if (!downsampler_initialized) {
 printf(" Error: first you must initialize downsampler !!!!!");
 exit(0);
}


for (int hop0=0;hop0<=255;hop0++){
 for (int hop1=4; hop1<=10;hop1++) {
 //ratio for possitive hops. max ratio=3 min ratio=1
 float maxr=2.7f;
 float minr=1.0f;//si fuese menor, un hop mayor podria ser inferior a un hop menor

 // le ponemos rango 0.55 y no rango 8 para que acierte mejor, ya que la prediccion
 // que vamos a usar es mala (pixel izquierdo) y asi mejora
 const float range=0.70f;//0.55f;//0.8f; //con rango menor da mas calidad pero se gastan mas bits!!!!
 double rpos = min (maxr,pow(range*((255-hop0)/hop1),1.0f/3.0f));
 rpos=max(minr,rpos);

 //ratio for negative hops. max ratio=3 min ratio=1
 double rneg = min(maxr,pow(range*(hop0/hop1),1.0f/3.0f));
 rneg=max(minr,rneg);

 //compute hops 0,1,2,6,7,8 (hops 3,4,5 are known and dont need cache)
 // 6,7 and 8 are not needed because cache is symmetrix. they are not computed
 //-------------------------------------------------------------------
 int h=(int)(hop0-hop1*rneg*rneg*rneg);
 int hop_min=1;
 int hop_max=255-hop_min;
 h=min(hop_max,h);h=max(h,hop_min);
 //cache_hops[hop0][hop1-4][0] = 0;//(unsigned char)h;//(hop0-hop1*rneg*rneg*rneg);
cache_hops[hop0][hop1-4][0] = (unsigned char)h;//(hop0-hop1*rneg*rneg*rneg);

 h=(int)(hop0-hop1*rneg*rneg);
 h=min(hop_max,h);h=max(h,hop_min);
 cache_hops[hop0][hop1-4][1] = (unsigned char)h;//(hop0-hop1*rneg*rneg);

 h=(int)(hop0-hop1*rneg);
 h=min(hop_max,h);h=max(h,hop_min);
 cache_hops[hop0][hop1-4][2] = (unsigned char)h;//(hop0-hop1*rneg);
/*
 h=(int)(hop0+hop1*rpos);
 h=min(hop_max,h);h=max(h,hop_min);
 cache_hops[hop0][hop1-4][3] = (unsigned char)h;//(hop0+hop1*rpos);

 h=(int)(hop0+hop1*rpos*rpos);
 h=min(hop_max,h);h=max(h,hop_min);
 cache_hops[hop0][hop1-4][4] = (unsigned char) h;//(hop0+hop1*rpos*rpos);

 h=(int)(hop0+hop1*rpos*rpos*rpos);
 h=min(hop_max,h);h=max(h,hop_min);
 cache_hops[hop0][hop1-4][5] = (unsigned char)h;//(hop0+hop1*rpos*rpos*rpos);
*/
 }//endfor hop1
}//endfor hop0


//memory allocation for result and hops
//---------------------------------------
//esto debe estar en el downsampler
//width_down_Y=width_orig_Y/pppy;
//height_down_Y=height_orig_Y/pppx;

//aqui hay que comprobar el modelo de color con la variable yuv_model
//-----------------------------------------
//esto debe estar en el downsampler
//width_down_UV=width_down_Y/2;
//height_down_UV=height_down_Y/2;




hops_Y=malloc(height_down_Y*sizeof (unsigned char *));
hops_U=malloc(height_down_UV*sizeof (unsigned char *));
hops_V=malloc(height_down_UV*sizeof (unsigned char *));

result_Y=malloc(height_down_Y*sizeof (unsigned char *));
result_U=malloc(height_down_UV*sizeof (unsigned char *));
result_V=malloc(height_down_UV*sizeof (unsigned char *));


result2_Y=malloc(height_down_Y*sizeof (unsigned char *));
result2_U=malloc(height_down_UV*sizeof (unsigned char *));
result2_V=malloc(height_down_UV*sizeof (unsigned char *));



for (int i=0;i<height_down_Y;i++)
  {
  hops_Y[i]=malloc(width_down_Y* sizeof (unsigned char));
  result_Y[i]=malloc(width_down_Y* sizeof (unsigned char));
  result2_Y[i]=malloc(width_down_Y* sizeof (unsigned char));
  }

for (int i=0;i<height_down_UV;i++)
  {

  hops_U[i]=malloc(width_down_UV* sizeof (unsigned char));
  hops_V[i]=malloc(width_down_UV* sizeof (unsigned char));


  result_U[i]=malloc(width_down_UV* sizeof (unsigned char));
  result_V[i]=malloc(width_down_UV* sizeof (unsigned char));

  result2_U[i]=malloc(width_down_UV* sizeof (unsigned char));
  result2_V[i]=malloc(width_down_UV* sizeof (unsigned char));

  }

ent_stream_flag=malloc(height_down_Y* sizeof (unsigned char));

// these lines print the cache, for debug purposes
//----------------------------------
/*
for (int hop0=0;hop0<=255;hop0++)
{
for (int hop1=4;hop1<=10;hop1++)
{
for (int hn=0;hn<=2;hn++)
{

printf( "y=%d h1=%d h%d=%d \n",hop0,hop1,hn,cache_hops[hop0][hop1-4][hn]);

}
printf ( "y=%d h1=%d h3=%d \n",hop0,hop1,(hop0-hop1));
printf ( "y=%d h1=%d h4=%d \n",hop0,hop1,(hop0));
printf ( "y=%d h1=%d h5=%d \n",hop0,hop1,(hop0+hop1));

for (int hn=3;hn<=5;hn++)
{

printf( "y=%d h1=%d h%d=%d \n",hop0,hop1,hn+3,cache_hops[hop0][hop1-4][hn]);

}



}



}

*/

inteligent_discard_Y=malloc(height_down_Y*sizeof(bool));
inteligent_discard_U=malloc(height_down_UV*sizeof(bool));
inteligent_discard_V=malloc(height_down_UV*sizeof(bool));

inteligent_discard_mode = 3;

quantizer_initialized=true;

}// end function
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void close_quantizer(){



// free memory in inverse order than initialization
// --------------------------------------------------
for (int i=0;i<height_down_Y;i++)
  {
  free (hops_Y[i]);
  free(result_Y[i]);
  free(result2_Y[i]);
  }

for (int i=0;i<height_down_UV;i++)
  {

  free(hops_U[i]);
  free (hops_V[i]);


  free (result_U[i]);
  free (result_V[i]);

  free (result2_U[i]);
  free (result2_V[i]);

  }

free (hops_Y);
free(hops_U);
free (hops_V);

free (result_Y);
free(result_U);
free(result_V);

free (result2_Y);
free(result2_U);
free(result2_V);

//inteliigent discard
free (inteligent_discard_Y);
free (inteligent_discard_U);
free (inteligent_discard_V);

quantizer_initialized=false;

}//end close quantizer

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
bool quantize_scanline(unsigned char **orig_YUV, int y,int width, unsigned char **hops,unsigned char **result_YUV) {
/// this function quantize the luminances or chrominances of one scanline
/// inputs : orig_YUV (which can be orig_down_Y, orig_down_U or orig_down_V), line, width,
/// outputs: hops, result_YUV (which can be result_Y, result_U or result_V)


//inteligent discard
bool softline=true;


if (DEBUG) printf ("ENTER in quantize_scanline( %d)...\n",y);

 const int max_h1=10;
 const int min_h1=4;
 const int start_h1=(max_h1+min_h1)/2;
 int  h1=start_h1;

 bool last_small_hop=true; //last hop was small
 bool small_hop=true;//current hop is small


 int emin=255;//error min
 int error=0;//computed error

 unsigned char oc=127;//orig_YUV[y][0];//original color
 unsigned char hop0=0; //prediction
 unsigned char quantum=oc; //final quantum asigned value
 unsigned char hop_value=0;//data from cache
 unsigned char hop_number=4;// final assigned hop

 //mejora de prediccion
 int grad=0;

 //unsigned char prev_color=128;
 //char * result_signal=result_YUV[y];
 //char * result_hops=hops[y];
//this bucle is for only one scanline, excluding first pixel
//----------------------------------------------------------
for (int x=0;x<width;x++)
  {

  // --------------------- PHASE 1: PREDICTION---------------------------------------------------------
  oc=orig_YUV[y][x];//original color

  if (y>0 && x>0 && x!=width-1){

    /*if (last_small_hop) hop0=(result_YUV[y][x-1]+result_YUV[y-1][x]+result_YUV[y-1][x+1])/3;
    else
    hop0=(result_YUV[y][x-1]+result_YUV[y-1][x+1])>>1;
*/
    //prediccion izquierda para scanlines salteadas
    //hop0=result_YUV[y][x-1];
    hop0=quantum;//prev_color;
    }
  else if (x==0 && y>0){
    //este caso vamos a usar 128 y no el pixel superior ya que debido al orden de scanlines no lo tendremos

    //hop0=result_YUV[y-1][0];
    hop0=127;

    }
  else if (y>0){//x=width-1
    //hop0=(result_YUV[y][x-1]+result_YUV[y-1][x])>>1;

    //prediccion izquierda para scanlines salteadas
    //hop0=result_YUV[y][x-1];
    hop0=quantum;//prev_color;
    }
  else{ //y=0
    //hop0=result_YUV[y][x-1];
    hop0=quantum;//prev_color;
   }

  //-------------------------PHASE 2: HOPS COMPUTATION-------------------------------

  hop0=hop0+grad;
  if (hop0>255) hop0=255;
  else if (hop0<1) hop0=1;

  hop_number=4;// prediction corresponds with hop_number=4
  quantum=hop0;//this is the initial predicted quantum, the value of prediction
  small_hop=true;//i supossed initially that hop will be small (3,4,5)
  emin=oc-hop0 ; if (emin<0) emin=-emin;//minimum error achieved

  if (emin>h1/2){ //only enter in computation if emin>threshold


  //positive hops
  //--------------
  if (oc>=hop0)
    {
     //case hop0 (most frequent)
     //--------------------------

     if ((quantum +h1)>255) goto phase3;

     //case hop1 (frequent)
     //---------------------
     error=emin-h1;

     if (error<0) error=-error;

     if (error<emin){

       hop_number=5;
       emin=error;
       quantum+=h1;

       //if (emin<4) goto phase3;
       }
     else goto phase3;


      // case hops 6 to 8 (less frequent)
      // --------------------------------
      for (int i=3;i<6;i++){
        //cache normal
        //hop_value=cache_hops[hop0][h1-4][i];//indexes(i) are 3 to 5

        //cache de 5KB simetrica
        hop_value=255-cache_hops[255-hop0][h1-4][5-i];//indexes are 2 to 0

        error=oc-hop_value;
        if (error<0) error=-error;
        if (error<emin){

          hop_number=i+3;
          emin=error;
          quantum=hop_value;

          //if (emin<4) break;// go to phase 3
          }
         else break;
        }

    }

  //negative hops
  //--------------
  else{


    //case hop0 (most frequent)
    //--------------------------
     if ((quantum -h1)<0)    goto phase3;

     //case hop1 (frequent)
     //-------------------
     error=emin-h1;
     if (error<0) error=-error;

     if (error<emin){

       hop_number=3;
       emin=error;
       quantum-=h1;
       //if (emin<4) goto phase3;
       }
      else goto phase3;

      // case hops 2 to 0 (less frequent)
      // --------------------------------
      for (int i=2;i>=0;i--){

        hop_value=cache_hops[hop0][h1-4][i];//indexes are 2 to 0

        //hop_value=255-cache_hops[255-hop0][h1-4][5-i];//indexes are 2 to 0

        error=hop_value-oc;
        if (error<0) error=-error;
        if (error<emin){

          hop_number=i;
          emin=error;
          quantum=hop_value;
          //if (emin<4) break;// go to phase 3
          }
         else break;
        }
    }

}//endif emin

  //------------- PHASE 3: assignment of final quantized value --------------------------
  phase3:

  result_YUV[y][x]=quantum;
  //*result_signal=quantum; result_signal++;

  //prev_color=quantum;
 //hop_number=5;


  hops[y][x]=hop_number;
  //*result_hops=hop_number; result_hops++;



  //hops_type[hop_number]++;

  //------------- PHASE 4: h1 logic  --------------------------
  if (hop_number>5 || hop_number<3) {small_hop=false; }//true by default

  //if (small_hop==true && last_small_hop==true){
 if (small_hop * last_small_hop){

    if (h1>min_h1) h1--;
    }
  else {

    h1=max_h1;
    }

  last_small_hop=small_hop;
//  printf("%d,",hop_number);
//if (h1<min_h1 || h1>max_h1) printf("fatal error %d \n", h1);

   if (hop_number==5) grad=1;
   else if (hop_number==3) grad=-1;
   else if (!small_hop) grad=0;
if (x>2)
{
    switch (inteligent_discard_mode)
    {
        case 0: // Never soft-line
            softline=false;
            break;
        case 1: // Only Hop0 is soft-line
            if (hop_number>4 || hop_number<4)
                softline=false;
            else
                softline=true;
            break;
        case 2: //  Hop0 & Hop1 is soft-line
            if (hop_number>5 || hop_number<3)
                softline=false;
            else
                softline=true;
            break;
        case 3: // Hop0, Hop1 & Hop2 are softline
            if (hop_number>6 || hop_number<2)
                softline=false;
            else
                softline=true;
            break;
        case 4: // Hop0, Hop1, Hop2 & Hop3 are softline
            if (hop_number>7 || hop_number<1)
                softline=false;
            else
                softline=true;
            break;
        case 5: // Allways everything is softline
            softline=true;
            break;
        default:
            softline=false;
            break;

    }
}
  }
//  printf("\n");
//if (softline) fprintf(stderr, "linea descartada \n ");


return softline;
}
