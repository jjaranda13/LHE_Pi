
#include <stdint.h>
#include <stdlib.h>

#include "image_yuv.h"
#include "upsampler_decoder.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

void to_rgb (yuv_image *img, uint8_t *data);

yuv_image * allocate_yuv_image(int width, int height) {
	yuv_image *img;
	img = (yuv_image *)malloc(sizeof(yuv_image));
	img->width = width;
	img->height = height;
	img->Y_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	img->U_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height / 4);
	img->V_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height / 4);
	if (img->Y_data == NULL || img->U_data == NULL || img->V_data == NULL)
		return NULL;
	else
		return img;
}

void free_yuv_image(yuv_image *img) {
	free(img->Y_data);
	free(img->U_data);
	free(img->V_data);
	free(img);
}

void upsample_frame_exp(yuv_image *img, yuv_image *img_up) {
	scale_epx(img->Y_data, img->height, img->width, img_up->Y_data, THRESHOLD);
	scale_epx(img->U_data, img->height/2, img->width /2, img_up->U_data, THRESHOLD);
	scale_epx(img->V_data, img->height/2, img->width /2, img_up->V_data, THRESHOLD);
}

void upsample_frame_bilinear(yuv_image *img, yuv_image *img_inter, yuv_image *img_up)
{
    for (int y = 0; y< img->height; y++)
    {
        upsample_line_horizontal(img->Y_data+y*img->width, img_inter->Y_data+y*img_inter->width, img->width, img_inter->width);
    }
    for (int y = 0; y< img->height/2; y++)
    {
        upsample_line_horizontal(img->U_data+y*img->width/2, img_inter->U_data+y*img_inter->width/2, img->width/2, img_inter->width/2);
        upsample_line_horizontal(img->V_data+y*img->width/2, img_inter->V_data+y*img_inter->width/2, img->width/2, img_inter->width/2);
    }
    upsample_line_vertical(img_inter->Y_data, img_up->Y_data, img_inter->height, img_up->height, img_inter->width);
    upsample_line_vertical(img_inter->U_data, img_up->U_data, img_inter->height/2, img_up->height/2, img_inter->width/2);
    upsample_line_vertical(img_inter->V_data, img_up->V_data, img_inter->height/2, img_up->height/2, img_inter->width/2);
}

void upsample_frame_edge(yuv_image *img, yuv_image *img_inter, yuv_image *img_up)
{
    if (img->width*2 == img_up->width && img->height*2 == img_up->height)
    {
    	scale_edge_2(img->Y_data, img->height, img->width, img_up->Y_data);
        scale_edge_2(img->U_data, img->height / 2, img->width / 2, img_up->U_data);
        scale_edge_2(img->V_data, img->height / 2, img->width / 2, img_up->V_data);
    }
    else
    {
        upsample_frame_bilinear(img, img_inter, img_up);
    }
    return;
}

int save_to(char *filename, yuv_image *img)
{
	int status;
	uint8_t * data;
	
	data = (uint8_t *) malloc(3*img->width*img->height * sizeof(uint8_t));
	to_rgb (img,data);
	
    status = stbi_write_bmp(filename, img->width, img->height, 3, data);
    if (status == 0)
    {
        fprintf(stderr,"%s:%s:%d:ERR: Could not write the specified %s file \n", __FILE__,__func__ ,__LINE__, filename);
        return 1;
    }

    free(data);	
	return 0;
}

void to_rgb (yuv_image *img, uint8_t *data)
{
	int y, u, v, r, g, b;

	for (int line = 0; line<img->height; line++)
	{
		for (int x = 0; x<img->width; x++)
		{
			y = img->Y_data[line*img->width+x];
			u = img->U_data[(line/2)*(img->width/2)+(x/2)];
			v = img->V_data[(line/2)*(img->width/2)+(x/2)];

			r = (1000*y+1402*(v-128))/1000;
			g = (1000*y-344*(u-128)- 714*(v-128))/1000;
			b = (1000*y+1772*(u-128))/1000;

			r = r < 0 ? 0 : (r > 255? 255 : r);
			g = g < 0 ? 0 : (g > 255? 255 : g);
			b = b < 0 ? 0 : (b > 255? 255 : b);

			data[line*img->width*3+x*3] = (uint8_t)r;
			data[line*img->width*3+x*3+1] = (uint8_t)g;
			data[line*img->width*3+x*3+2] = (uint8_t)b;
		}	
	}
	return;
}
