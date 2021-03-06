#include <stdint.h>

void rgb2yuv(unsigned char *rgb, int rgb_channels);
void yuv2rgb(unsigned char **y, unsigned char **u, unsigned char ** v, int channels, int width, int height, char *data,int yuvmodel);
double get_PSNR_Y(unsigned char **result_Y, unsigned char ** orig_Y, int height , int width);
void interpolate_scanline(uint8_t ** values, int scanline, int prev_scaline, int next_scanline, int values_width);
void interpolate_adaptative(uint8_t ** origin, int ori_height, int ori_width, uint8_t ** destination);
void scale_epx(unsigned char **channel, int c_height, int c_width, unsigned char **epx,int umbral);
void scale_epx_H2(unsigned char **channel, int height, int down_width, unsigned char **epx,int umbral);
void yuv444toyuv420(unsigned char** Y, unsigned char** U, unsigned char** V, int width, int height);
