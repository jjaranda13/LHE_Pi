#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "entropic_decoder.h"

void test_entropic_decoder() {
	printf("    INFO: Started test of the entropic decoder\n");

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

	if (get_data(data_array, 3) != 0) {
		printf("ERROR: get_data test 1 FAILED\n");
		return;
	}
	if (get_data(data_array, 4) != 1) {
		printf("ERROR: get_data test 2 FAILED\n");
		return;
	}
	if (get_data(data_array, 8) != 0) {
		printf("ERROR: get_data test 3 FAILED\n");
		return;
	}
	if (get_data(data_array, 11) != 0) {
		printf("ERROR: get_data test 4 FAILED\n");
		return;
	}
	if (get_data(data_array, 12) != 1) {
		printf("ERROR: get_data test 5 FAILED\n");
		return;
	}
	if (get_data(data_array, 13) != 1) {
		printf("ERROR: get_data test 6 FAILED\n");
		return;
	}
	if (get_data(data_array, 14) != 1) {
		printf("ERROR: get_data test 7 FAILED\n");
		return;
	}
	if (get_data(data_array, 15) != 0) {
		printf("ERROR: get_data test 8 FAILED\n");
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
	if (get_rlc_number(data_array, &subindex, 4) != 2) {
		printf("ERROR: get_rlc_number test 1 FAILED\n");
		return;
	}
	subindex = 0;
	if (get_rlc_number(data_array, &subindex, 2) != 0) {
		printf("ERROR: get_rlc_number test 2 FAILED\n");
		return;
	}
	subindex = 7;
	if (get_rlc_number(data_array, &subindex, 8) != 14) {
		printf("ERROR: get_rlc_number test 3 FAILED\n");
		return;
	}
	subindex = 7;
	if (get_rlc_number(data_array, &subindex, 2) != 0) {
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

	//Test decode_line_entropic

	// HOP_P1-HOP_N2-HOP_N4-hop_0-HOP_P3-HOP_N3-HOP_0-HOP_0-HOP_P2-HOP_0-HOP_N4-HOP_N1-HOP_N2-HOP_P1
	// 5     2       0        4    7      1     4   4   6    4   0       3   2     5
	// 01   00001  000000001  1  000001 0000001 1   1  0001  1  000000001 001 00001 01
	// 01000010 00000001 10000010 00000111 00011000 00000100 10000101
	// 66        1         130        7       24         4       133
	uint8_t bits_array[7] = { 66,1,130,7,24,4,133 };
	uint8_t syms_array[14] = { 15 };
	uint8_t expected_syms_array[14] = { HOP_P1,HOP_N2,HOP_N4,HOP_0,HOP_P3,HOP_N3,HOP_0,HOP_0,HOP_P2,HOP_0,HOP_N4,HOP_N1,HOP_N2,HOP_P1 };
	int length_obtained = decode_line_entropic(bits_array, syms_array, 7);

	if (length_obtained != 14 ) {
		printf("ERROR: decode_line_entropic test 1 FAILED %d\n", length_obtained);
		return;
	}
	for (int i = 0; i < 13; i++) {
		if (expected_syms_array[i] != syms_array[i]) {
			printf("ERROR: decode_line_entropic test 2 FAILED\n");
			return;
		}
	}

	// HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-{ 0 1110(x14 HOP_0)}-HOP_P1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{111 000000 (X77 HOP_O)}-HOP_N1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{1 000001 (X32 HOP_0)}
	// 7x 4 - 14x 4 - 5 - 7x 4 - 77x 4 - 3 - 7x 4 - 16x 4
	// 11111110 11100111 11111111 00000000 11111111 10000010
	uint8_t bits_array_2[6] = { 254,231,255,0,255, 130};
	uint8_t syms_array_2[130] = { 15 };
	uint8_t expected_syms_array_2[130] = { HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_P1,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_N1, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0 };
	length_obtained = decode_line_entropic(bits_array_2, syms_array_2, 6);

	if (length_obtained != 130) {
		printf("ERROR: decode_line_entropic test 3 FAILED - %d\n",length_obtained);
		return;
	}
	for (int i = 0; i < 146; i++) {
		if (expected_syms_array_2[i] != syms_array_2[i]) {
			printf("ERROR: decode_line_entropic test 4-%d FAILED\n", i);
			return;
		}
	}

	
	uint8_t bits_array_3[68] = { 0x1F, 0x29, 0x6F, 0xCF, 0x5F, 0xCB, 0x9A, 0x5C, 0xBF, 0x24, 0x0E, 0x53, 0x4E, 0x9F, 0x4F, 0xF0, 0xFF, 0x91, 0x0F, 0xE9, 0xEC, 0xFF, 0x16, 0x59, 0xB9, 0xEC, 0xA7, 0xBF, 0x9D, 0x9D, 0x3D, 0x9A, 0x52, 0x9B, 0x36, 0x5C, 0x5B, 0xF0, 0xE6, 0xCD, 0x9B, 0x37, 0x9D, 0xCD, 0xE7, 0x73, 0x4B, 0x37, 0x9B, 0x34, 0xB9, 0xD9, 0xB3, 0x73, 0x4B, 0x9D, 0x36, 0x6E, 0x76, 0x6C, 0xA7, 0x65, 0x2E, 0x74, 0xB9, 0xD3, 0x4A };
	uint8_t syms_array_3[340] = { 15 };

	length_obtained = decode_line_entropic(bits_array_3, syms_array_3, 68);

	printf("INFO: lenght is %d\n", length_obtained);


	printf("INFO: decode_line_entropic tests completed\n");
	//Test decode_symbols_entropic


	// uint8_t bits_array[7] = { 66,1,130,7,24,4,133 };
	// uint8_t syms_array[14] = { 15 };
	// expected_syms_array[14] = { HOP_P1,HOP_N2,HOP_N4,HOP_0,HOP_P3,HOP_N3,HOP_0,HOP_0,HOP_P2,HOP_0,HOP_N4,HOP_N1,HOP_N2,HOP_P1 };
	int bits_processed;
	length_obtained = decode_symbols_entropic(bits_array, syms_array,7, 14, &bits_processed);
	

	if (bits_processed != 7) {
		printf("ERROR: decode_symbols_entropic test 1 FAILED %d\n", length_obtained);
		return;
	}
	if (length_obtained != 14) {
		printf("ERROR: decode_symbols_entropic test 2 FAILED %d\n", length_obtained);
		return;
	}
	for (int i = 0; i < 13; i++) {
		if (expected_syms_array[i] != syms_array[i]) {
			printf("ERROR: decode_symbols_entropic test 3 FAILED\n");
			return;
		}
	}

	// HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-{ 0 1110(x14 HOP_0)}-HOP_P1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{111 000000 (X77 HOP_O)}-HOP_N1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{1 000001 (X32 HOP_0)}
	// 7x 4 - 14x 4 - 5 - 7x 4 - 77x 4 - 3 - 7x 4 - 16x 4
	// 11111110 11100111 11111111 00000000 11111111 10000010
	//uint8_t bits_array_2[6] = { 254,231,255,0,255, 130 };
	//uint8_t syms_array_2[130] = { 15 };
	//uint8_t expected_syms_array_2[130] = { HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_P1,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_N1, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0 };
	length_obtained = decode_symbols_entropic(bits_array_2, syms_array_2,6, 130, &bits_processed);

	if (length_obtained != 130) {
		printf("ERROR: decode_line_entropic test 4 FAILED - %d\n", length_obtained);
		return;
	}
	if (bits_processed != 6) {
		printf("ERROR: decode_line_entropic test 5 FAILED - %d\n", length_obtained);
		return;
	}
	for (int i = 0; i < 130; i++) {
		if (expected_syms_array_2[i] != syms_array_2[i]) {
			printf("ERROR: decode_line_entropic test 6-%d FAILED\n", i);
			return;
		}
	}

	// uint8_t bits_array[7] = { 66,1,130,7,24,4,133 };
	// uint8_t syms_array[14] = { 15 };
	// expected_syms_array[14] = { HOP_P1,HOP_N2,HOP_N4,HOP_0,HOP_P3,HOP_N3,HOP_0,HOP_0,HOP_P2,HOP_0,HOP_N4,HOP_N1,HOP_N2,HOP_P1 };

	length_obtained = decode_symbols_entropic(bits_array, syms_array,7, 15, &bits_processed);

	if (length_obtained != 14) {
		printf("ERROR: decode_symbols_entropic test 7 FAILED %d\n", length_obtained);
		return;
	}
	if (bits_processed != 7) {
		printf("ERROR: decode_line_entropic test 8 FAILED - %d\n", length_obtained);
		return;
	}
	for (int i = 0; i < 13; i++) {
		if (expected_syms_array[i] != syms_array[i]) {
			printf("ERROR: decode_symbols_entropic test 9 FAILED\n");
			return;
		}
	}

	// HOP_P1-HOP_N2-HOP_N4-HOP_0-HOP_P3-HOP_N3-HOP_0-NEW_LINE-HOP_0-HOP_P2-HOP_0-HOP_N4-HOP_N1-HOP_N2-HOP_P1
	//    5     2       0     4     7      1      4              4      6     4      0      3      2     5
	//    01  00001 000000001 1   000001 0000001  1   000000000  1     0001   1  000000001 001   00001   01
	// 01000010 00000001 10000010 00000110  00000000 01000110 00000001 00100001 01-000000 
	//    66       1       130       6        0       70       1       33         64
	uint8_t bits_array3[9] = { 66,1,130,6,0,70,1,33,64 };
	uint8_t syms_array3[7] = { 15 };
	uint8_t expected_syms_array3[7] = { HOP_P1,HOP_N2,HOP_N4,HOP_0,HOP_P3,HOP_N3,HOP_0 };
	length_obtained = decode_symbols_entropic(bits_array3, syms_array3, 9,20, &bits_processed);

	if (length_obtained != 7) {
		printf("ERROR: decode_line_entropic test 10 FAILED %d\n", length_obtained);
		return;
	}
	if (bits_processed != 4) {
		printf("ERROR: decode_line_entropic test 11 FAILED - %d\n", bits_processed);
		return;
	}
	for (int i = 0; i < 7; i++) {
		if (expected_syms_array3[i] != syms_array3[i]) {
			printf("ERROR: decode_line_entropic test 22 FAILED\n");
			return;
		}
	}
	
	printf("INFO: decode_symbols_entropic tests completed\n");

	printf("    INFO: entropic_decoder_test completed sucessfully\n");
	return;
}




/*int main(int argc, char *argv[]) {

	test_entropic_decoder();

}*/
