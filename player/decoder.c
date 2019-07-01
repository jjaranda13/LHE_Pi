
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
#include "image_yuv.h"

#define BITS_BUFFER_LENGHT 3000

#define Y_STATE 0
#define U_STATE 1
#define V_STATE 2

double timeval_diff(struct timeval *a, struct timeval *b);


int decode_stream(bool fullscreen, get_bits_context * ctx,  char * output_file) {
	bool is_output_file;
	char filename[120];
	int status, subframe = 0,frame, component_state = Y_STATE, frame_counter = 0, line_counter = 0, width, height, pppx, pppy;
	bool *is_Y = NULL, *is_U = NULL, *is_V = NULL, first = true;
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
	is_Y = (bool *)malloc (sizeof(bool)* height);
	is_U = (bool *)malloc (sizeof(bool)* height/pppy);
	is_V = (bool *)malloc (sizeof(bool)* height/pppy);
	img = allocate_yuv_image(width/pppx, height/pppy);
	img_inter = allocate_yuv_image(width, height/pppy);
	img_up = allocate_yuv_image(width, height);

	if (hops == NULL || img == NULL || img_up == NULL || is_Y == NULL || is_U == NULL|| is_V == NULL) {
		printf("Cannot allocate memory\n");
		return(-1);
	}
	gettimeofday(&t_ini, NULL);

	// Player loop, never ends until process is killed.
	while (true) {

		int readed_hops, line_num = -1, past_subframe, past_frame;
		double secs;

		uint16_t headers = 0;

        headers = get_aligned_byte(ctx);
        headers = (get_aligned_byte(ctx)<<8) + headers;

		// In the first iteration fill the variables with the same values.
		if (first) {
			status = get_header(headers, height/pppy, &component_state, &line_num, &subframe, &frame);
			if (status != 0) {
				printf("Wrong header\n");
			}
			first = false;
		}
		past_subframe = subframe;
		past_frame = frame;
		status = get_header(headers, height/pppy, &component_state, &line_num, &subframe, &frame);

		if (status == 1) {
			get_aligned_byte(ctx); // Trash the 0
			get_aligned_byte(ctx); // Trash the 1
			get_aligned_byte(ctx); // Trash the 65
			get_aligned_byte(ctx); // Trash the Stream header 1
			get_aligned_byte(ctx); // Trash the Stream header 2
			get_aligned_byte(ctx); // Trash the Stream header 3
			get_aligned_byte(ctx); // Trash the Stream header 4

            headers = get_aligned_byte(ctx);
            headers = (get_aligned_byte(ctx)<<8) + headers;
			status = get_header(headers, height/pppy, &component_state, &line_num, &subframe, &frame);

		}

		if (status != 0 || line_num > height) {
			printf("Wrong header or line_num too high status=%d line_num=%d",status, line_num);
			line_num = 0;
		}

		if (is_frame_completed(subframe, past_subframe, frame, past_frame)) {
			frame_counter++;
			if (line_counter <= height && line_counter >= 0.2 * height){
                reconstruct_frame(img, is_Y, is_U, is_V, height/pppy, width/pppx);
                upsample_frame_edge(img, img_inter, img_up);

                handle_window();
                play_frame(img_up->Y_data, img_up->U_data, img_up->V_data, width, width/2);
			}
			reset_control_arrays(is_Y, is_U, is_V, height, pppy);
			line_counter = 0;

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
		line_counter++;
		switch (component_state) {
		case Y_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/pppx);
			decode_line_quantizer(hops, img->Y_data + (width/pppx) * line_num, readed_hops);
			is_Y[line_num] = true;
			break;
		case U_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/(pppx*2));
			decode_line_quantizer(hops, img->U_data + (width/(pppx*2))*line_num, readed_hops);
			is_U[line_num] = true;
			break;
		case V_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width/(pppx*2));
			decode_line_quantizer(hops, img->V_data + (width/(pppx*2))*line_num, readed_hops);
			is_V[line_num] = true;
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
	freopen(NULL, "rb", stdin);

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

int get_header(uint16_t header, int max_line_num, int *line_type, int *line_num, int *subframe, int *frame){

    int aux = header / (max_line_num/2);

    *frame = aux/4; //header / (max_line_num*2);

    if(aux % 4 == 0 || aux % 4 == 1) {
        *line_type = Y_STATE;
        *line_num = header - 2*max_line_num*(*frame);
    }
    else if (aux % 4 == 2) {
        *line_type = U_STATE;
        *line_num = header - 2*max_line_num*(*frame) - max_line_num;
    }
    else if (aux % 4 == 3) {
        *line_type = V_STATE;
        *line_num = header - 2*max_line_num*(*frame) - 3*max_line_num/2;
    }
    *subframe = *line_num%8;
    if (header == 0)
    {
        return 1;
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

bool is_frame_completed(int subframe, int past_subframe,int frame,int past_frame) {

	//bool cond1 = subframe < past_subframe - 5;
	bool cond2 = frame != past_frame;

	if (cond2){
		return true;
	}
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

void reset_control_arrays(bool is_Y[], bool is_U[], bool is_V[], int height,int pppy) {

	memset((void *)is_Y, false, sizeof(bool) * height);
	memset((void *)is_U, false, sizeof(bool) * height/pppy);
	memset((void *)is_V, false, sizeof(bool) * height/pppy);
	return;
}

double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}
