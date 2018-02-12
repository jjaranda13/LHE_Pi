#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "quantizer_decoder.h"

void test_quantizer_decoder() {

	printf("INFO: Started test of the quantizer decoder\n");
	//adapt_h1 test
	int h1 = START_H1; // =7
	bool last_small_hop = true;

	h1 = adapt_h1(h1, HOP_0, &last_small_hop);
	if (h1 != START_H1-1 || last_small_hop != true) {
		printf("ERROR: adapt_h1 test 1 FAILED\n");
		return;
	}
	h1 = adapt_h1(h1, HOP_0, &last_small_hop);
	if (h1 != START_H1 - 2 || last_small_hop != true) {
		printf("ERROR: adapt_h1 test 2 FAILED\n");
		return;
	}
	last_small_hop = false;
	h1 = adapt_h1(h1, HOP_0, &last_small_hop);
	if (h1 != MAX_H1 || last_small_hop != true) {
		printf("ERROR: adapt_h1 test 3 FAILED\n");
		return;
	}
	h1 = adapt_h1(h1, HOP_0, &last_small_hop);
	if (h1 != MAX_H1 - 1 || last_small_hop != true) {
		printf("ERROR: adapt_h1 test 4 FAILED\n");
		return;
	}
	h1 = adapt_h1(h1, HOP_P4, &last_small_hop);
	if (h1 != MAX_H1 || last_small_hop != false) {
		printf("ERROR: adapt_h1 test 5 FAILED\n");
		return;
	}
	// calculate_ranges test.
	double positive_ratio, negative_ratio;

	calculate_ranges(127, 7, &positive_ratio, & negative_ratio);
	printf(" Expected positive_ratio= 2.15289 | Result ratio = %lf\n", positive_ratio);
	printf(" Expected negative_ratio= 2.15289 | Result ratio = %lf\n", negative_ratio);

	calculate_ranges(254, 7, &positive_ratio, &negative_ratio);
	if (positive_ratio != 1.0f || negative_ratio != 2.7f) {
		printf("ERROR: adapt_h1 test 2 FAILED\n");
		return;
	}

	calculate_ranges(1, 7, &positive_ratio, &negative_ratio);
	if (positive_ratio != 2.7f || negative_ratio != 1.0f) {
		printf("ERROR: adapt_h1 test 3 FAILED\n");
		return;
	}

	calculate_ranges(90, 7, &positive_ratio, &negative_ratio);
	printf(" Expected positive_ratio= 2.60329 | Result ratio = %lf\n", positive_ratio);
	printf(" Expected negative_ratio= 1.91942 | Result ratio = %lf\n", negative_ratio);


	printf("INFO: quantizer_decoder_test completed sucessfully\n");
}