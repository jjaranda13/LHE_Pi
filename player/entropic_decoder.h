#pragma once
/**
* @file entropic_decoder.h
* @author Francisco José Juan Quintanilla
* @date Sep 2017
* @brief Entropic decoder for LHE_rpi codec.
*
* This module is the entropic decoder the player of LHE_Pi codec. It uses an
* static huffman tree and an automatic RLC encoding. It works at scanline level
* and process a whole scanline. The huffman table is hardcoded an it is:
*   HOP_O  = 1
*   HOP_P1 = 01
*   HOP_N1 = 001
*   HOP_P2 = 0001
*   HOP_N2 = 00001
*   HOP_P3 = 000001
*   HOP_N3 = 0000001
*   HOP_P4 = 00000001
*   HOP_N4 = 000000001
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "get_bits.h"

/**
* @brief Number of bits to represent the RLE.
*
* This parameter specifies the number of bits uses to represent the number of
* hops0. In case it is equal to 4 the maximum number of hop0 that will be coded
* as RLE is 16. It defaults to 4. It MUST be equal to the one used at the coder
*/
#define RLC_LENGHT_INI 4

/**
* @brief Number of consecutive Hop0 required to start RLE
*
* This parameter specifies the number consecutive hop0 that will activate the RLE mode.
* Defaults to 7. It must te equal to the value used in the coder.
*/
#define CONDITION_LENGHT_INI 7

/**
* @brief List of symbols for the hops.
*
* This table shows the equivalent number that is used to represent the hops now on.
*/
#define HOP_0 4
#define HOP_N1 3
#define HOP_P1 5
#define HOP_N2 2
#define HOP_P2 6
#define HOP_N3 1
#define HOP_P3 7
#define HOP_N4 0
#define HOP_P4 8

/**
* @brief Defines whether jump to evens is activated.
* 
*/
#define JUMP_TO_EVENS

/**
* @brief Allocates memory for the entropic decoded
*
* @param width Width of the image. It is used to allocate memory.
* @return hops Pointer to the memory allocated to be used in hops
*/
uint8_t* allocate_entropic_decoder(int width);

/**
* @brief Frees the memory of the hops array.
*
* @param hops Pointer to the memory allocated for hops.
*/
void free_entropic_decoder(uint8_t* hops);


/**
* @brief Decodes a scanline from binary data into symbols.
*
* This function fills an array of hops using the bitstream. It implements the
* static huffman table and the automatic RLE. The pointer to hop must be
* inizialized before calling the function.

* @param bits The pointer to the bitstream of data.
* @param hops The resulting array of hops is stored in this pointer.
* @param bytes_lenght The lenght in bytes of the bitstream.
* @return Lenght of the array of hops returned. it should be img_width minus one.

* @warning hops pointer must be allocated before calling this function.
*/
int decode_line_entropic( uint8_t * bytes , uint8_t * hops,  int bytes_lenght);

/**
* @brief Decodes a scanline from binary data into symbols.
*
* This function fills an array of hops using the bitstream. It implements the
* static huffman table and the automatic RLE. The pointer to hop must be
* inizialized before calling the function.

* @param bits The pointer to the bitstream of data.
* @param hops The resulting array of hops is stored in this pointer.
* @param bytes_lenght The lenght in bytes of the bitstream.
* @param hops_lenght The number of Hops that must be returned.
* @param readed_bytes The number of bytes that were readed from the get_bits_context.
* @return Lenght of the array of hops returned. it should be img_width minus one.

* @warning hops pointer must be allocated before calling this function.
*/
int decode_symbols_entropic(uint8_t * bytes, uint8_t * hops, int bytes_lenght, int hops_lenght, int * readed_bytes);

/**
* @brief Obtains a number of hops from a get_bits_context
*
* This function obtains an array of hops from the get_bits_context. It returns
* the actual number of hops readed. The number of hops to be readed must be
* supplied to the function.

* @param ctx The pointer to the get_bits_structure.
* @param hops The resulting array of hops is stored in this pointer.
* @param hops_lenght The number of Hops that must be returned.
* @return Lenght of the array of hops returned. it should be img_width minus one.

* @warning hops pointer must be allocated before calling this function.
*/
int obtain_symbols_entropic(get_bits_context * ctx, uint8_t * hops, int hops_lenght);
/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS. NOT TO BE CALLED FROM OUTSIDE
/////////////////////////////////////////////////////////////////////////

/**
* test_bit
* This function is used to check if there is a one in a determined possiton
* inside a byte. It is used several times within the code.
*
* @param data The byte which will be tested.
* @param possition The byte possition taht is tested.
* @return True if the byte contains a 1 in the specifies posstion false otherwise.
*/
bool test_bit(uint8_t data, int possition);

/**
* set_bit
* This function is used to set to 1 a determined possition inside a byte. It is
* used several times within the code.
*
* @param data The byte which will be modified.
* @param possition The byte possition that will be set to 1.
* @return The modified byte.
*/
uint8_t set_bit(uint8_t data, int possition);

/**
* get_data
* This function obtains a bit from the bitstream input. It work with an idex of
* individual bits.It checks whether the required bit is inside the renge of the
* input bistream.
*
* @param bits Whole bitstream pointer
* @param sub_index Index in bits of the @p bits which is wanted.
* @param bytes_lenght Total lenght of the bitstream @p bits.
* @return One if the bit is one, zero if the required bit is zero, -1 if error.
*/
int get_data(uint8_t * bits, int sub_index);
/**
* get_hop
* Returns the hop number using the number of zeros that were readed previously.
* This can be done as the huffman three is static and defined previously.
*
* @param zeros_since_a_one Symbol that represents the hop.
* @return The symbol of the hop matching the code.
*/
uint8_t get_hop(int zeros_since_a_one);

/**
* get_rlc_number
* This function returns the number of hops0 that were coded using RLC. It takes
* from the bitstream the specified number of bits determined.
*
* @param bits Whole bitstream pointer
* @param sub_index Index in bits of the @p bits which is wanted.
* @param bytes_lenght Total lenght of the bitstream @p bits.
* @param rlc_lenght Number of bits used to represent the number of hops0.
* @return The number of hops0 codified used RLE.
*/
int get_rlc_number(uint8_t * bits, int * sub_index, int rlc_lenght);

/**
* get_rlc_number_get_bits
* This function returns the number of hops0 that were coded using RLC. It takes
* from the get_bits_context.
*
* @param ctx The get bits context to get the bits from.
* @param rlc_lenght Number of bits used to represent the number of hops0.
* @return The number of hops0 codified used RLE.
*/
int get_rlc_number_get_bits(get_bits_context * ctx, int rlc_lenght);
/**
* add_hop0
* This function adds to the array of symbols the specified number of hop0
* symbols.
*
* @param hops A pointer to the array of hop symbols.
* @param hops_counter Current index of of the array @p hops
* @param rlc_number Number of
* @param count Specifies how many hops0 must be introduced.
* @return Void.
*/
void add_hop0(uint8_t * hops, int *hops_counter, int count);

uint8_t get_first_hop(uint16_t data, int * bits_used, bool is_pre_huffman);
