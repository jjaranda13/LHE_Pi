#pragma once

#include <stdint.h>

typedef struct yuv_image {
	uint8_t *Y_data;
	uint8_t *U_data;
	uint8_t *V_data;
	int width;
	int color_width;
	int height;
	int color_height;
} yuv_image;

yuv_image * allocate_yuv_image(int width, int height);
void free_yuv_image(yuv_image *img);

void upsample_frame_exp(yuv_image *img, yuv_image *img_up);
void upsample_frame_edge(yuv_image *img, yuv_image *img_inter, yuv_image *img_up);
void upsample_frame_bilinear(yuv_image *img, yuv_image *img_inter, yuv_image *img_up);

int save_to( char *filename, yuv_image *img);
