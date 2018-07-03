/**
* @file get_bits.c
* @author Francisco Jos� Juan Quintanilla
* @date Apr 2018
* @brief Get_bits implementation.
*
* This module is in charge of feeding to the rest of the codec with the required
* bits. it works by storing information in a buffer to the reads are optimal.
*  There is various methods to read from the stream and to skip some parts.
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/

#include <stdlib.h>

#include "get_bits.h"


void init_get_bits(FILE * file, get_bits_context * ctx) {
	ctx->handler = file;
	ctx->buffer = 0;
	ctx->buffer_left = 0;
}

uint8_t get_bit(get_bits_context * ctx) {

	uint8_t byte, mask = 1;
	uint32_t buffer = ctx->buffer;
	int buffer_left = ctx->buffer_left;

	if (ctx->buffer_left == 0) {
		if (!feof(stdin) && fread(&buffer, sizeof(uint32_t), 1, ctx->handler) != 1)
		{
			printf("INFO: Finished reading  the stream\n");
			fflush(stdout);
			exit(1);
		}
		ctx->buffer = buffer;
		buffer_left = 32;
		byte = buffer;
		mask = mask << (buffer_left - 24 - 1);
		mask = mask & byte;
	}
	else if (buffer_left <= 8) {
		byte = buffer >> 24;
		mask = mask << (buffer_left - 1);
		mask = mask & byte;
	}
	else if (buffer_left <= 16) {
		byte = buffer >> 16;
		mask = mask << (buffer_left - 8 - 1);
		mask = mask & byte;
	}
	else if (buffer_left <= 24) {
		byte = buffer >> 8;
		mask = mask << (buffer_left - 16 - 1);
		mask = mask & byte;
	}
	else if (buffer_left <= 32){
		byte = buffer;
		mask = mask << (buffer_left - 24 - 1);
		mask = mask & byte;
	}
	else {
		printf("ERROR: buffer_left went out of range");
	}
	buffer_left--;
	ctx->buffer_left = buffer_left;
	if (mask != 0) {
		return 1;
	}
	return 0;
}

uint8_t get_aligned_byte(get_bits_context * ctx) {
	
	int buffer_left = 0;
	uint8_t byte = 0;
	uint32_t buffer;

	finish_byte(ctx);
	buffer_left = ctx->buffer_left;
	buffer = ctx->buffer;

	if (buffer_left == 0) {
		if (!feof(stdin) && fread(&buffer, sizeof(uint32_t), 1, ctx->handler) != 1)
		{
			printf("INFO: Finished reading  the stream\n");
			fflush(stdout);
			exit(1);
		}
		ctx->buffer = buffer;
		buffer_left = 32;
		byte = buffer;

	}
	else if (buffer_left == 8) {
		byte = buffer >> 24;
	}
	else if (buffer_left == 16) {
		byte = buffer >> 16;
	}
	else if (buffer_left == 24) {
		byte = buffer >> 8;
	}
	else if (buffer_left == 32) {
		byte = buffer;
	}
	buffer_left -= 8;
	ctx->buffer_left = buffer_left;
	return byte;
}

void finish_byte(get_bits_context * ctx) {
	while (ctx->buffer_left % 8) {
		get_bit(ctx);
	}
	return;
}

int forward_to_nal(get_bits_context * ctx) {

	bool is_found = false;
	int nal_counter = 0;
	uint8_t byte = 0;

	while (!is_found) {
		byte = get_aligned_byte(ctx);
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
	return 0;
}
