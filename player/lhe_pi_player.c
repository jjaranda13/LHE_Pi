#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "lhe_pi_player.h"
#include "entropic_decoder_test.h"
#include "quantizer_decoder_test.h"
#include "player_decoder_test.h"

#include "entropic_decoder.h"
#include "quantizer_decoder.h"
#include "upsampler_decoder.h"
#include "player_decoder.h"

typedef struct player_options {
	int width;
	int height;
	bool is_file;
	char *filename;
	char *address;
	int port;
} player_options;

typedef struct process_mem {
	uint8_t* bits;
	uint8_t* hops;
	uint8_t* component_value;
} process_mem;

typedef struct line_mem {
	process_mem Y_mem;
	process_mem U_mem;
	process_mem V_mem;
} line_mem;

typedef struct yuv_image {
	uint8_t* Y_data;
	uint8_t* U_data;
	uint8_t* V_data;
	int width;
	int height;
} yuv_image;


int main(int argc, char *argv[]) {

	//test_entropic_decoder();
	//test_quantizer_decoder();
	//test_player_decoder();

	player_options options;
	line_mem line_memory;
	yuv_image image;

	bool* is_line_recieved = (bool *)malloc(sizeof(bool) * MAX_LINE_WIDTH);

	for (int i = 0; i < MAX_LINE_WIDTH; i++) {
		is_line_recieved[i] = false;
	}

	if (parse_cmd(argc, argv, &options) != 0) {
		quit();
	}
	allocate_line_mem(&line_memory, MAX_LINE_WIDTH, MAX_BYTES_LENGHT);
	allocate_yuv_image(&image, options.width, options.height);
	init_player(options.width, options.height);


	while (1) { // TODO -> while I have data in the socket and there no sigterm signal. This should be changed
		int is_there_frame;
		int line_num;
		int component_value_lenght;
	
		// Streamer should gives me:
		// Pointer to the data. bits_Y[linea][byte] bits_U[linea][byte] bits_V[linea][byte]
		// The number of bytes the data has tam_bytes_Y[linea] tam_bytes_U[linea] tam_bytes_V[linea]
		// Flag when a frame is ready

		if (is_there_frame) {
			for (int i = 0; i < line_num; i++) {
				component_value_lenght = decode_line_entropic(bits_Y[i], line_memory.Y_mem.hops, tam_bytes_Y[i]);
				decode_line_quantizer(line_memory.Y_mem.hops, line_memory.Y_mem.component_value, component_value_lenght);
				upsample_line_horizontal(line_memory.Y_mem.component_value, image.Y_data + (image.width*i), component_value_lenght, image.width, BILINEAR);
			}
			for (int i = 0; i < line_num; i++) {
				component_value_lenght = decode_line_entropic(bits_U[i], line_memory.U_mem.hops, tam_bytes_U[i]);
				decode_line_quantizer(line_memory.U_mem.hops, line_memory.U_mem.component_value, component_value_lenght);
				upsample_line_horizontal(line_memory.U_mem.component_value, image.U_data + (image.width*i), component_value_lenght, image.width, BILINEAR);
			}
			play_frame(image.Y_data, image.U_data, image.V_data, image.width, image.width / 2);
			is_there_frame = 0;
		}





		/* This code is usefull for a next iteration rightnow we will use the whole image as input.
		int hops_lenght = decode_line_entropic(line_memory.Y_mem.bits, line_memory.Y_mem.hops, bytes_lenght);
		decode_line_quantizer(line_memory.Y_mem.hops, line_memory.Y_mem.component_value, hops_lenght);

		if (is_line_recieved[line]) { // I have recieved a line that is aready here.
			for (int i = 0; i < image.height; i++) { // Upsample the image gaps
				if (is_line_recieved[i] == false) {
					// TODO: interpolate the line using two specified
				}
			}
			play_frame(image.Y_data, image.U_data, image.V_data, image.width, image.width/2); 
			
			for (int i = 0; i < MAX_LINE_WIDTH; i++) { // Reset the is_line_recieved array with zeros
				is_line_recieved[i] = false;
			}
			upsample_line_horizontal(line_memory.Y_mem.component_value, image.Y_data+ (line*image.width), width);
			upsample_line_horizontal(line_memory.U_mem.component_value, image.U_data + (line*image.width), width);
			is_line_recieved[line] = true;
		}
		else {
			upsample_line_horizontal(line_memory.Y_mem.component_value, image.Y_data + (line*image.width), width);
			is_line_recieved[line] = true;
		
		}
		*/
	}

	free_line_mem(&line_memory);
	free_yuv_image(&image);
	close_player();
	free(is_line_recieved);

	return 0;

}

