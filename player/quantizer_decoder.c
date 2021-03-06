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


void build_hop_cache() {
	int h;
	const int hop_min = 1, hop_max = 255-1;
	const float maxr = MAX_R, minr = MIN_R, range = RANGE;
	double rpos, rneg;

	for (int hop0=0;hop0<=255;hop0++) {
 		for (int hop1=MIN_H1; hop1<=MAX_H1;hop1++) {

			rpos = min(maxr,pow(range*((255-hop0)/hop1),1.0f/3.0f));
 			rpos = max(minr,rpos);

			rneg = min(maxr,pow(range*(hop0/hop1),1.0f/3.0f));
 			rneg=max(minr,rneg);

			h=(int)(hop0-hop1*rneg*rneg*rneg);
			h=min(hop_max,h);
			h=max(h,hop_min);
			cache_hops[hop0][hop1-4][0] = (uint8_t)h;//(hop0-hop1*rneg*rneg*rneg);

			h=(int)(hop0-hop1*rneg*rneg);
			h=min(hop_max,h);
			h=max(h,hop_min);
			cache_hops[hop0][hop1-4][1] = (uint8_t)h;//(hop0-hop1*rneg*rneg);

			h=(int)(hop0-hop1*rneg);
			h=min(hop_max,h);
			h=max(h,hop_min);
			cache_hops[hop0][hop1-4][2] = (uint8_t)h;//(hop0-hop1*rneg);

 		}
	}

}
void decode_line_quantizer(uint8_t * hops, uint8_t * component_value, int hops_lenght) {

    int ghost_hop = 0;
    signed char gradient = 0, h1 = START_H1;
	unsigned char hop0;
	bool last_small_hop = true, small_hop;
	uint8_t current_hop;

	for (int x = 0; x < hops_lenght; x++) {

		current_hop = hops[x];

		if (x == 0) {
			hop0 = INIT_PREDICTION;
		}
		else {
			hop0 = component_value[x + ghost_hop - 1];
		}

		// Checks that there wont be overflow plus if gradient is activated.
#ifdef IS_GRADIENT
		//printf("Hop0 is %u gradient is %d\n", hop0, gradient );
		hop0 = hop0+gradient > 255? 255: hop0+gradient < 1? 1:hop0+gradient;
#endif

		switch (current_hop)
		{
		case HOP_0:
			component_value[x + ghost_hop] = hop0;
#ifdef JUMP_TO_EVENS
			if ((x + ghost_hop)%2 == 0) {
                ghost_hop++;
                component_value[x + ghost_hop] = hop0;
			}
#endif
			break;
		case HOP_P1:
			component_value[x + ghost_hop] = hop0 + h1;
			break;
		case HOP_P2:
			component_value[x + ghost_hop] = 255-cache_hops[255-hop0][h1-4][2];
			break;
		case HOP_P3:
			component_value[x + ghost_hop] = 255-cache_hops[255-hop0][h1-4][1];
			break;
		case HOP_P4:
			#ifdef IS_MAX_HOPS
			component_value[x + ghost_hop] = 255;
			#else
			component_value[x + ghost_hop] = 255-cache_hops[255-hop0][h1-4][0];
			#endif
			break;
		case HOP_N1:
			component_value[x + ghost_hop] = hop0 - h1;
			break;
		case HOP_N2:
			component_value[x + ghost_hop] = cache_hops[hop0][h1-4][2];
			break;
		case HOP_N3:
			component_value[x + ghost_hop] = cache_hops[hop0][h1-4][1];
			break;
		case HOP_N4:
			#ifdef IS_MAX_HOPS
			component_value[x] = 0;
			#else
			component_value[x + ghost_hop] = cache_hops[hop0][h1-4][0];
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
		//printf("gradient is %d\n", gradient );
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
		return (char)-1;
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
