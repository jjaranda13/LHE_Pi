#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "get_bits.h"

typedef struct yuv_image {
	uint8_t* Y_data;
	uint8_t* U_data;
	uint8_t* V_data;
	int width;
	int height;
} yuv_image;

int decode_stream(int width, int height, FILE * file);
int decode_stream_2(int width, int height, get_bits_context * ctx);
int decode_stream_3(int width, int height, get_bits_context * ctx);

int decode_stream_file(int width, int height, char * filename);
int decode_stream_stdin(int width, int height);
int thrash_til_nal(FILE * stream);
int buffer_til_nal(FILE * stream, uint8_t * buffer, int buffer_max_lenght);
int get_header(uint16_t header, int *state, int *line_num, int *subframe);
void reconstruct_frame(yuv_image *img, bool is_Y[1080], bool is_U[1080], bool is_V[1080], int height, int width);
bool is_frame_completed(int subframe, int past_subframe);
void find_available_lines(int line, bool is_component[1080], int line_lenght, int *upper_line, int *lower_line);
void reset_control_arrays(unsigned int subframe_counter[8], bool is_Y[1080], bool is_U[1080], bool is_V[1080]);
yuv_image * allocate_yuv_image(int width, int height);
void free_yuv_image(yuv_image *  img);
void upsample_frame(yuv_image * img, yuv_image *img_up);
void upsample_frame_adaptative(yuv_image * img, yuv_image *img_up);