int parse_cmd(int argc, char *argv[], player_options * options) {

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-w") == 0) {
			options->width = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--height") == 0 || strcmp(argv[i], "-h") == 0) {
			options->height = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--filename") == 0 || strcmp(argv[i], "-f") == 0) {
			options->filename = argv[i + 1];

		}
		else if (strcmp(argv[i], "--address") == 0 || strcmp(argv[i], "-a") == 0) {
			options->address = argv[i + 1];
		}
		else if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
			options->port = atoi(argv[i + 1]);
		}
	}
	if (options->filename != NULL && (options->address != NULL || options->port != NULL)) {
		printf("There is a filename suplied. There cannot be adress or port either.\n");
		return -1;
	}
	if (options->address != NULL && options->port == NULL) {
		printf("A port must be specified for the application to connect.\n");
		return -1;
	}
	if (options->address != NULL && options->port == NULL) {
		printf("An address must be specified for the application to connect.\n");
		return -1;
	}
	if (options->width == NULL) {
		printf("Width of the representation must be supplied\n");
		return -1;
	}
	if (options->height == NULL) {
		printf("Height of the representation must be supplied\n");
		return -1;
	}
	if (options->address == NULL && options->filename == NULL) {
		printf("It is needes to be suplied either an ip or a filename.\n");
		return -1;
	}
	if (options->filename != NULL) {
		options->is_file = true;
	}
	else {
		options->is_file = false;
	}
	return 0;
}

void allocate_line_mem(line_mem * line_memory,int max_line_width, int max_bytes_lenght) {

	(*line_memory).Y_mem.bits = (uint8_t *)malloc(sizeof(uint8_t)*max_bytes_lenght);
	(*line_memory).U_mem.bits = (uint8_t *)malloc(sizeof(uint8_t)*max_bytes_lenght/2);
	(*line_memory).V_mem.bits = (uint8_t *)malloc(sizeof(uint8_t)*max_bytes_lenght/2);

	(*line_memory).Y_mem.hops = allocate_entropic_decoder(max_line_width);
	(*line_memory).U_mem.hops = allocate_entropic_decoder(max_line_width / 2);
	(*line_memory).V_mem.hops = allocate_entropic_decoder(max_line_width / 2);

	(*line_memory).Y_mem.component_value = allocate_quantizer_decoder(max_line_width);
	(*line_memory).U_mem.component_value = allocate_quantizer_decoder(max_line_width / 2);
	(*line_memory).V_mem.component_value = allocate_quantizer_decoder(max_line_width / 2);
	return;
}

void free_line_mem(line_mem *  line_memory) {
	free((*line_memory).Y_mem.bits);
	free((*line_memory).U_mem.bits);
	free((*line_memory).V_mem.bits);

	free_entropic_decoder((*line_memory).Y_mem.hops);
	free_entropic_decoder((*line_memory).U_mem.hops);
	free_entropic_decoder((*line_memory).V_mem.hops);

	free_quantizer_decoder((*line_memory).Y_mem.component_value);
	free_quantizer_decoder((*line_memory).U_mem.component_value);
	free_quantizer_decoder((*line_memory).V_mem.component_value);
	return;
}

void allocate_yuv_image(yuv_image *  img, int width, int height) {
	(*img).width = width;
	(*img).height = height;
	(*img).Y_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height);
	(*img).U_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height/4);
	(*img).V_data = (uint8_t *)malloc(sizeof(uint8_t)*width*height/4);
	return;
}

void free_yuv_image(yuv_image *  img) {
	free((*img).Y_data);
	free((*img).U_data);
	free((*img).V_data);
}

