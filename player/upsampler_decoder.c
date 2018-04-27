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
		upsampled_values[i] = (upsampled_values[i + (next_distance*img_width)] *prev_distance + upsampled_values[i - (prev_distance*img_width)] * next_distance) / total_distance;
	}
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

void scale_adaptative(uint8_t * origin, int ori_height, int ori_width, uint8_t * destination) {

	int dst_y, dst_x, ori_y, ori_x;
	int a, b, c, d;
	int dst_height = ori_height * 2;
	int dst_width = ori_width * 2;

	dst_y = 0;
	ori_y = dst_y / 2;
	for (dst_x = 0; dst_x < dst_width - 1; dst_x++) {
		ori_x = dst_x / 2;
		if (dst_x % 2) {
			destination[dst_y*dst_width + dst_x] = (origin[ori_y*ori_width + ori_x] + origin[ori_y*ori_width + ori_x + 1]) / 2;
		}
		else {
			destination[dst_y*dst_width + dst_x] = origin[ori_y*ori_width + ori_x];
		}
	}
	ori_x = dst_width - 1;
	destination[dst_y*dst_width + dst_width - 1] = origin[ori_y*ori_width + ori_x];

	dst_y = dst_height - 1;
	ori_y = dst_y / 2;
	for (dst_x = 0; dst_x < dst_width - 1; dst_x++) {
		ori_x = dst_x / 2;
		if (dst_x % 2) {
			destination[dst_y*dst_width + dst_x] = (origin[ori_y*ori_width + ori_x] + origin[ori_y*ori_width + ori_x + 1]) / 2;
		}
		else {
			destination[dst_y*dst_width + dst_x] = origin[ori_y*ori_width + ori_x];
		}
	}
	ori_x = dst_width - 1;
	destination[dst_y*dst_width + dst_width - 1] = origin[ori_y*ori_width + ori_x];

	dst_x = 0;
	ori_x = dst_x / 2;
	for (dst_y = 1; dst_y < dst_height - 1; dst_y++) {
		ori_y = dst_y / 2;
		if (dst_y % 2) {
			destination[dst_y*dst_width + dst_x] = (origin[ori_y*ori_width + ori_x] + origin[(ori_y + 1)*ori_width + ori_x]) / 2;
		}
		else {
			destination[dst_y*dst_width + dst_x] = origin[ori_y*ori_width + ori_x];
		}
	}

	dst_x = dst_width - 1;
	ori_x= dst_x / 2;
	for (dst_y = 1; dst_y < dst_height - 1; dst_y++) {
		ori_y = dst_y / 2;
		if (dst_y % 2) {
			destination[dst_y*dst_width + dst_x] = (origin[ori_y*ori_width + ori_x] + origin[(ori_y + 1)*ori_width + ori_x]) / 2;
		}
		else {
			destination[dst_y*dst_width + dst_x] = origin[ori_y*ori_width + ori_x];
		}
	}

	for (dst_y = 1; dst_y < dst_height - 1; dst_y++) {
		
		for (dst_x = 1; dst_x < dst_width - 1; dst_x++) {

			ori_x = dst_x / 2;
			ori_y = dst_y / 2;

			if((dst_x % 2 == 0 && dst_y % 2 == 0)) {
				destination[dst_y*dst_width + dst_x] = origin[ori_y*ori_width + ori_x];
			}
			else if (dst_x % 2 && dst_y % 2) {
				a = origin[ori_y*ori_width + ori_x];
				b = origin[ori_y*ori_width + ori_x + 1];
				c = origin[(ori_y + 1)*ori_width + ori_x];
				d = origin[(ori_y + 1)*ori_width + ori_x + 1];

				if (abs(a - d) >= abs(b - c)) {
					destination[dst_y*dst_width + dst_x] = (b + c) / 2;
				}
				else {
					destination[dst_y*dst_width + dst_x] = (a + d) / 2;
				}
			}
		}
	}

	for (dst_y = 1; dst_y < dst_height - 1; dst_y++) {
		for (dst_x = 1; dst_x < dst_width - 1; dst_x++) {
			if ((dst_x % 2 == 0 && dst_y % 2) || (dst_x % 2  && dst_y % 2 == 0)) {
				a = destination[(dst_y - 1)*dst_width + dst_x];
				b = destination[dst_y*dst_width + dst_x + 1];
				c = destination[(dst_y + 1)*dst_width + dst_x];
				d = destination[dst_y*dst_width + dst_x - 1];

				if (abs(a - c) >= abs(b - d)) {
					destination[dst_y*dst_width + dst_x] = (b + d) / 2;
				}
				else {
					destination[dst_y*dst_width + dst_x] = (a + c) / 2;
				}
			}
		}
	}
}
