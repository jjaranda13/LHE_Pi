#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "file_decoder.h"
#include "entropic_decoder.h"
#include "quantizer_decoder.h"
#include "player_decoder.h"

typedef struct yuv_image {
	uint8_t* Y_data;
	uint8_t* U_data;
	uint8_t* V_data;
	int width;
	int height;
} yuv_image;

int decode_file(char * filename) {
	
	FILE *file;
	int readed = 1;
	uint8_t *bits = NULL, *hops = NULL, *component_value=NULL;
	uint16_t line_num;
	uint64_t start_pos;
	yuv_image image;


	void play_frame(uint8_t * Y_data, uint8_t * U_data, uint8_t * V_data, int Y_width, int UV_width);
	void close_player();
	int next_quantity;
	int height = 256; // This will be extracted from the file
	int width = 256; // This will be extracted from the file
	image.height = height;
	image.width = width;
	init_player(width, height);
	hops = (uint8_t *)malloc(sizeof(uint8_t)* width);
	bits = (uint8_t *)malloc(sizeof(uint8_t)* width + width);

	image.Y_data = (uint8_t *)malloc(sizeof(uint8_t)* width*height);
	image.U_data = (uint8_t *)malloc(sizeof(uint8_t)* width*height/4);
	image.V_data = (uint8_t *)malloc(sizeof(uint8_t)* width*height/4);

	readed = (int)fopen_s(&file,filename, "rb");
	if (readed != 0) {
		printf("Error opening the file\n");
		return -1;
	}

	for (int i = 0; i < height; i++) {
		uint8_t line_num_high, line_num_low;
		uint8_t zero;
		readed = (int)fread(&zero, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_high, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_low, sizeof(uint8_t), 1, file);
		line_num = line_num_low + (line_num_high << 8);
		start_pos = ftell(file);
		readed = (int)fread(bits, sizeof(uint8_t), width+ width, file);
		int obtained_hops = decode_symbols_entropic(bits, hops, readed, width, &next_quantity);
		fseek(file,(long) start_pos+ next_quantity, SEEK_SET);

		decode_line_quantizer(hops, image.Y_data + (image.width*line_num), obtained_hops);
		printf("Line_num=%05d  width=320 number_of_bytes=%05d obained_hops=%05d start_pos=%03x end_pos=%03x zero_is=%01x\n", (unsigned int)line_num, (unsigned int)next_quantity, (unsigned int)obtained_hops, (unsigned int)start_pos, (unsigned int)ftell(file), zero);
	}
	
	for (int i = 0; i < height/2; i++) {
		uint8_t zero;
		uint8_t line_num_high, line_num_low;
		readed = (int)fread(&zero, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_high, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_low, sizeof(uint8_t), 1, file);
		line_num = line_num_low + (line_num_high << 8);
		start_pos = ftell(file);
		readed = (int)fread(bits, sizeof(uint8_t), width/2, file);
		int obtained_hops = decode_symbols_entropic(bits, hops, readed, width/2, &next_quantity);
		fseek(file, (long)start_pos + next_quantity, SEEK_SET);

		decode_line_quantizer(hops, image.U_data + (image.width/2)*line_num, obtained_hops);
		printf("Line_num=%05d  width=320 number_of_bytes=%05d obained_hops=%05d start_pos=%03x end_pos=%03x zero_is=%01x\n", (unsigned int)line_num, (unsigned int)next_quantity, (unsigned int)obtained_hops, (unsigned int)start_pos, (unsigned int)ftell(file), zero);
	}
	
	for (int i = 0; i < height / 2; i++) {
		uint8_t zero;
		uint8_t line_num_high, line_num_low;
		readed = (int)fread(&zero, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_high, sizeof(uint8_t), 1, file);
		readed = (int)fread(&line_num_low, sizeof(uint8_t), 1, file);
		line_num = line_num_low + (line_num_high << 8);
		start_pos = ftell(file);
		readed = (int)fread(bits, sizeof(uint8_t), width / 2, file);
		int obtained_hops = decode_symbols_entropic(bits, hops, readed, width / 2, &next_quantity);
		fseek(file, (long)start_pos + next_quantity, SEEK_SET);

		decode_line_quantizer(hops, image.V_data + (image.width / 2)*line_num, obtained_hops);
		printf("Line_num=%05d  width=320 number_of_bytes=%05d obained_hops=%05d start_pos=%03x end_pos=%03x zero_is=%01x\n", (unsigned int)line_num, (unsigned int)next_quantity, (unsigned int)obtained_hops, (unsigned int)start_pos, (unsigned int)ftell(file), zero);
	}
	play_frame(image.Y_data, image.U_data, image.V_data, width, width/2);

	
	return 0;

}

