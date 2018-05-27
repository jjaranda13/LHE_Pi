#include <fcntl.h>  
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#ifdef _WIN32 
	#include <io.h>  
	#include <windows.h>
#elif __linux__
	#include <unistd.h>
#endif

#include "quantizer_decoder.h"
#include "entropic_decoder.h"
#include "upsampler_decoder.h"
#include "player_decoder.h"
#include "decoder.h"
#include "get_bits.h"

#define BITS_BUFFER_LENGHT 3000

#define Y_STATE 0
#define U_STATE 1
#define V_STATE 2

int decode_stream(int width, int height, FILE * file) {

	int status, subframe = 0, component_state = Y_STATE;
	unsigned int line_debug_counter = 0, frame_debug_counter = 0,  subframe_counter[24] = { 0 };
	bool is_Y[1080] = { false }, is_U[1080] = { false },
		is_V[1080] = { false }, first = true;
	uint8_t *bits = NULL, *hops = NULL;
	yuv_image *img, *img_up;

	init_player(width*2, height*2);
	hops = (uint8_t *)malloc(sizeof(uint8_t)* width);
	bits = (uint8_t *)malloc(sizeof(uint8_t)* BITS_BUFFER_LENGHT);
	img = allocate_yuv_image(width, height);
	img_up = allocate_yuv_image(width*2, height*2);
	if (hops == NULL || bits == NULL || img == NULL || img_up == NULL) {
		printf("Cannot allocate memory\n");
		return(-1);
	}

	fflush(stdin);
	// Thrash the stream until a nal is recieved.
	status = thrash_til_nal(file);
	if (status != 0) {
		printf("Cannot read until the start of the stream\n");
		return(-1);
	}

	// Player loop, never ends until process is killed.
	while (true) {
		int bits_lenght, index = 0;

		bits_lenght = buffer_til_nal(file, bits, BITS_BUFFER_LENGHT);
		while (index < bits_lenght) {
			int readed_hops, readed_bytes, line_num,
				past_subframe;
			uint16_t headers;
			

			headers = bits[index + 1] + (bits[index] << 8);
			index += 2;
			// In the first iteration fill the variables with the same values.
			if (first) {
				status = get_header(headers, &component_state, &line_num, &subframe);
				first = false;
			}
			past_subframe = subframe;
			status = get_header(headers, &component_state, &line_num, &subframe);
			if (status != 0 || line_num > height) {
				printf("Wrong header or line_num too high line_debug_counter=%d\n", line_debug_counter);
				line_num = 0;
			}

			if (is_frame_completed(subframe, past_subframe)) {
				reconstruct_frame(img, is_Y, is_U, is_V, height, width);
				upsample_frame(img, img_up);
				play_frame(img_up->Y_data, img_up->U_data, img_up->V_data, width*2, width);
				reset_control_arrays(subframe_counter, is_Y, is_U, is_V);
				frame_debug_counter++;
			}
			switch (component_state) {
			case Y_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != width) {
					printf("Symbols obtained are not the ones expected\n");
				}
				decode_line_quantizer(hops, img->Y_data + width*line_num, width);
				is_Y[line_num] = true;
				subframe_counter[subframe]++;
				break;
			case U_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width / 2, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != (width / 2)) {
					printf("Symbols obtained are not the ones expected\n");
				}
				decode_line_quantizer(hops, img->U_data + (width / 2)*line_num, width / 2);
				is_U[line_num] = true;
				subframe_counter[subframe]++;
				break;
			case V_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width / 2, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != (width / 2)) {
					printf("Symbols obtained are not the ones expected\n");
				}
				decode_line_quantizer(hops, img->V_data + (width / 2)*line_num, width / 2);
				is_V[line_num] = true;
				subframe_counter[subframe]++;
				break;
			}
		}
	}
	return 0;
}

int decode_stream_2(int width, int height, get_bits_context * ctx) {

	int status, subframe = 0, component_state = Y_STATE;
	unsigned int subframe_counter[8] = { 0 };
	bool is_Y[1080] = { false }, is_U[1080] = { false },
		is_V[1080] = { false }, first = true;
	uint8_t *hops = NULL;
	yuv_image *img, *img_up;

	init_player(width * 2, height * 2);
	hops = (uint8_t *)malloc(sizeof(uint8_t)* width);
	img = allocate_yuv_image(width, height);
	img_up = allocate_yuv_image(width * 2, height * 2);
	if (hops == NULL || img == NULL || img_up == NULL) {
		printf("Cannot allocate memory\n");
		return(-1);
	}
	// Thrash the stream until a nal is recieved.
	status = forward_to_nal(ctx);
	if (status != 0) {
		printf("Cannot read until the start of the stream\n");
		return(-1);
	}

	// Player loop, never ends until process is killed.
	while (true) {

		int readed_hops, line_num, past_subframe;
		uint16_t headers;
		uint8_t header_high;

		header_high = get_aligned_byte(ctx);
		headers = get_aligned_byte(ctx) + (header_high << 8);
		// In the first iteration fill the variables with the same values.
		if (first) {
			status = get_header(headers, &component_state, &line_num, &subframe);
			first = false;
		}
		past_subframe = subframe;
		status = get_header(headers, &component_state, &line_num, &subframe);
		if (status == 1) {
			get_aligned_byte(ctx); // Trash the 0
			get_aligned_byte(ctx); // Trash the 1
			get_aligned_byte(ctx); // Trash the 65

			header_high = get_aligned_byte(ctx);
			headers = get_aligned_byte(ctx) + (header_high << 8);
			status = get_header(headers, &component_state, &line_num, &subframe);

		}
		if (status != 0 || line_num > height) {
			printf("Wrong header or line_num too high ");
			line_num = 0;
		}

		if (is_frame_completed(subframe, past_subframe)) {
			reconstruct_frame(img, is_Y, is_U, is_V, height, width);
			upsample_frame_adaptative(img, img_up);
			handle_window();
			play_frame(img_up->Y_data, img_up->U_data, img_up->V_data, width * 2, width);
			reset_control_arrays(subframe_counter, is_Y, is_U, is_V);
		}
		switch (component_state) {
		case Y_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width);
			if (readed_hops != width) {
				printf("Symbols obtained are not the ones expected\n");
			}
			decode_line_quantizer(hops, img->Y_data + width * line_num, width);
			is_Y[line_num] = true;
			subframe_counter[subframe]++;
			break;
		case U_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width / 2);
			if (readed_hops != (width / 2)) {
				printf("Symbols obtained are not the ones expected\n");
			}
			decode_line_quantizer(hops, img->U_data + (width / 2)*line_num, width / 2);
			is_U[line_num] = true;
			subframe_counter[subframe]++;
			break;
		case V_STATE:
			readed_hops = obtain_symbols_entropic(ctx, hops, width / 2);
			if (readed_hops != (width / 2)) {
				printf("Symbols obtained are not the ones expected\n");
			}
			decode_line_quantizer(hops, img->V_data + (width / 2)*line_num, width / 2);
			is_V[line_num] = true;
			subframe_counter[subframe]++;
			break;
		}
	}
	return 0;
}

