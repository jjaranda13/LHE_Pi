#pragma once
/**
* @file upsampler_decoder.h
* @author Francisco José Juan Quintanilla
* @date Sep 2017
* @brief Upsampler for LHE_rpi codec.
*
* This module is the upsampler of the lhe_pi codec. It upsamples the image 
* using selectable methods.
*
* For more:
* @see https://github.com/jjaranda13/LHE_Pi
*/
#include <stdint.h>

#define BILINEAR 0
#define EPX 1


void upsample_line_horizontal(uint8_t * component_value, uint8_t * upsampled_value, int component_value_width, int upsample_value_width, int mode);
void interpolate_scanline_vertical(uint8_t * upsampled_values, int scaline, int prev_scaline, int next_scanline, int img_width, int mode);
//void upsample_line_vertical(uint8_t ** component_value, uint8_t ** upsampled_values, int component_value_height);

