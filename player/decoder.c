
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
	#include <io.h>
	#include <windows.h>
#elif __linux__
	#include <unistd.h>
#endif
#include <sys/time.h>

#include "quantizer_decoder.h"
#include "entropic_decoder.h"
#include "upsampler_decoder.h"
#include "player_decoder.h"
#include "decoder.h"
#include "get_bits.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


#define BITS_BUFFER_LENGHT 3000

#define Y_STATE 0
#define U_STATE 1
#define V_STATE 2

double timeval_diff(struct timeval *a, struct timeval *b);


int decode_stream(bool fullscreen, get_bits_context * ctx,  char * output_file) {

	bool is_output_file;
	char filename[120];
	int status, subframe = 0, component_state = Y_STATE, frame_counter = 0,
        width, height, pppx, pppy;
	unsigned int subframe_counter[8] = { 0 };
	bool is_Y[1080] = { false }, is_U[1080] = { false },
		is_V[1080] = { false }, first = true;
	uint8_t *hops = NULL;
	uint32_t headers = 0;
	yuv_image *img, *img_up, *img_inter;
	struct timeval t_ini, t_fin;

	build_hop_cache();
	
	if(strcmp(output_file, "NULL") == 0) {
		is_output_file = false;	
	} else {
		is_output_file = true;
	}
	// Thrash the stream until a nal is recieved.
	status = forward_to_nal(ctx);
	if (status != 0) {
		printf("Cannot read until the start of the stream\n");
		return(-1);
	}
	headers = get_aligned_byte(ctx);
	headers = get_aligned_byte(ctx) + (headers << 8);
    headers = get_aligned_byte(ctx) + (headers << 8);
    headers = get_aligned_byte(ctx) + (headers << 8);
	status = get_stream_header(headers, &width, &height , &pppx, &pppy);

	init_player(width, height, fullscreen);
	hops = (uint8_t *)malloc(sizeof(uint8_t)* width/pppx);
	img = allocate_yuv_image(width/pppx, height/pppy);
	img_inter = allocate_yuv_image(width, height/pppy);
	img_up = allocate_yuv_image(width, height);

	if (hops == NULL || img == NULL || img_up == NULL) {
		printf("Cannot allocate memory\n");
		return(-1);
	}
	gettimeofday(&t_ini, NULL);
	
	// Player loop, never ends until process is killed.
	while (true) {

		int readed_hops, line_num, past_subframe;
		double secs;
		
		uint16_t headers = 0;

		headers = get_aligned_byte(ctx);
		headers = get_aligned_byte(ctx) + (headers << 8);
		// In the first iteration fill the variables with the same values.
		if (first) {
			status = get_header(headers, &component_state, &line_num, &subframe);
			if (status != 0) {
				printf("Wrong header\n");
			}
			first = false;
		}
		past_subframe = subframe;
		status = get_header(headers, &component_state, &line_num, &subframe);

		if (status == 1) {
			get_aligned_byte(ctx); // Trash the 0
			get_aligned_byte(ctx); // Trash the 1
			get_aligned_byte(ctx); // Trash the 65
			get_aligned_byte(ctx); // Trash the Stream header 1
			get_aligned_byte(ctx); // Trash the Stream header 2
			get_aligned_byte(ctx); // Trash the Stream header 3
			get_aligned_byte(ctx); // Trash the Stream header 4

			headers = get_aligned_byte(ctx);
			headers = get_aligned_byte(ctx) + (headers << 8);
			status = get_header(headers, &component_state, &line_num, &subframe);

		}
		if (status != 0 || line_num > height) {
			printf("Wrong header or line_num too high ");
			line_num = 0;
		}

		if (is_frame_completed(subframe, past_subframe)) {
			frame_counter++;
			reconstruct_frame(img, is_Y, is_U, is_V, height/pppy, width/pppx);
			upsample_frame_adaptative(img, img_inter, img_up);

			handle_window();
			play_frame(img_up->Y_data, img_up->U_data, img_up->V_data, width, width/2);
			reset_control_arrays(subframe_counter, is_Y, is_U, is_V);
			
			if (frame_counter % 60 == 0) {
				gettimeofday(&t_fin, NULL);
				secs = timeval_diff(&t_fin, &t_ini);
				printf("INFO: fps are %f\n", 60/secs);
				gettimeofday(&t_ini, NULL);
			}
			if (is_output_file) {
				sprintf(filename,output_file,frame_counter);
				status = save_to( filename, img_up);
				if (status != 0) {
					return 1;	
				}
			}
		}
		switch (component_state) {
		case Y_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/pppx);
			decode_line_quantizer(hops, img->Y_data + (width/pppx) * line_num, readed_hops);
			is_Y[line_num] = true;
			subframe_counter[subframe]++;
			break;
		case U_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/(pppx*2));
			decode_line_quantizer(hops, img->U_data + (width/(pppx*2))*line_num, readed_hops);
			is_U[line_num] = true;
			subframe_counter[subframe]++;
			break;
		case V_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/(pppx*2));
			decode_line_quantizer(hops, img->V_data + (width/(pppx*2))*line_num, readed_hops);
			is_V[line_num] = true;
			subframe_counter[subframe]++;
			break;
		}
	}
	return 0;
}