int decode_stream_stdin(int width, int height) {
	get_bits_context ctx;
	#ifdef _WIN32 
		int status = _setmode(_fileno(stdin), _O_BINARY);
		if (status == -1) {
			printf("Cannot set mode for stdin\n");
			return(-1);
		}
	#endif

	init_get_bits(stdin, &ctx);

	return decode_stream_2(width, height, &ctx);
}

int decode_stream_file(int width, int height, char * filename) {
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

	return decode_stream_2(width, height, &ctx);
}

int thrash_til_nal(FILE * stream) {

	bool is_found = false;
	int nal_counter = 0;
	size_t readed;
	uint8_t byte;

	while (!is_found) {
		readed = fread(&byte, (size_t)1, (size_t)sizeof(uint8_t), stream);
		if (readed == 0){
			#ifdef _WIN32 
				Sleep(10);
			#elif __linux__
				usleep(10 * 1000);
			#endif	
		}
			
		else {
			switch (nal_counter) {
			case 0:
				if (byte == 0x00)
					nal_counter = 1;
				break;
			case 1:
				if (byte == 0x00)
					nal_counter = 2;
				else
					nal_counter = 0;
				break;
			case 2:
				if (byte == 0x00)
					nal_counter = 3;
				else
					nal_counter = 0;
				break;
			case 3:
				if (byte == 0x01)
					nal_counter = 4;
				else if (byte == 0x00)
					nal_counter = 3;
				else
					nal_counter = 0;
				break;
			case 4:
				if (byte == 0x65)
					is_found = true;
				else
					nal_counter = 0;
				break;
			}
		}
	}
	return 0;
}

int buffer_til_nal(FILE * stream, uint8_t * buffer, int buffer_max_lenght) {

	bool is_finished = false;
	char nal_counter = 0;
	size_t readed;
	uint8_t byte;
	int i = 0;

	while (!is_finished && i < buffer_max_lenght) {

		readed = fread(&byte, 1, sizeof(uint8_t), stream);
		if (readed == 0) {
			#ifdef _WIN32 
				Sleep(10);
			#elif __linux__
				usleep(10 * 1000);
			#endif
		}
		else {
			switch (nal_counter) {
			case 0:
				if (byte == 0x00)
					nal_counter = 1;
				else {
					buffer[i] = byte;
					i++;
				}
				break;
			case 1:
				if (byte == 0x00)
					nal_counter = 2;
				else {
					nal_counter = 0;
					buffer[i] = 0x00;
					buffer[i + 1] = byte;
					i += 2;
				}
				break;
			case 2:
				if (byte == 0x00)
					nal_counter = 3;
				else {
					nal_counter = 0;
					buffer[i] = 0x00;
					buffer[i + 1] = 0x00;
					buffer[i + 2] = byte;
					i += 3;
				}
				break;
			case 3:
				if (byte == 0x01)
					nal_counter = 4;
				else if (byte == 0x00) {
					nal_counter = 3;
					buffer[i] = 0x00;
					i ++;
				}
				else {
					nal_counter = 0;
					buffer[i] = 0x00;
					buffer[i + 1] = 0x00;
					buffer[i + 3] = 0x00;
					buffer[i] = byte;
					i += 4;
				}
				break;
			case 4:
				if (byte == 0x65)
					is_finished = true;
				else {
					nal_counter = 0;
					buffer[i] = 0x00;
					buffer[i + 1] = 0x00;
					buffer[i + 3] = 0x00;
					buffer[i + 4] = 0x01;
					buffer[i + 5] = byte;
					i += 5;
				}
				break;
			default:
				printf("Error obtaining the NAL");
				break;
			}
		}
	}
	return i;
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

void upsample_frame_adaptative(yuv_image * img, yuv_image *img_up) {
	scale_adaptative(img->Y_data, img->height, img->width, img_up->Y_data);
	scale_adaptative(img->U_data, img->height / 2, img->width / 2, img_up->U_data);
	scale_adaptative(img->V_data, img->height / 2, img->width / 2, img_up->V_data);
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
	free((*img).Y_data);
	free((*img).U_data);
	free((*img).V_data);
	free(img);
}
