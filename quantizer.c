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

#include "main.h"
#include "quantizer.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

// la cache realmente podria ser de 10KB cache_hops[255][7][6]; e incluso de la mitad (5KB) pues es simetrica
unsigned char cache_hops[255][7][6]; //10KB cache


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_quantizer(){
///this function pre computes the cache
///la cache es cache[hop0][h1][hop] = 10.000 bytes = 10 KB

if (DEBUG) printf ("enter in init_quantizer()...");

for (int hop0=0;hop0<=255;hop0++)
{
 for (int hop1=4; hop1<=10;hop1++)
 {
 //ratio for possitive hops. max ratio=3
 double rpos = min (3.0f,pow(0.8f*(255-hop0)/hop1,1.0f/3.0f));

 //ratio for negative hops. max ratio=3
 double rneg = min(3.0f,pow(0.8f*(hop0)/hop1,1.0f/3.0f));

 //compute hops 0,1,2,6,7,8
 //hops 3,4,5 are known
 cache_hops[hop0][hop1-4][0] = (unsigned char)(hop0-hop1*rneg*rneg*rneg);
 cache_hops[hop0][hop1-4][1] = (unsigned char)(hop0-hop1*rneg*rneg);
 cache_hops[hop0][hop1-4][2] = (unsigned char)(hop0-hop1*rneg);
 cache_hops[hop0][hop1-4][3] = (unsigned char)(hop0+hop1*rpos);
 cache_hops[hop0][hop1-4][4] = (unsigned char) (hop0+hop1*rpos*rpos);
 cache_hops[hop0][hop1-4][5] = (unsigned char)(hop0+hop1*rpos*rpos*rpos);

 }//for hop1
}//for hop0

}// end function
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void quantize_scanline(char **orig_YUV, int y,int width, char **hops,char **result_YUV) {
/// this function quantize the luminances or chrominances of one scanline
/// inputs : orig_YUV (which can be result_Y, result_U or result_V), line, width,
/// outputs: hops, result_YUV (which can be result_Y, result_U or result_V)

if (DEBUG) printf ("enter in quantize_scanline()...");
char max_h1=10;
char min_h1=4;
char start_h1=(max_h1+min_h1)<<1;
char h1=start_h1;
char last_hop=4;

bool last_small_hop=true; //last hop was small
bool small_hop=true;//current hop is small
char hop0=0; //prediction
char emin=255;//error min
char error=0;//computed error
char oc=orig_YUV[y][0];//original color
char quantum=oc; //quantum value
char hop_number=4;
unsigned char hop_value=0;//data from cache


//first hop is original signal value, and also result_YUV
hops[y][0]=quantum;
result_YUV[y][0]=quantum;





for (int x=1;x<width;x++)
  {

  // --------------------- PHASE 1: PREDICTION---------------------------------------------------------
  //original color
  oc=orig_YUV[y][x];

  if (y>0 && x!=width-1)
    {

    if (last_small_hop) hop0=(result_YUV[y][x-1]+result_YUV[y-1[x]]+result_YUV[y-1][x+1])/3;
    else hop0=(result_YUV[y][x-1]+result_YUV[y-1][x+1])>>1;

    }
  else if (y>0)//x=width
    {
    hop0=(result_YUV[y][x-1]+result_YUV[y-1][x])>>1;
    }
  else //y=0
   {
    hop0=result_YUV[y][x-1];
   }


  //-------------------------PHASE 2: HOPS COMPUTATION-------------------------------
  hop_number=4;
  quantum=hop0;
  small_hop=true;

  //positive hops
  //--------------
  if (oc>=hop0)
    {
     //case hop0 (most frequent)
     emin=oc-hop0 ;
     if (emin<4) goto phase3;// only enter in computation if emin>=4

     //case hop1 (frequent)
     error=emin-h1;
     if (error<0) error=-error;
     if (error<emin)
       {
       hop_number=5;
       emin=error;
       quantum+=h1;
       if (emin<4) goto phase3;
       }

      // case hops 6 to 8 (less frequent)
      for (int i=3;i<6;i++)
        {
        hop_value=cache_hops[hop0][h1-4][i];//indexes are 3 to 5
        error=oc-hop_value;
        if (error<0) error=-error;
        if (error<emin)
          {
          hop_number=i+3;
          emin=error;
          quantum=hop_value;

          if (emin<4) break;// go to phase 3
          }
        }

    }

  //negative hops
  //--------------
  else
    {
    //case hop0 (most frequent)
     emin=hop0-oc;
     if (emin<4) goto phase3;// only enter in computation if emin>=4

     //case hop1 (frequent)
     error=emin-h1;
     if (error<0) error=-error;
     if (error<emin)
       {
       hop_number=3;
       emin=error;
       quantum+=h1;
       if (emin<4) goto phase3;
       }

      // case hops 2 to 0 (less frequent)
      for (int i=2;i>=0;i--)
        {
        hop_value=cache_hops[hop0][h1-4][i];//indexes are 2 to 0
        error=oc-hop_value;
        if (error<0) error=-error;
        if (error<emin)
          {
          hop_number=i;
          emin=error;
          quantum=hop_value;
          if (emin<4) break;// go to phase 3
          }
        }
    }




  //------------- PHASE 3: assignment of final quantized value --------------------------
  phase3:

  result_YUV[line][x]=quantum;
  hops[line][x]=hop_number;

  //------------- PHASE 4: h1 logic  --------------------------
  if (hop_number>5 || hop_number<3) small_hop=false; //true by default

  if (small_hop && last_small_hop)
    {
    if (h1>min_h1) h1--;
    }
  else
    {
    h1=max_h1;
    }
  last_small_hop=small_hop;
  }
}