int decode_stream_stdin(bool fullscreen, char * output_file) {
	get_bits_context ctx;
	#ifdef _WIN32
		int status = _setmode(_fileno(stdin), _O_BINARY);
		if (status == -1) {
			printf("Cannot set mode for stdin\n");
			return(-1);
		}
	#endif

	init_get_bits(stdin, &ctx);

	return decode_stream(fullscreen, &ctx, output_file);
}

int decode_stream_file(bool fullscreen, char * filename, char * output_file) {
	int status;
	FILE * file;
	get_bits_context ctx;
	#ifdef _WIN32
		status = (int)fopen_s(&file, filename, "rb");
	#elif __linux__
		file = fopen(filename, "rb");
		if (file == NULL) {
			status = 1;
		}
		else {
			status = 0;
		}
	#endif

	if (status != 0) {
		printf("Error opening the file\n");
		return -1;
	}
	init_get_bits(file, &ctx);

	return decode_stream(fullscreen, &ctx, output_file);
}

int get_header(uint16_t header, int *state, int *line_num, int *subframe) {
	int frame_type;

	frame_type = header >> 13;
	*line_num = header & ~(0xE000);
	*subframe = *line_num % 8;
	
	if (frame_type == 3) {
		*state = Y_STATE;
	}
	else if (frame_type == 2) {
		*state = U_STATE;
	}
	else if (frame_type == 1) {
		*state = V_STATE;
	}
	else if (frame_type == 0 && *line_num == 0) {
		// It everything is 0 it means that we found the first two bytes of a NAL.
		return 1;
	}
	else {
		printf("Inconsistent line number found.\n");
		return -1;
	}
	return 0;
}

int get_stream_header(uint32_t headers, int *width, int *height , int *pppx, int *pppy)
{
    uint8_t aux;
    uint16_t aux16;

    aux = (uint8_t) (headers>>28);
    *pppx = aux;

    aux = (uint8_t) (headers>>24);
    aux &= ~(0xf0);
    *pppy = aux;

    aux16 = (uint16_t) (headers>>12);
    aux16 &= ~(0xf000);
    *width = aux16;

    aux16 = (uint16_t) headers;
    aux16 &= ~(0xf000);
    *height = aux16;

    return 0;
}

bool is_frame_completed(int subframe, int past_subframe) {

	bool cond1 = subframe < past_subframe - 5;
	if (cond1)
		return true;
	else
		return false;
}

