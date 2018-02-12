/**
* @file entropic_decoder.c
* @author Francisco José Juan Quintanilla
* @date Sep 2017
* @brief Entropic decoder for LHE_rpi codec.
* @see https://github.com/jjaranda13/LHE_Pi
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "entropic_decoder.h"

uint8_t* allocate_entropic_decoder(int width) {
	return (uint8_t *)malloc(sizeof(uint8_t)* width);
}

void free_entropic_decoder(uint8_t* hops) {
	free(hops);
}

#define RLC1 0
#define RLC2 1
#define HUFFMAN 2
#define PRE_HUFFMAN 3

int decode_line_entropic(uint8_t * bytes, uint8_t * hops, int bytes_lenght) {
	int mode = HUFFMAN, h0_counter = 0, hops_counter = 0, zero_counter = 0, hop = 15, data = 3, rlc_number = 0;
	int condition_length = CONDITION_LENGHT_INI;
	int rlc_length = RLC_LENGHT_INI;
	for (int i = 0; i < bytes_lenght << 3; i++) {
		data = get_data(bytes, i);
		switch (mode) {
		case HUFFMAN:
			if (data == 0) {
				zero_counter++;
			}
			else if (data == 1) {
				hop = get_hop(zero_counter);
				if (hop == HOP_0) {
					h0_counter++;
				}
				else {
					h0_counter = 0;
				}
				if (h0_counter == condition_length) {
					mode = RLC1;
				}
				hops[hops_counter] = hop;
				hops_counter++;
				zero_counter = 0;
			}
			break;
		case PRE_HUFFMAN:
			if (data == 0) {
				zero_counter++;
			}
			else if (data == 1) {
				hop = get_hop(zero_counter + 1);
				h0_counter = 0;
				hops[hops_counter] = hop;
				hops_counter++;
				zero_counter = 0;
				mode = HUFFMAN;
			}
			break;
		case RLC1:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 15);
				rlc_length += 1;
				mode = RLC2;
			}
			break;
		case RLC2:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
				rlc_length = RLC_LENGHT_INI;
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 31);
			}
			break;
		}
	}
	return hops_counter;
}
int decode_symbols_entropic(uint8_t * bytes, uint8_t * hops, int bytes_lenght, int hops_lenght, int * readed_bytes) {
	int mode = HUFFMAN, h0_counter = 0, hops_counter = 0, zero_counter = 0,
		hop = 15, data = 3, rlc_number = 0;
	int condition_length = CONDITION_LENGHT_INI;
	int rlc_length = RLC_LENGHT_INI;
	int i = 0;
	bool break_seq = false;
	while (hops_counter < hops_lenght && i / 8 < bytes_lenght && !break_seq) {

		data = get_data(bytes, i);
		switch (mode) {
		case HUFFMAN:
			if (data == 0) {
				zero_counter++;
				if (zero_counter == 9) {
					i -= 9;
					break_seq = true;
				}
			}
			if (data == 1) {
				hop = get_hop(zero_counter);
				if (hop == HOP_0) {
					h0_counter++;
				}
				else {
					h0_counter = 0;
				}
				if (h0_counter == condition_length) {
					mode = RLC1;
				}
				hops[hops_counter] = hop;
				hops_counter++;
				zero_counter = 0;
			}
			break;
		case PRE_HUFFMAN:
			if (data == 0) {
				zero_counter++;
				if (zero_counter == 9) {
					i -= 9;
					break_seq = true;
				}
			}
			else if (data == 1) {
				hop = get_hop(zero_counter + 1);
				h0_counter = 0;
				hops[hops_counter] = hop;
				hops_counter++;
				zero_counter = 0;
				mode = HUFFMAN;
			}
			break;
		case RLC1:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 15);
				rlc_length += 1;
				mode = RLC2;
			}
			break;
		case RLC2:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
				rlc_length = RLC_LENGHT_INI;
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 31);
			}
			break;
		}
		i++;
	}
	*readed_bytes = i % 8 ? (i / 8) + 1 : i / 8;
	return hops_counter;
}

bool test_bit(uint8_t data, int possition) {

	uint8_t mask = 1;
	mask = mask << possition;
	mask = mask & data;
	if (mask == 0) {
		return false;
	}
	return true;
}

uint8_t set_bit(uint8_t data, int possition) {

	uint8_t mask = 1;
	mask = mask << possition;
	mask = mask | data;
	return mask;
}

int get_data(uint8_t * bits, int sub_index) {

	int index = sub_index >> 3;
	int position = 7 - (sub_index % 8);
	if (test_bit(bits[index], position)) {
		return 1;
	}
	return 0;
}

uint8_t get_hop(int zeros_since_a_one) {

	if (zeros_since_a_one == 0) {
		return (uint8_t)HOP_0;
	}
	else if (zeros_since_a_one == 1) {
		return (uint8_t)HOP_P1;
	}
	else if (zeros_since_a_one == 2) {
		return (uint8_t)HOP_N1;
	}
	else if (zeros_since_a_one == 3) {
		return (uint8_t)HOP_P2;
	}
	else if (zeros_since_a_one == 4) {
		return (uint8_t)HOP_N2;
	}
	else if (zeros_since_a_one == 5) {
		return (uint8_t)HOP_P3;
	}
	else if (zeros_since_a_one == 6) {
		return (uint8_t)HOP_N3;
	}
	else if (zeros_since_a_one == 7) {
		return (uint8_t)HOP_P4;
	}
	else if (zeros_since_a_one == 8) {
		return (uint8_t)HOP_N4;
	}
	else {
		return (uint8_t)255;
	}

}

int get_rlc_number(uint8_t * bits, int * sub_index, int rlc_lenght) {
	int rlc_number = 0;
	for (int j = rlc_lenght - 1; j >= 0; j--) {
		*sub_index = *sub_index + 1;
		if (get_data(bits, *sub_index) == 1) {
			rlc_number = set_bit(rlc_number, j);
		}
	}
	return rlc_number;
}

void add_hop0(uint8_t * hops, int *hops_counter, int count) {
	for (int i = 0; i < count; i++) {
		hops[*hops_counter] = HOP_0;
		*hops_counter = (*hops_counter + 1);
	}
}
