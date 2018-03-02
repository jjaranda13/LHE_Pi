#include <fcntl.h>  
#include <io.h>  
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>

#include "quantizer_decoder.h"
#include "entropic_decoder.h"
#include "upsampler_decoder.h"
#include "player_decoder.h"
#include "decoder.h"


#define BITS_BUFFER_LENGHT 3000

#define Y_STATE 1
#define U_STATE 2
#define V_STATE 3

int decode_stream(int width, int height, FILE * file) {

	int status, line_debug_counter=0;
	bool is_subframe[24] = { false }, is_Y[1080] = { false },
		is_U[1080] = { false }, is_V[1080] = { false }, first = true;
	uint8_t *bits = NULL, *hops = NULL;
	yuv_image *img;

	init_player(width, height);
	hops = (uint8_t *)malloc(sizeof(uint8_t)* width);
	bits = (uint8_t *)malloc(sizeof(uint8_t)* BITS_BUFFER_LENGHT);
	img = allocate_yuv_image(width, height);
	if (hops == NULL || bits == NULL || img == NULL) {
		printf("Cannot allocate memory\n");
		return(-1);
	}
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
				past_component_state, subframe, component_state,
				past_subframe;
			uint16_t headers;
			

			headers = bits[index + 1] + (bits[index] << 8);
			index += 2;
			// In the first iteration fill the variables with the same values.
			if (first) {
				status = get_header(headers, &component_state, &line_num, &subframe);
				first = false;
			}
			past_component_state = component_state;
			past_subframe = subframe;
			status = get_header(headers, &component_state, &line_num, &subframe);
			if (status != 0 || line_num > height) {
				printf("Wrong header or line_num too high line_debug_counter=%d\n", line_debug_counter);
				line_num = 240;
				//return -1;
			}

			if (is_frame_completed(component_state, past_component_state, subframe, past_subframe, is_subframe)) {
				reconstruct_frame(img, is_Y, is_U, is_V, height, width);
				play_frame(img->Y_data, img->U_data, img->V_data, width, width / 2);
				reset_control_arrays(is_subframe, is_Y, is_U, is_V);
			}
			switch (component_state) {
			case Y_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != width) {
					printf("Symbols obtained are not the ones expected\n");
					//return -1;
				}
				decode_line_quantizer(hops, img->Y_data + width*line_num, width);
				is_Y[line_num] = true;
				is_subframe[subframe] = true;
				break;
			case U_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width / 2, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != (width / 2)) {
					printf("Symbols obtained are not the ones expected\n");
					//return -1;
				}
				decode_line_quantizer(hops, img->U_data + (width / 2)*line_num, width / 2);
				is_U[line_num] = true;
				is_subframe[subframe] = true;
				break;
			case V_STATE:
				readed_hops = decode_symbols_entropic(bits + index, hops, bits_lenght - index, width / 2, &readed_bytes);
				index += readed_bytes;
				if (readed_hops != (width / 2)) {
					printf("Symbols obtained are not the ones expected\n");
					//return -1;
				}
				decode_line_quantizer(hops, img->V_data + (width / 2)*line_num, width / 2);
				is_V[line_num] = true;
				is_subframe[subframe] = true;
				break;
			}
			line_debug_counter++;
		}
	}
	return 0;
}

int decode_stream_stdin(int width, int height) {
	int status;

	status = _setmode(_fileno(stdin), _O_BINARY);
	if (status == -1) {
		printf("Cannot set mode for stdin\n");
		return(-1);
	}
	return decode_stream(width, height, stdin);
}

int decode_stream_file(int width, int height, char * filename) {
	int status;
	FILE * file;

	status = (int)fopen_s(&file, filename, "rb");
	if (status != 0) {
		printf("Error opening the file\n");
		return -1;
	}
	return decode_stream(width, height, file);
}

