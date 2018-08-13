/**
* @file get_bits.h
* @author Francisco Jos� Juan Quintanilla
* @date Apr 2018
* @brief Get_bits header.
*
* This module is in charge of feeding to the rest of the codec with the required
* bits. it works by storing information in a buffer to the reads are optimal.
* There is various methods to read from the stream and to skip some parts.
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
* @brief Context structure that hold the data.
*
* This structure holds the information of the put_bits functions. There are three
* parameters required. The file handler, the buffer to store the information in and
* buffer_left which tell us how many bits do we have left from the buffer to read.
* This structure must be passed to any get_bits function.
*/
typedef struct get_bits_context {
	FILE * handler;
	uint32_t buffer;
	uint32_t next_buffer;
	int buffer_left;
} get_bits_context;

/**
* @brief Inizialices the get_bits_context
*
* This function inicialices the context structure using the file passed.
*
* @param file File handler to include in the structure.
* @param ctx  The get_bits context structure to fill.
*/
void init_get_bits(FILE * file, get_bits_context * ctx);

/**
* @brief Obtains a bit from the context structure.
*
* This function returns a bit from the handler in the get_bits_context.
*
* @param ctx  The get_bits context structure to get the bit from
* @return The bit value obtained.
*/
uint8_t get_bit(get_bits_context * ctx);

/**
* @brief Obtains an aligned byte from the context structure.
*
* This function returns a byte from the context. In case there is an
* intermediate byte being processed it is discarded in order to get an
* aligned one.
*
* @param ctx  The get_bits context structure to get the bit from
* @return The byte value obtained.
*/
uint8_t get_aligned_byte(get_bits_context * ctx);

/**
* @brief Discards the rest of the current byte.
*
* This function discards the necesary number of bits up to the aligment
* of the reader with the next byte. It can be used to align the stream.
*
* @param ctx  The get_bits context structure to get the bit from
*/
void finish_byte(get_bits_context * ctx);

/**
* @brief Discards the input stream up to finding a NAL.
*
* This function will discard the stream up to finding a NAL. The next
* avaiable bits will be those after the NAL.
*
* @param ctx  The get_bits context structure to get the bit from.
* @return 0 if everything was allright, anything else if error.
*/
int forward_to_nal(get_bits_context * ctx);

/**
* @brief Show the next bits to come, up to the number.
*
* @param ctx  The get_bits context structure to get the bit from.
* @param number  Number of bits to preview
* @return 0 if everything was allright, anything else if error.
*/
uint8_t show_byte(get_bits_context * ctx);

/**
* @brief Show the next bits to come, up to the number.
*
* @param ctx  The get_bits context structure to get the bit from.
* @param number  Number of bits to preview
* @return 0 if everything was allright, anything else if error.
*/
uint16_t show_short(get_bits_context * ctx);

/**
* @brief Skip the number bits to come, up to the number.
*
* @param ctx  The get_bits context structure to get the bit from.
* @param number  Number of bits to skip.
* @return 0 if everything was allright, anything else if error.
*/
void skip_bits(get_bits_context * ctx, int number);

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS. NOT TO BE CALLED FROM OUTSIDE
/////////////////////////////////////////////////////////////////////////

void load_new_buffer(get_bits_context * ctx);
