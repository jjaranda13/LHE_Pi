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

#define THRESHOLD 38


void upsample_line_horizontal(uint8_t * component_value, uint8_t * upsampled_value, int component_value_width, int upsample_value_width);
void upsample_line_vertical(uint8_t * component_value, uint8_t * upsampled_value, int component_value_height, int upsample_value_height, int width);
void interpolate_scanline_vertical(uint8_t * upsampled_values, int scaline, int prev_scaline, int next_scanline, int img_width);
void scale_epx(uint8_t *channel, int c_height, int c_width, uint8_t *epx, int umbral);
void scale_edge_1(uint8_t * origin, int ori_height, int ori_width, uint8_t * destination);
void scale_edge_2(uint8_t * origin, int ori_height, int ori_width, uint8_t * destination);