int thrash_til_nal(FILE * stream) {

	bool is_found = false;
	int nal_counter = 0;
	size_t readed;
	uint8_t byte;

	while (!is_found) {
		readed = fread(&byte, (size_t)1, (size_t)sizeof(uint8_t), stream);
		if (readed == 0) {
			Sleep(10);
			break;
		}

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
	return 0;
}

int buffer_til_nal(FILE * stream, uint8_t * buffer, int buffer_max_lenght) {

	bool is_finished = false;
	int nal_counter = 0;
	size_t readed;
	uint8_t byte;
	uint32_t i = 0;

	while (!is_finished && i < buffer_max_lenght) {

		readed = fread(&byte, 1, sizeof(uint8_t), stream);
		if (readed == 0) {
			Sleep(10);

			break;
		}

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
			else {
				nal_counter = 0;
				buffer[i] = 0x00;
				buffer[i + 1] = 0x00;
				buffer[i + 2] = 0x00;
				buffer[i + 3] = byte;
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
				buffer[i + 2] = 0x00;
				buffer[i + 3] = 0x01;
				buffer[i + 4] = byte;
				i += 5;
			}
			break;
		}
	}
	return i;
}

int get_header(uint16_t header, int *state, int *line_num, int *subframe) {
	int frame_type;
	
	frame_type = header >> 14;
	*line_num = header & ~(0xC000);
	*subframe = *line_num % 24;
	if (frame_type == 3) {
		*state = Y_STATE;
	}
	else if (frame_type == 2) {
		*state = U_STATE;
	}
	else if (frame_type == 1) {
		*state = V_STATE;
	}
	else {
		printf("Inconsistent line number found.\n");
		return -1;
	}
	return 0;
}

bool is_frame_completed(int component_state, int past_component_state, int subframe, int past_subframe, bool is_subframe[24]) {
	bool cond1, cond2, cond3;
	cond1 = component_state == Y_STATE;
	cond2 = subframe != past_subframe;
	cond3 = is_subframe[subframe];
	if (cond1 && cond2 && cond3)
		return true;
	else
		return false;
}

void reconstruct_frame(yuv_image *img, bool is_Y[1080], bool is_U[1080], bool is_V[1080], int height, int width) {
	int upper_line, lower_line;
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
				interpolate_scanline_vertical(img->Y_data, i, lower_line, upper_line, width, BILINEAR);
			}
			else {
				// Leave past scanline, wont be too bad as images have temporal coorelation.
			}
		}
	}
	for (int i = 0; i < (height / 2); i++) {
		if (!is_U[i]) {
			find_available_lines(i, is_U, height / 2, &upper_line, &lower_line);
			if (lower_line != -1 && upper_line == -1) {
				memcpy((void*)(img->U_data + width*i), (void*)(img->U_data + width*lower_line), (width / 2));
			}
			else if (lower_line == -1 && upper_line != -1) {
				memcpy((void*)(img->U_data + width*i), (void*)(img->U_data + width*upper_line), (width / 2));
			}
			else if (lower_line != -1 && upper_line != -1) {
				interpolate_scanline_vertical(img->U_data, i, lower_line, upper_line, width / 2, BILINEAR);
			}
			else {
				// Leave past scanline, wont be too bad as images have temporal coorelation.
			}
		}
	}
	for (int i = 0; i < (height / 2); i++) {
		if (!is_V[i]) {
			find_available_lines(i, is_V, height / 2, &upper_line, &lower_line);
			if (lower_line != -1 && upper_line == -1) {
				memcpy((void*)(img->V_data + width*i), (void*)(img->V_data + width*lower_line), (width / 2));
			}
			else if (lower_line == -1 && upper_line != -1) {
				memcpy((void*)(img->V_data + width*i), (void*)(img->V_data + width*upper_line), (width / 2));
			}
			else if (lower_line != -1 && upper_line != -1) {
				interpolate_scanline_vertical(img->V_data, i, lower_line, upper_line, width / 2, BILINEAR);
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

void reset_control_arrays(bool is_subframe[24], bool is_Y[1080], bool is_U[1080], bool is_V[1080]) {

	memset((void *)is_subframe, false, sizeof(bool) * 24);
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
