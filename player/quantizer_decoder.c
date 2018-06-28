#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "quantizer_decoder.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

uint8_t* allocate_quantizer_decoder(int width) {
	return (uint8_t *)malloc(sizeof(uint8_t)*width);
}

void free_quantizer_decoder(uint8_t* component_value) {
	free(component_value);
	return;
}

void decode_line_quantizer(uint8_t * hops, uint8_t * component_value, int hops_lenght) {

	char gradient = 0;
	unsigned char  h1 = START_H1, hop0;
	bool last_small_hop = true, small_hop;
	double positive_ratio, negative_ratio;
	uint8_t current_hop;

	for (int x = 0; x < hops_lenght; x++) {
		current_hop = hops[x];

		if (x == 0) {
			hop0 = INIT_PREDICTION;
		}
		else {
			hop0 = component_value[x - 1];
		}
		// Checks that there wont be overflow plus if gradient is activated.
		#ifdef IS_GRADIENT
		if ((hop0 + gradient >= 1) && (hop0 + gradient <= 255))
			hop0 += gradient;
		#endif

		calculate_ranges(hop0, h1, &positive_ratio, &negative_ratio);

		switch (current_hop)
		{
		case HOP_0:
			component_value[x] = hop0;
			break;
		case HOP_P1:
			component_value[x] = hop0 + h1;
			break;
		case HOP_P2:
			component_value[x] = hop0 + (int)ceil(h1*positive_ratio);
			break;
		case HOP_P3:
			component_value[x] = hop0 + (int)ceil(h1*positive_ratio*positive_ratio);
			break;
		case HOP_P4:
			#ifdef IS_MAX_HOPS
			component_value[x] = 255;
			#else
			component_value[x] = hop0 + (int)ceil(h1*positive_ratio*positive_ratio*positive_ratio);
			#endif
			break;
		case HOP_N1:
			component_value[x] = hop0 - h1;
			break;
		case HOP_N2:
			component_value[x] = hop0 - (int)ceil(h1*negative_ratio);
			break;
		case HOP_N3:
			component_value[x] = hop0 - (int)ceil(h1*negative_ratio*negative_ratio);
			break;
		case HOP_N4:
			#ifdef IS_MAX_HOPS
			component_value[x] = 0;
			#else
			component_value[x] = hop0 - (int)ceil(h1*negative_ratio*negative_ratio*negative_ratio);
			#endif
			break;
		default:
			printf("ERROR: Unexpected symbol were found.");
			break;
		}
		small_hop = is_small_hop(current_hop);
		h1 = adapt_h1(h1, small_hop, last_small_hop);
		#ifdef IS_GRADIENT
		gradient = adapt_gradient(current_hop, small_hop, gradient);
		#endif
		last_small_hop = small_hop;
	}
}

bool is_small_hop(uint8_t hop) {

	if (hop > HOP_P1 || hop < HOP_N1) {
		return false;
	}
	else {
		return true;
	}
}

unsigned char adapt_h1(unsigned char  h1, bool small_hop, bool last_small_hop) {

	if (small_hop * last_small_hop) {
		if (h1 > MIN_H1) {
			h1--;
		}
	}
	else {
		h1 = MAX_H1;
	}
	return h1;
}

char adapt_gradient(uint8_t current_hop, bool small_hop, char prev_gradient) {

	if (current_hop == HOP_P1) {
		return 1;
	}
	else if (current_hop == HOP_N1) {
		return -1;
	}
	else if (!small_hop) {
		return 0;
	}
	else {
		return prev_gradient;
	}
}

void calculate_ranges(uint8_t hop0, uint8_t hop1, double * positive_ratio, double * negative_ratio) {

	*positive_ratio = min(MAX_R, pow(RANGE*((255 - hop0) / hop1), 1.0f / 3.0f));
	*positive_ratio = max(MIN_R, *positive_ratio);

	*negative_ratio = min(MAX_R, pow(RANGE*(hop0 / hop1), 1.0f / 3.0f));
	*negative_ratio = max(MIN_R, *negative_ratio);

	return;
}
