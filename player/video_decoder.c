#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "video_decoder.h"

void suma_delta_scanline(uint8_t * delta_values, uint8_t * previous_values, uint8_t * frame_encoded, int lenght)
{

	int delta, signo, image;
	bool negative;

	for (int x = 0; x < lenght; x++) {
		delta = delta_values[x];
		delta = delta - 128;
		if (delta < 0) {
			negative = true;
			delta = -delta;
		}
		else {
			negative = false;
		}
		if (delta < TRAMO_1) {
			if (!negative) {
				image = previous_values[x] + delta;
			}
			else {
				image = previous_values[x] - delta;
			}
		}
		else if (delta <= TRAMO_1 + (TRAMO_2 - TRAMO_1) / 2) {
			delta = (delta - TRAMO_1) * 2;
			delta += TRAMO_1;
			if (!negative) {
				image = previous_values[x] + delta;
			}
			else {
				image = previous_values[x] - delta;
			}
		}
		else {
			delta = (delta - (TRAMO_2 - TRAMO_1) / 2 - TRAMO_1) * 4;
			delta += TRAMO_2;
			if (!negative) {
				image = previous_values[x] + delta;
			}
			else {
				image = previous_values[x] - delta;
			}
		}
		frame_encoded[x] = saturate_to_byte(image);
	}
}

uint8_t saturate_to_byte(int number) {
	if (number > 255)
	{
		number = 255;
	}
	else if (number < 1)
	{
		number = 1;
	}
	return (uint8_t)number;
}
