#pragma once
/**
* @file upsampler_decoder.c
* @author Francisco Jos� Juan Quintanilla
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

void upsample_line_horizontal(uint8_t * component_value, uint8_t * upsampled_value, int component_value_width, int upsample_value_width) {
	
	float ppx = ((float)upsample_value_width) / (float)component_value_width;
	double int_part, fract_part;
	int prev_index;

	for (int i = 0; i < upsample_value_width; i++) {
		fract_part = modf(i / ppx, &int_part);
		prev_index = (int)int_part;
		int prev_part = (int)(fract_part * 16);

		upsampled_value[i] = (component_value[prev_index ] * (16 - prev_part) + component_value[prev_index] * prev_part) / 16;
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

void resize_component(uint8_t * orig_values, uint8_t * dest_values, int orig_width, int orig_height, int dest_width, int dest_height) {

	/*float x_orig, y_orig, fract_part;
	int first_x_orig, second_x_orig, second_x_part, first_x_part, first_y_orig, second_y_orig, second_y_part, first_y_part;
	float pppx = (float) orig_width / (float)dest_width;
	float pppy = (float) orig_height / (float)dest_height;


	for (int y = 0; y < dest_height; y++) {
		for (int x = 0; x < dest_width; x++) {
			 x_orig = x * pppx;
			 y_orig = y * pppy;
			 fract_part = modff(x_orig, &x_orig);
			 first_x_orig = (int)x_orig;
			 second_x_orig = first_x_orig +1;
			 second_x_part = (int)(fract_part * 16);
			 first_x_part = 16 - second_x_part;

			int value  = (orig_values[y*orig_width+x] * (16 - prev_part) + component_value[prev_index] * prev_part) / 16;

			fract_part = modff(y_orig, &y_orig);
			 first_y_orig = (int)y_orig;
			 second_y_orig = first_y_orig + 1;
			 second_y_part = (int)(fract_part * 16);
			 first_y_part = 16 - second_y_part;

			 
		}
	}*/
	return;
}

void scale_epx(uint8_t *channel, int c_height, int c_width, uint8_t *epx, int umbral) {
	int a, b, c, d, t, e1, e2, e3, e4, p, count, exp_x, exp_y;

	int pppx = 2;
	int pppy = 2;
	int exp_width = c_width * pppx;
	t = umbral;

	for (int y = 0; y < c_height; y++) {
		for (int x = 0; x < c_width; x++) {
			p = channel[y*c_width + x];
			e1 = p;
			e2 = p;
			e3 = p;
			e4 = p;

			a = channel[(y - 1)*c_width + x];
			b = channel[y*c_width + x + 1];
			c = channel[y*c_width + x - 1];
			d = channel[(y + 1)*c_width + x];

			count = 0;
			if (abs(c - a) < t) {
				e1 = (a + c + p) / 3;
				count++;
			}
			if (abs(b - a) < t) {
				e2 = (a + b + p) / 3;
				count++;
			}
			if (abs(d - c) < t) {
				e3 = (d + c + p) / 3;
				count++;
			}
			if (abs(b - d) < t) {
				e4 = (b + d + p) / 3;
				count++;
			}
			if (count >= 3) {
				e1 = p;
				e2 = p;
				e3 = p;
				e4 = p;
			}
			exp_x = x * pppx;
			exp_y = y * pppy;

			epx[exp_y*exp_width + exp_x] = e1;
			epx[exp_y*exp_width + exp_x + 1] = e2;
			epx[(exp_y + 1)*exp_width + exp_x] = e3;
			epx[(exp_y + 1)*exp_width + exp_x + 1] = e4;
		}
	}
}