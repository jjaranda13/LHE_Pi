#pragma once
#include <stdio.h>
#include <stdint.h>
#include "entropic_decoder.h"

void test_entropic_decoder() {
	printf("INFO: Started test of the entropic decoder\n");

	//Test set_bit
	uint8_t data = 0;
	data = set_bit(data, 0);
	if (data != 1) {
		printf("ERROR: set_bit test 1 FAILED\n");
		return;
	}
	data = set_bit(data, 7);
	if (data != 129) {
		printf("ERROR: set_bit test 2 FAILED\n");
		return;
	}
	data = set_bit(data, 7);
	if (data != 129) {
		printf("ERROR: set_bit test 3 FAILED\n");
		return;
	}
	data = set_bit(data, 4);
	if (data != 145) {
		printf("ERROR: set_bit test 4 FAILED\n");
		return;
	}
	printf("INFO: set_bit tests completed\n");

	//Test test_bit
	data = 129;
	if (!test_bit(data, 0)) {
		printf("ERROR: test_bit test 1 FAILED\n");
		return;
	}
	if (test_bit(data, 1)) {
		printf("ERROR: test_bit test 2 FAILED\n");
		return;
	}
	if (!test_bit(data, 7)) {
		printf("ERROR: test_bit test 3 FAILED\n");
		return;
	}
	printf("INFO: test_bit tests completed\n");

	//Test get_data
	uint8_t data_array[2] = {137,14}; // 1000 1001 0000 1110

	if (get_data(data_array, 3, 2) != 0) {
		printf("ERROR: get_data test 1 FAILED\n");
		return;
	}
	if (get_data(data_array, 4, 2) != 1) {
		printf("ERROR: get_data test 2 FAILED\n");
		return;
	}
	if (get_data(data_array, 8, 2) != 0) {
		printf("ERROR: get_data test 3 FAILED\n");
		return;
	}
	if (get_data(data_array, 11, 2) != 0) {
		printf("ERROR: get_data test 4 FAILED\n");
		return;
	}
	if (get_data(data_array, 12, 2) != 1) {
		printf("ERROR: get_data test 5 FAILED\n");
		return;
	}
	if (get_data(data_array, 13, 2) != 1) {
		printf("ERROR: get_data test 6 FAILED\n");
		return;
	}
	if (get_data(data_array, 14, 2) != 1) {
		printf("ERROR: get_data test 7 FAILED\n");
		return;
	}
	if (get_data(data_array, 15, 2) != 0) {
		printf("ERROR: get_data test 8 FAILED\n");
		return;
	}
	if (get_data(data_array, 16, 2) != -1) {
		printf("ERROR: get_data test 9 FAILED\n");
		return;
	}
	printf("INFO: get_data tests completed\n");

	// Test get_hop
	if (get_hop(0) != HOP_0) {
		printf("ERROR: get_hop test 1 FAILED\n");
		return;
	}
	if (get_hop(1) != HOP_P1) {
		printf("ERROR: get_hop test 2 FAILED\n");
		return;
	}
	if (get_hop(2) != HOP_N1) {
		printf("ERROR: get_hop test 3 FAILED\n");
		return;
	}
	if (get_hop(5) != HOP_P3) {
		printf("ERROR: get_hop test 4 FAILED\n");
		return;
	}
	if (get_hop(7) != HOP_P4) {
		printf("ERROR: get_hop test 5 FAILED\n");
		return;
	}
	if (get_hop(8) != HOP_N4) {
		printf("ERROR: get_hop test 6 FAILED\n");
		return;
	}
	if (get_hop(9) != 255) {
		printf("ERROR: get_hop test 7 FAILED\n");
		return;
	}
	printf("INFO: get_hop tests completed\n");

	//Test get_rlc_number
	// 1000 1001 0000 1110 { 137,14 }
	int subindex = 4;
	if (get_rlc_number(data_array, &subindex, 2, 4) != 2) {
		printf("ERROR: get_rlc_number test 1 FAILED\n");
		return;
	}
	subindex = 0;
	if (get_rlc_number(data_array, &subindex, 2, 2) != 0) {
		printf("ERROR: get_rlc_number test 2 FAILED\n");
		return;
	}
	subindex = 7;
	if (get_rlc_number(data_array, &subindex, 2, 8) != 14) {
		printf("ERROR: get_rlc_number test 3 FAILED\n");
		return;
	}
	subindex = 7;
	if (get_rlc_number(data_array, &subindex, 2, 2) != 0) {
		printf("ERROR: get_rlc_number test 4 FAILED\n");
		return;
	}
	printf("INFO: get_rlc_number tests completed\n");

	//Test add_hop0
	uint8_t hops_array[10] = { 0,5,7,8,4, 2,3,0,0,0 }; // 1000 1001 0000 1110
	int hops_counter = 7;

	add_hop0(hops_array, &hops_counter, 3);
	if (hops_array[6] != HOP_N1 || hops_array[7] != HOP_0 || hops_array[8] != HOP_0 || hops_array[9] != HOP_0) {
		printf("ERROR: add_hop0 test 1 FAILED\n");
		return;
	}
	printf("INFO: add_hop0 tests completed\n");

	//Test init_entropic_decoder
	uint8_t * pointer = init_entropic_decoder(2);

	printf("INFO: init_entropic_decoder tests completed\n");
	//Test decode_line_entropic

	// HOP_P1-HOP_P4-HOP_P3-HOP_N3-HOP_O-HOP_0-HOP_P2-HOP_0
	// 01000000 01000001 00000011 10001100
	uint8_t bits_array[4] = { 64,65,3,140 };
	uint8_t syms_array[8] = { 15,15,15,15,15,15,15,15 };
	uint8_t expected_syms_array[8] = { HOP_P1,HOP_P4,HOP_P3,HOP_N3,HOP_0,HOP_0,HOP_P2,HOP_0 };
	int length_obtained = decode_line_entropic(bits_array, syms_array, 4);

	if (length_obtained != 8 ) {
		printf("ERROR: decode_line_entropic test 1 FAILED\n");
		return;
	}
	for (int i = 0; i < 8; i++) {
		if (expected_syms_array[i] != syms_array[i]) {
			printf("ERROR: decode_line_entropic test 2 FAILED\n");
			return;
		}
	}

	// HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_P0-{7 in 4 bits- 0111}-HOP_P1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_P0-{10 in 4 bits-1010}-HOP_P0-{11 in 5 bits-01011}
	// 11111110 11101111 11111010 10101100
	uint8_t bits_array_2[4] = { 254,239,250,172 };
	uint8_t syms_array_2[44] = { 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 };
	uint8_t expected_syms_array_2[44] = { HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_P1,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0 };
	length_obtained = decode_line_entropic(bits_array_2, syms_array_2, 4);

	if (length_obtained != 44) {
		printf("ERROR: decode_line_entropic test 3 FAILED - %d\n",length_obtained);
		return;
	}
	for (int i = 0; i < 44; i++) {
		if (expected_syms_array_2[i] != syms_array_2[i]) {
			printf("ERROR: decode_line_entropic test 4-%d FAILED\n", i);
			return;
		}
	}
	printf("INFO: decode_line_entropic tests completed\nINFO: entropic_decoder_test completed sucessfully\n");
	return;
}