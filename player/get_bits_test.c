#include "get_bits.h"
#include "stdio.h"
#include "stdlib.h"
int main() {

	get_bits_context ctx;
	uint8_t value;
    uint16_t value_short;
    FILE * f = fopen("test/binarytest.bin", "rb");
	init_get_bits(f, &ctx);

	// 60000081FEE5379D9EE6CD9A6E6CD9B3 This is contained in the binarytest.bin
	uint8_t values[128] = { 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,0,1,0,1,0,0,1,1,0,1,1,1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1 };

	for (int i = 0; i < 84; i++) {
		value = get_bit(&ctx);
		if (value != values[i]) {
			printf("Error in value=%d real_value=%d iteration %d \n",value,values[i],  i);
			//break;
		}
	}
	fclose(f);
	f = fopen("test/binarytest.bin", "rb");
	init_get_bits(f, &ctx);

    skip_bits(&ctx, 1);
    value =get_aligned_byte(&ctx);
    if(value != 0x00)
        printf("Error in value=%x real_value=0x00  \n",value);

    skip_bits(&ctx, 1);
    value =get_aligned_byte(&ctx);
    if(value != 0x81)
        printf("Error in value=%x real_value=0x81  \n",value);

	fclose(f);
	f = fopen("test/binarytest.bin", "rb");
	init_get_bits(f, &ctx);

	value_short = show_short(&ctx);
	skip_bits(&ctx, 16);
    if(value_short != 0x6000)
        printf("Error in value=%x real_value=0x6000  \n",value);

	value_short = show_short(&ctx);
	skip_bits(&ctx, 1);
    if(value_short != 0x0081)
        printf("Error in value=%x real_value=0x0081  \n",value);

	value_short = show_short(&ctx);
    if(value_short != 0x0103)
        printf("Error in value=%x real_value=0x0103  \n",value);

    finish_byte(&ctx);

    //0F E6 E5 9B 29 DE 6C F7 36 6C D9 98 CA 06 3F 32 8F 34 23 0E 65 3B 9C 50 BB 39 8F 49 8B 2E 69 DC D9 DC FC 90 E7 56 AF 9E CD 99 DD 9D 9D 9B 9E CD 9D 9E E6 CF B3 B9 FD 80
	uint8_t values2[56] = {0x0F, 0xE6, 0xE5, 0x9B, 0x29, 0xDE, 0x6C, 0xF7, 0x36, 0x6C, 0xD9, 0x98, 0xCA, 0x06, 0x3F, 0x32, 0x8F, 0x34, 0x23, 0x0E, 0x65, 0x3B, 0x9C, 0x50, 0xBB, 0x39, 0x8F, 0x49, 0x8B, 0x2E, 0x69, 0xDC, 0xD9, 0xDC, 0xFC, 0x90, 0xE7, 0x56, 0xAF, 0x9E, 0xCD, 0x99, 0xDD, 0x9D, 0x9D, 0x9B, 0x9E, 0xCD, 0x9D, 0x9E, 0xE6, 0xCF, 0xB3, 0xB9, 0xFD, 0x80};
    uint8_t hopss[256] = {0};
	int hops = decode_line_entropic(values2, hopss, 56);
	printf("hops %d\n", hops);
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return 0;
}
