#pragma once
/**
* @file upsampler_decoder.c
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
#include <stdlib.h>
#include <math.h>
#include "upsampler_decoder.h"

void upsample_line_horizontal(uint8_t * component_value, uint8_t * upsampled_value, int component_value_width, int upsample_value_width, int mode) {
	
	float ppx = ((float)upsample_value_width) / (float)component_value_width;
	double int_part, fract_part;
	int prev_index;

	if (mode == BILINEAR) {
		for (int i = 0; i < upsample_value_width; i++) {
			fract_part = modf(i / ppx, &int_part);
			prev_index = (int)int_part;
			int prev_part = (int)(fract_part * 16);

			upsampled_value[i] = (component_value[prev_index] * prev_part + component_value[prev_index + 1] * (16 - prev_part)) / 16;
		}
	}
	else {
	}
	return;
}

void interpolate_scanline_vertical(uint8_t * upsampled_values, int scaline, int prev_scaline, int next_scanline, int img_width) {

	int total_distance = next_scanline - prev_scaline;
	int prev_distance = scaline - prev_scaline;
	int next_distance = next_scanline - scaline;

	for (int i = scaline*img_width; i < (scaline + 1)*img_width; i++) {
		upsampled_values[i] = (upsampled_values[i + (next_distance*img_width)] * prev_distance + upsampled_values[i - (prev_distance*img_width)] * next_distance) / total_distance;
	}
	return;
}
/*
void upsample_line_vertical(uint8_t ** component_value, uint8_t ** upsampled_values, int component_value_height) {

	float ppy = ((float)height) / (float)component_value_height;

	switch (mode)
	{
	case BILINEAR:

		float int_part, fract_part;
		int prev_index;
		for (int i = 0; i < height; i++) {
			fract_part = modf(i / ppy, &int_part);
			prev_index = (int)int_part;
			int prev_part = (int)(fract_part * 16);
			for (int j = 0; i < width; j++) {
				upsampled_values[i][j] = (component_value[prev_part][j] * prev_part + component_value[prev_part + 1][j] * (16 - prev_part)) / 16;
			}
		}
		break;

	default:
		break;
	}
	return;
} */