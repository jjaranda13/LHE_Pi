#pragma once
/**
* @file entropic_decoder.h
* @author Francisco Jos� Juan Quintanilla
* @date Sep 2017
* @brief Quantizer decoder for LHE_rpi codec.
*
* This module is the quatizer decoder the player of LHE_Pi codec. It uses a
* right pixel prediction in conjunction with the LHE algorithm.
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/
#include <stdint.h>

/**
* @brief Maximum value of hop1 allowed.
*/
#define MAX_H1 10
/**
* @brief minimum value of hop1 allowed.
*/
#define MIN_H1 4
/**
* @brief The start value of h1. The formula that sould eb followed is "start_h1 = (max_h1 + min_h1) / 2;"
*/
#define START_H1 7
/**
* @brief Maximum value of the ratioS that are used to expand the hops accorss the range.
*/
#define MAX_R 2.7f
/**
* @brief Minimum value of the ratioS that are used to expand the hops accorss the range.
*/
#define MIN_R 1.0f
/**
* @brief Percentage of the rest of the range that will be covered.
*/
#define RANGE 0.70f

/**
* @brief defines if the gradient technique should be used. Comment to deactivate,
*uncomment to activate
*/
#define IS_GRADIENT

/**
* @brief Defines the behaviour of the maximum hops. If activated the maximum
* hops lead to the maximum range. If not the behaviour is the normal jumping
* using the normal ratio. Comment to deactivate, uncomment to activate
*/
// #define IS_MAX_HOPS

/**
* @brief Defines the prediction that will be used for the first pixel. As there is no
* hops lead to the maximum range. If not the behaviour is the normal jumping
* using the normal ratio.
*/
#define INIT_PREDICTION 127

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


uint8_t cache_hops[256][7][3]; //5KB cache [Y][h1][hop_number]

/**
* @brief Allocates memory for the quantizer.
*
* @param width Width of the image. It is used to allocate memory.
* @return component_value Pointer to the memory allocated to be used to store component_value.
*/
uint8_t* allocate_quantizer_decoder(int width);

/**
* @brief Frees the memory of the component_value array.
*
* @param hops Pointer to the memory allocated for component_value.
*/
void free_quantizer_decoder(uint8_t* component_value);

/**
* @brief Decodes a scanline from hops to component values(could be Y, U or V).
*
* This function takes the hop array which must be width-1 and the first
* component of the line. It generates a an array of lenght width containing
* the component values for the scanline.
*
* @param hops The input array of hops generated by the entropic.
* @param component_value The value of the first pixel of the scanline.
* @param hops_lenght The lenght  of the hops.

* @warning component_value pointer must be allocated before callign this function.
*/
void decode_line_quantizer(uint8_t * hops, uint8_t * component_value, int hops_lenght);

int decode_line_quantizer2(uint8_t * hops, uint8_t * component_value, int hops_lenght);

void build_hop_cache();

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS. NOT TO BE CALLED FROM OUTSIDE
/////////////////////////////////////////////////////////////////////////

uint8_t calculate_hop (uint8_t hop, unsigned char hop0, signed char h1);
/**
* @brief Indicates if the current hop is a small hop
*
* This function determines whether a hop is small. This is used in next
* computations as input
*
* @param actual_hop The hop to be determined if it is small.
* @return True if it is small false otherwise
*/
bool is_small_hop(uint8_t hop);

/**
* @brief Adapts h1 for the next pixel.
*
* This function performs the h1 adaptationn using the current hop and the
* last_small_hop. The hop1 returned will be used as hop1 for the next pixel
* of the scaline.
*
* @param h1 Hop1 value for the current pixel.
* @param small_hop Indicates if the acutal pixel is small.
* @param last_small_hop Indicates if the last pixel were small.
* @return Adapted Hop1 to be used in the next pixel
*/
unsigned char adapt_h1(unsigned char  h1, bool small_hop, bool last_small_hop);

/**
* @brief Calculates the gradient value for the next iteration.
*
* This function performs the gradient calculation suing the actual hop. The
* value returned can be used as the gradient for the enxt iteration.
*
* @param current_hop The hop of the current pixel.
* @param small_hop Tell the function if the current hop is small
* @param prev_gradient the gradient used in the current pixel.
* @return gradient to be used in the next pixel
*/
char adapt_gradient(uint8_t current_hop, bool small_hop, char prev_gradient);

/**
* @brief Calcules the ratios the ranges for LHE longer hops.
*
* This functions performs the calculation of the ratios used to multiply h1.
*
* @param hop0 Hop0 value, it is the prediction used for the hop.
* @param hop1 Hop1 value for the current pixel.
* @param positive_ratio Pointer to return the value of positive_ratio
* @param negative_ratio Pointer to return the value of negative_ratio
*/
void calculate_ranges(uint8_t hop0, uint8_t hop1, double * positive_ratio, double * negative_ratio);
