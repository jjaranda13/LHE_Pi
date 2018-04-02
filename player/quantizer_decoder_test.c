#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "quantizer_decoder.h"

void test_quantizer_decoder() {
	
	printf("INFO: Started test of the quantizer decoder\n");

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