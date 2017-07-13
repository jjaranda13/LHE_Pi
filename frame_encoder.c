




#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include "globals.h"
#include "imgUtil.h"



void encode_frame()
{

load_frame("../LHE_Pi/img/lena.bmp");
printf("frame loaded");
rgb2yuv(rgb,orig_Y,orig_U,orig_V,rgb_channels);

printf("rgb2yuv done");
for (int i=0;i<height_orig;i++)
  {
  down_avg_horiz(orig_Y,orig_down_Y,i,2);
  }
printf("down done");
yuv2rgb();
save_frame("../LHE_Pi/img/kk.bmp", width_orig/2, height_orig/2, 1, orig_down_Y);
printf("save done");


}
