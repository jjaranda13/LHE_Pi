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
	
	// HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-HOP_0-{ 0 1110(x14 HOP_0)}-HOP_P1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{1110 00000 (X77 HOP_O)}-HOP_P1-HOP_0-HOP_0-HOP_0-HOP_0-HOP_O-HOP_0-HOP_0-{1 000001 (X16 HOP_0)}
	//   1    1      1     1     1     1     1      01110               1      1     1     1    1      1     1     1      111000000               1     1     1     1     1     1    1     1      1000001
	// 11111110 11101111 11111110 00000111 11111100 00010000
	//    fe       ef       fe       07       fc       20
	uint8_t bits_array_2[6] = { 0xfe,0xef,0xfe,0x07,0xfc,0x10 };
	uint8_t syms_array_2[132] = { 15 };
	uint8_t expected_syms_array_2[130] = { HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_P1,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0 ,HOP_P1, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0,HOP_0, HOP_0,HOP_0 };
	length_obtained = decode_line_entropic(bits_array_2, syms_array_2, 6);

	if (length_obtained != 130) {
		printf("ERROR: decode_line_entropic test 3 FAILED - %d\n",length_obtained);
		return;
	}
	for (int i = 0; i < 130; i++) {
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

	/*if (length_obtained != 130) {
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
	}*/

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
	/*length_obtained = decode_symbols_entropic(bits_array3, syms_array3, 9,20, &bits_processed);

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
			printf("ERROR: decode_line_entropic test 12 FAILED\n");
			return;
		}
	}*/
	
	uint8_t bits_array4[79] = { 0x02, 0x7f, 0x8a, 0x96, 0x0d, 0x0a, 0x22, 0x04, 0x33, 0x65, 0x9f, 0x67, 0xbe, 0x56, 0x4a, 0x37, 0x03, 0xe4, 0x65, 0x64, 0x25, 0xc9, 0x90, 0xea, 0x13, 0xce, 0xa2, 0x7d, 0x34, 0xec, 0xfd, 0x2c, 0xd9, 0xbf, 0x3f, 0xb3, 0x04, 0x49, 0x3d, 0x92, 0x7e, 0x94, 0xfe, 0x9b, 0x34, 0xb3, 0x73, 0x4d, 0x9d, 0x2c, 0xbe, 0x59, 0xfa, 0x59, 0x60, 0xe5, 0x3b, 0xe6, 0x9e, 0x04, 0xd3, 0x7f, 0x81, 0xd9, 0xe9, 0x66, 0xcb, 0xfc, 0x0d, 0x0a, 0x9e, 0xc9, 0xd3, 0xfa, 0x6f, 0x37, 0xa6, 0xc0, 0xc2 };
    // 0000001  001      1111111   0 0010   1       01      001     01      1      000001  1      01      00001   01      0001    0001   0000001 00001   1      001    1      01      1     001    01     
	//  HOP_N3  HOP_N1   7XHOP_O   2XHOP_0  HOP_P1  HOP_P1  HOP_N1  HOP_P1  HOP_0  hop_p3  HOP_0  HOP_P1  HOP_N2  HOP_P1  HOP_P2  HOP_P2 HOP_N3  HOP_N2  HOP_0  HOP_N1 HOP_0  HOP_P1  HOP_0 HOP_N1 HOP_P1
	
	// 1     001     1111      01     1     001    111      01      1111     001    01     01     1     001     001    01     0001   1     01      11       0000001 11
	// HOP_0 HOP_N1  4x HOP_0  HOP_P1 HOP_0 HOP_N1 3x HOP_0 HOP_P1  4X HOP_0 HOP_N1 HOP_P1 HOP_P1 HOP_0 HOP_N1  HOP_N1 HOP_P1 HOP_P2 HOP_0  HOP_P1 2XHOP_0  HOP_N3  2xHOP_0  
	
	//  11      001    0001   1      001    01     01      1      001    00001   001    01      11       001     001    1     001    00001  11       01     01     00001  001    111      001    11       01      01     0001   001    1111     01     001     1      01      001    11       01      1      001     11111   01      001    01     1     00110110011011111100    1111111 0 1100  110000010001001001001111011001001001111110100101001 111111  0 1001 101100110100101100110111001101001101100111010010110010111110010110011111101001011001011000001110010100111011111001101001111000000100110100110 11111111 0 0000 01110110011110100101100110110010 11111111 0 0000 01101000010101001111011001001110100  1111111  0 1001 10111100110111101001101100000011000010
	// 2XHOP_0  NOP_N1 HOP_P2 HOP_0  HOP_N1 HOP_P1 HOP_P1  HOP_0  HOP_N1 HOP_N2  HOP_N1 HOP_P1  2XHOP_0  HOP_N1  HOP_N1 HOP_0 HOP_N1 HOP_N2 2XHOP_0  HOP_P1 HOP_P1 HOP_N2 HOP_N1 3XHOP_0  HOP_N1 2XHOP_0  HOP_P1  HOP_P1 HOP_P2 NOP_N1 4XHOP_0  HOP_P1 HOP_N1  HOP_0  HOP_P1  HOP_N1 2XHOP_0  HOP_P1  HOP_0  HOP_N1  5XHOP_0 HOP_P1  HOP_N1 HOP_P1 HOP_0
	
	uint8_t syms_array4[340] = { 15 };

	length_obtained = decode_symbols_entropic(bits_array4, syms_array4, 77, 320, &bits_processed);

	if (length_obtained != 320) {
		printf("ERROR: decode_line_entropic test 13 FAILED %d\n", length_obtained);
		//return;
	}
	if (bits_processed != 77) {
		printf("ERROR: decode_line_entropic test 14 FAILED - %d\n", bits_processed);
		//return;
	}
	uint8_t syms_array5 [340] = { 15 };

	length_obtained = decode_line_entropic(bits_array4, syms_array5, 77);

	printf("Info: lenght is %d\n", length_obtained);


	printf("INFO: decode_symbols_entropic tests completed\n");

	printf("    INFO: entropic_decoder_test completed sucessfully\n");
	return;
}




/*int main(int argc, char *argv[]) {

	test_entropic_decoder();

}*/