void reconstruct_frame(yuv_image *img, bool is_Y[1080], bool is_U[1080], bool is_V[1080], int height, int width) {
	int upper_line, lower_line;
	int UVwidth = width / 2;
	int UVheight = height / 2;
	for (int i = 0; i < height; i++) {
		if (!is_Y[i]) {
			find_available_lines(i, is_Y, height, &upper_line, &lower_line);
			if (lower_line != -1 && upper_line == -1) {
				memcpy((void*)(img->Y_data + width*i), (void*)(img->Y_data + width*lower_line), width);
			}
			else if (lower_line == -1 && upper_line != -1) {
				memcpy((void*)(img->Y_data + width*i), (void*)(img->Y_data + width*upper_line), width);
			}
			else if (lower_line != -1 && upper_line != -1) {
				interpolate_scanline_vertical(img->Y_data, i, lower_line, upper_line, width);
			}
			else {
				// Leave past scanline, wont be too bad as images have temporal coorelation.
			}
		}
	}
	for (int i = 0; i < UVheight; i++) {
		if (!is_U[i]) {
			find_available_lines(i, is_U, UVheight, &upper_line, &lower_line);
			if (lower_line != -1 && upper_line == -1) {
				memcpy((void*)(img->U_data + UVwidth *i), (void*)(img->U_data + UVwidth *lower_line), UVwidth);
			}
			else if (lower_line == -1 && upper_line != -1) {
				memcpy((void*)(img->U_data + UVwidth *i), (void*)(img->U_data + UVwidth *upper_line), UVwidth);
			}
			else if (lower_line != -1 && upper_line != -1) {
				interpolate_scanline_vertical(img->U_data, i, lower_line, upper_line, UVwidth);
			}
			else {
				// Leave past scanline, wont be too bad as images have temporal coorelation.
			}
		}
	}
	for (int i = 0; i < UVheight; i++) {
		if (!is_V[i]) {
			find_available_lines(i, is_V, UVheight, &upper_line, &lower_line);
			if (lower_line != -1 && upper_line == -1) {
				memcpy((void*)(img->V_data + UVwidth *i), (void*)(img->V_data + UVwidth *lower_line), UVwidth);
			}
			else if (lower_line == -1 && upper_line != -1) {
				memcpy((void*)(img->V_data + UVwidth *i), (void*)(img->V_data + UVwidth *upper_line), UVwidth);
			}
			else if (lower_line != -1 && upper_line != -1) {
				interpolate_scanline_vertical(img->V_data, i, lower_line, upper_line, UVwidth);
			}
			else {
				// Leave past scanline, wont be too bad as images have temporal coorelation.
			}
		}
	}
}
void find_available_lines(int line, bool is_component[1080], int line_lenght, int *upper_line, int *lower_line) {
	*lower_line = -1;
	*upper_line = -1;
	for (int i = line - 1; i >= 0; i--) {
		if (is_component[i]) {
			*lower_line = i;
			break;
		}
	}
	for (int i = line + 1; i < line_lenght; i++) {
		if (is_component[i]) {
			*upper_line = i;
			break;
		}
	}
	return;
}

void upsample_frame(yuv_image * img, yuv_image *img_up) {
	scale_epx(img->Y_data, img->height, img->width, img_up->Y_data, THRESHOLD);
	scale_epx(img->U_data, img->height/2, img->width /2, img_up->U_data, THRESHOLD);
	scale_epx(img->V_data, img->height/2, img->width /2, img_up->V_data, THRESHOLD);
}

void upsample_frame_adaptative(yuv_image * img, yuv_image * img_inter, yuv_image *img_up)
{	
    if (img->width*2 == img_up->width && img->height*2 == img_up->height)
    {
    	scale_adaptative(img->Y_data, img->height, img->width, img_up->Y_data);
        scale_adaptative(img->U_data, img->height / 2, img->width / 2, img_up->U_data);
        scale_adaptative(img->V_data, img->height / 2, img->width / 2, img_up->V_data);
    }
    else
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
    return;
}

void reset_control_arrays(unsigned int subframe_counter[8], bool is_Y[1080], bool is_U[1080], bool is_V[1080]) {

	memset((void *)subframe_counter, false, sizeof(unsigned int) * 8);
	memset((void *)is_Y, false, sizeof(bool) * 1080);
	memset((void *)is_U, false, sizeof(bool) * 1080);
	memset((void *)is_V, false, sizeof(bool) * 1080);
	return;
}

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

void free_yuv_image(yuv_image *  img) {
	free(img->Y_data);
	free(img->U_data);
	free(img->V_data);
	free(img);
}

double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}

int save_to( char *filename, yuv_image *img)
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
