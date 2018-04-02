#pragma once
/**
* @file video_decoder.h
* @author Francisco José Juan Quintanilla
* @date Jan 2018
* @brief Video decoder for LHE_rpi codec.
*
* This module is the video decoder the player of LHE_Pi codec. It performs
* calculations about motion compensation and video playing.
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/

/**
* @brief Defines the first chunk of the range for the delta calculation.
*/
#define TRAMO_1 52

/**
* @brief Defines the second chunk of the range for the delta calculation.
*/
#define TRAMO_2 204

/**
* @brief Performs the sumation of the delta with the previous frame
*
* This function takes the input delta and performs the sumation with the
* previous values. It stores the result into the frame_encoded pointer.
*
* @param delta_values The input array of delta values of the image.
* @param previous_values The input array of the values of the previous frame.
* @param frame_encoded The pointer in which the generated values are stored
* @param lenght Lenght of the delta_values, previous values data in bytes.
* @warning frame_encoded pointer must be allocated before callign this function.
*/
void suma_delta_scanline(uint8_t * delta_values, uint8_t * previous_values, uint8_t * frame_encoded, int lenght);

/////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS. NOT TO BE CALLED FROM OUTSIDE
/////////////////////////////////////////////////////////////////////////

/**
* @brief Performs the saturation of an int to a uint8_t.
*
* This functions accepts an int as parameter and returns the byte saturated. There
* is no truncation but saturated t the range of a byte. The range is from 255 to 1, 
* ero is not available in the conversion.
*
* @param number Number to saturate
* @return returns the number saturated to byte size minus 0.
*/
uint8_t saturate_to_byte(int number);


