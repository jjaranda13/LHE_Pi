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
#include "entropic_decoder.h"

uint8_t* init_entropic_decoder(int rlc_length, int condition_length, int width) {

	if (rlc_length > 0) {
		rlc_length_ini = rlc_length;
	}
	if (condition_length > 0) {
		condition_length_ini = condition_length;
	}
	return (uint8_t *)malloc(sizeof(uint8_t)* (width-1));
}

void close_entropic_decoder(uint8_t* hops) {

	free(hops);
}

int decode_line_entropic(uint8_t * bits, uint8_t * hops, int bytes_lenght) {
	int h0_counter = 0, hops_counter = 0, zeros_since_a_one = 0, hop = 0, 
		data = 0, rlc_number =0;
	int condition_length = condition_length_ini;
	int rlc_lenght = rlc_length_ini;
	bool is_last_rlc = false;
	
	for (int i = 0; i < bytes_lenght << 3; i++) {
		data = get_data(bits, i, bytes_lenght);
		if (data == 0) {
			zeros_since_a_one++;
		}
		else if (data == 1) {
			hop = get_hop(zeros_since_a_one);
			hops[hops_counter] = hop;
			hops_counter++;
			zeros_since_a_one = 0;
			if (hop == HOP_0) {
				h0_counter++;
				if ((h0_counter >= condition_length)) {
					rlc_number = get_rlc_number(bits, &i, bytes_lenght,rlc_lenght);
					add_hop0(hops, &hops_counter, rlc_number);
					rlc_lenght = rlc_length_ini + 1;
					is_last_rlc = true;
					h0_counter = 0;
				}
				else if (is_last_rlc) {
					rlc_number = get_rlc_number(bits, &i, bytes_lenght, rlc_lenght);
					add_hop0(hops, &hops_counter, rlc_number);
					h0_counter = 0;
				}
			}
			else {
				h0_counter = 0;
				rlc_lenght = rlc_length_ini;
				is_last_rlc = false;
			}
		}
		else {
			printf("Error tried to acces a mmeory out of range. A segfault could have been trown\n");
		}
	}
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


int get_data(uint8_t * bits, int sub_index, int bytes_lenght) {

	int index = sub_index >> 3;
	if (index >=bytes_lenght) {
		return -1;
	}
	int position = 7- (sub_index % 8);
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
		return (uint8_t) 255;
	}

}

int get_rlc_number(uint8_t * bits, int * sub_index, int bytes_lenght, int rlc_lenght) {
	int rlc_number = 0;
	for (int j = rlc_lenght - 1; j >= 0; j--) {
		*sub_index = *sub_index +1;
		if (get_data(bits, *sub_index, bytes_lenght) == 1) {
			rlc_number = set_bit(rlc_number, j);
		}
	}
	return rlc_number;
}

void add_hop0(uint8_t * hops, int *hops_counter, int count) {

	for (int i = 0; i < count; i++) {
		hops[*hops_counter] = HOP_0;
		*hops_counter= (*hops_counter +1);
	}
}
