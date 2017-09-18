#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "quantizer_decoder.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

uint8_t* init_quantizer_decoder(int width) {

	return (uint8_t *)malloc(sizeof(uint8_t)*width);
}
void close_quantizer_decoder(uint8_t* component_value) {
	free(component_value);
	return;
}

void decode_line_quantizer(uint8_t * hops, uint8_t * component_value, int width, uint8_t original_component) {

	int  h1 = START_H1;
	uint8_t actual_hop = 0;
	uint8_t hop0 = 0;
	bool last_small_hop = true;
	double positive_ratio, negative_ratio;

	for (int x = 0; x < width; x++) {
		actual_hop = hops[x];
		if (x == 0) {
			hop0 = 127;
		}
		else {
			hop0 = component_value[x - 1];
		}
		calculate_ranges(hop0, h1, &positive_ratio, &negative_ratio);

		switch (actual_hop)
		{
		case HOP_0:
			component_value[x] = hop0;
			break;
		case HOP_P1:
			component_value[x] = hop0 + h1;
			break;
		case HOP_P2:
			component_value[x] = hop0 + (uint8_t)(h1*positive_ratio);
			break;
		case HOP_P3:
			component_value[x] = hop0 + (uint8_t)(h1*positive_ratio*positive_ratio);
			break;
		case HOP_P4:
			component_value[x] = hop0 + (uint8_t)(h1*positive_ratio*positive_ratio*positive_ratio);
			break;
		case HOP_N1:
			component_value[x] = hop0 - h1;
			break;
		case HOP_N2:
			component_value[x] = hop0 - (uint8_t)(h1*negative_ratio);
			break;
		case HOP_N3:
			component_value[x] = hop0 - (uint8_t)(h1*negative_ratio*negative_ratio);
			break;
		case HOP_N4:
			component_value[x] = hop0 - (uint8_t)(h1*negative_ratio*negative_ratio*negative_ratio);
			break;
		default:
			printf("ERROR: Unexpected symbol were found.");
			break;
		}
		h1 = adapt_h1(h1, actual_hop, &last_small_hop);
	}
	
}
int adapt_h1(int h1, uint8_t actual_hop, bool * last_small_hop) {

	bool small_hop = true;

	if (actual_hop > HOP_P1 || actual_hop < HOP_N1) {
		small_hop = false;
	}
	if (small_hop * *last_small_hop) {

		if (h1 > MIN_H1) {
			h1--;
		}
	}
	else {
		h1 = MAX_H1;
	}
	*last_small_hop = small_hop;
	return h1;
}

void calculate_ranges(uint8_t hop0, uint8_t hop1, double * positive_ratio, double * negative_ratio) {

	*positive_ratio = min(MAX_R, pow(RANGE*((255 - hop0) / hop1), 1.0f / 3.0f));
	*positive_ratio = max(MIN_R, *positive_ratio);

	*negative_ratio = min(MAX_R, pow(RANGE*(hop0 / hop1), 1.0f / 3.0f));
	*negative_ratio = max(MIN_R, *negative_ratio);
	return;
}