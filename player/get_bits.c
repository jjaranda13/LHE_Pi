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
	load_new_buffer(ctx);
	load_new_buffer(ctx);
}

uint8_t get_bit(get_bits_context * ctx) {

	uint8_t byte, mask = 1;

	byte = show_byte(ctx);
	byte &= mask << 7;
	skip_bits(ctx, 1);

	if (byte == 0)
		return 0;
    return 1;
}

uint8_t get_aligned_byte(get_bits_context * ctx) {

	uint8_t byte;

	finish_byte(ctx);
	byte = show_byte(ctx);
	skip_bits(ctx, 8);

	return byte;
}

void finish_byte(get_bits_context * ctx) {

	if (ctx->buffer_left % 8 != 0)
		skip_bits(ctx, ctx->buffer_left % 8);
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

void skip_bits(get_bits_context * ctx, int number) {

	int prev_buffer_left;
    if (number == 0)
        return;
	else if (number <= ctx->buffer_left)
		ctx->buffer_left -= number;
	else {
        prev_buffer_left = ctx->buffer_left;
		load_new_buffer(ctx);
		ctx->buffer_left -= number-prev_buffer_left;
	}

	return;
}

uint8_t show_byte(get_bits_context * ctx) {

	uint8_t byte;

	if (ctx->buffer_left == 0) {
		load_new_buffer(ctx);
	}
	if (8 <= ctx->buffer_left) {
		byte = ctx->buffer >> (ctx->buffer_left - 8);
	}
	else {
		byte = ctx->buffer << (8 - ctx->buffer_left);
		byte |= ctx->next_buffer >> (32 - (8 -ctx->buffer_left));
	}

	return byte;
}

uint16_t show_short(get_bits_context * ctx) {

	uint16_t number;

	if (ctx->buffer_left == 0) {
		load_new_buffer(ctx);
	}
	if (16 <= ctx->buffer_left) {
		number = ctx->buffer >> (ctx->buffer_left - 16);
	}
	else {
		number = ctx->buffer << (16 - ctx->buffer_left);
		number |= ctx->next_buffer >> (32 - (16 -ctx->buffer_left));
	}

	return number;
}

void load_new_buffer(get_bits_context * ctx) {

    uint8_t bytes[4] = {0};
    uint32_t buffer = 0;
	ctx->buffer = ctx->next_buffer;
	if (!feof(stdin) && fread(bytes, sizeof(uint8_t), 4, ctx->handler) != 4) {
		printf("INFO: Finished reading the stream\n");
		fflush(stdout);
		exit(1);
	}
	buffer |= (uint32_t) bytes[0] << 24;
	buffer |= (uint32_t) bytes[1] << 16;
	buffer |= (uint32_t) bytes[2] << 8;
	buffer |= (uint32_t) bytes[3];

    ctx->next_buffer = buffer;
	ctx->buffer_left = 32;


	return;
}
