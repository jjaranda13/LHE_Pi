#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "get_bits.h"
#include "image_yuv.h"



int decode_stream(bool fullscreen, get_bits_context * ctx,  char * output_file);
int decode_stream_file(bool fullscreen, char * filename, char * output_file);
int decode_stream_stdin(bool fullscreen, char * output_file);


int get_header(uint16_t header, int *state, int *line_num, int *subframe);
int get_stream_header(uint32_t headers, int *width, int *height , int *pppx, int *pppy);
void reconstruct_frame(yuv_image *img, bool is_Y[1080], bool is_U[1080], bool is_V[1080], int height, int width);
bool is_frame_completed(int subframe, int past_subframe);
void find_available_lines(int line, bool is_component[1080], int line_lenght, int *upper_line, int *lower_line);
void reset_control_arrays(unsigned int subframe_counter[8], bool is_Y[1080], bool is_U[1080], bool is_V[1080]);





