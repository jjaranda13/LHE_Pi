/**
* @file entropic_decoder.c
* @author Francisco Jos� Juan Quintanilla
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
#include "get_bits.h"

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
	int i = 0, false_hops = 0;
	bool nal_found = false;
	while (hops_counter+ false_hops< hops_lenght && i < bytes_lenght << 3 && !nal_found) {

		data = get_data(bytes, i);
		switch (mode) {
		case HUFFMAN:
			if (data == 0) {
				zero_counter++;
				if (zero_counter == 24) {
					i -= 24;
					nal_found = true;
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
				//printf("Position %d is %d\n",hops_counter+false_hops,hops[hops_counter]);
				if ((hop < 5 && hop > 3) && (hops_counter + false_hops)%2 == 0) {
                    false_hops++;
				}
				hops_counter++;
				zero_counter = 0;
			}
			break;
		case PRE_HUFFMAN:
			if (data == 0) {
				zero_counter++;
				if (zero_counter == 24) {
					i -= 24;
					nal_found = true;
				}
			}
			else if (data == 1) {
				hop = get_hop(zero_counter + 1);
				h0_counter = 0;
				hops[hops_counter] = hop;
				if ((hop < 5 && hop > 3) && (hops_counter + false_hops)%2 == 0) {
                    false_hops++;
				}
				hops_counter++;
				zero_counter = 0;
				mode = HUFFMAN;
			}
			break;
		case RLC1:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
                false_hops += rlc_number;
				add_hop0(hops, &hops_counter, rlc_number);
				mode = PRE_HUFFMAN;
			}
			else {
                false_hops += 15;
				add_hop0(hops, &hops_counter, 15);
				rlc_length += 1;
				mode = RLC2;
			}
			break;
		case RLC2:
			if (data == 0) {
				rlc_number = get_rlc_number(bytes, &i, rlc_length);
				//printf("2 Position %d is +%d\n",hops_counter+false_hops, 2*rlc_number);
                false_hops += rlc_number;
				add_hop0(hops, &hops_counter, rlc_number);
				rlc_length = RLC_LENGHT_INI;
				mode = PRE_HUFFMAN;
			}
			else {
			//printf("2 Position %d is +62\n", hops_counter+false_hops);
                false_hops += 31;
				add_hop0(hops, &hops_counter, 31);
			}
			break;
		}
		i++;
	}
	//printf("Return %d\n", hops_counter+false_hops);
	*readed_bytes = i % 8 ? (i / 8) + 1 : i / 8;
	return hops_counter + false_hops;
}

int obtain_symbols_entropic(get_bits_context * ctx, uint8_t * hops, int hops_lenght) {
	int mode = HUFFMAN, h0_counter = 0, hops_counter = 0, hop = 15,
        rlc_number = 0, false_hops = 0, bits_used, bit_counter=0;

    uint16_t data;
	int condition_length = CONDITION_LENGHT_INI;
	int rlc_length = RLC_LENGHT_INI;
	while (hops_counter + false_hops < hops_lenght) {

		switch (mode) {
		case HUFFMAN:
            data = show_short(ctx);
            hop = get_first_hop(data, &bits_used, false);
            skip_bits(ctx, bits_used);
            bit_counter += bits_used;
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
#ifdef JUMP_TO_EVENS
            if ((hop < 5 && hop > 3) && (hops_counter + false_hops)%2 == 0) {
                false_hops++;
            }
#endif
            hops_counter++;
			break;
		case PRE_HUFFMAN:
            data = show_short(ctx);
            hop = get_first_hop(data, &bits_used, true);
            skip_bits(ctx, bits_used);
            h0_counter = 0;
            hops[hops_counter] = hop;
#ifdef JUMP_TO_EVENS
            if ((hop < 5 && hop > 3) && (hops_counter + false_hops)%2 == 0) {
                false_hops++;
            }
#endif
            hops_counter++;
            mode = HUFFMAN;
			break;
		case RLC1:
            data = get_bit(ctx);
			if (data == 0) {
				rlc_number = get_rlc_number_get_bits(ctx, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
#ifdef JUMP_TO_EVENS
				false_hops += rlc_number;
#endif
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 15);
#ifdef JUMP_TO_EVENS
				false_hops += 15;
#endif
				rlc_length += 1;
				mode = RLC2;
			}
			break;
		case RLC2:
             data = get_bit(ctx);
			if (data == 0) {
				rlc_number = get_rlc_number_get_bits(ctx, rlc_length);
				add_hop0(hops, &hops_counter, rlc_number);
#ifdef JUMP_TO_EVENS
				false_hops += rlc_number;
#endif
				rlc_length = RLC_LENGHT_INI;
				mode = PRE_HUFFMAN;
			}
			else {
				add_hop0(hops, &hops_counter, 31);
#ifdef JUMP_TO_EVENS
				false_hops += 31;
#endif
			}
			break;
		}
	}
	finish_byte(ctx);

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

uint8_t get_first_hop(uint16_t data, int * bits_used, bool is_pre_huffman) {

    uint8_t hop;

    if (data >= 0x8000) {
        *bits_used = 1;
        hop = HOP_0;
    }
    else if (data >= 0x4000) {
        *bits_used = 2;
        hop = HOP_P1;
    }
    else if (data >= 0x2000) {
        *bits_used = 3;
        hop = HOP_N1;
    }
    else if (data >= 0x0400) {
        if (data >= 0x1000) {
            *bits_used = 4;
            hop = HOP_P2;
        }
        else if (data >= 0x0800) {
            *bits_used = 5;
            hop = HOP_N2;
        }
        else {
            *bits_used = 6;
            hop = HOP_P3;
        }
    }
    else {
        if (data >= 0x0200) {
            *bits_used = 7;
            hop = HOP_N3;
        }
        else if (data >= 0x0100) {
            *bits_used = 8;
            hop = HOP_P4;
        }
        else {
            *bits_used = 9;
            hop = HOP_N4;
        }
    }
    if(is_pre_huffman){
        switch (hop) {
            case HOP_0:
                hop = HOP_P1;
                break;
            case HOP_P1:
                 hop = HOP_N1;
                break;
            case HOP_N1:
                hop = HOP_P2;
                break;
            case HOP_P2:
                hop = HOP_N2;
                break;
            case HOP_N2:
                hop = HOP_P3;
                break;
            case HOP_P3:
                hop = HOP_N3;
                break;
            case HOP_N3:
                hop = HOP_P4;
                break;
            case HOP_P4:
                hop = HOP_N4;
                break;
            default:
                printf("Error this should not be happening");
                break;
        }
    }
    return hop;
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

int get_rlc_number_get_bits(get_bits_context * ctx, int rlc_lenght) {
	uint8_t rlc_number = 0;
	rlc_number = show_byte(ctx);
	skip_bits(ctx, rlc_lenght);
	rlc_number >>= (8 - rlc_lenght);

	return rlc_number;
}

void add_hop0(uint8_t * hops, int *hops_counter, int count) {
	for (int i = 0; i < count; i++) {
		hops[*hops_counter] = HOP_0;
		*hops_counter = (*hops_counter + 1);
	}
}
