#include "get_bits.h"
#include "stdio.h"
#include "stdlib.h"
int main(int argc, char *argv[], char **envp) {
	FILE * f = fopen("C:/Users/quinta/Desktop/binarytest.bin", "rb");
	get_bits_context ctx;
	uint8_t value;

	init_get_bits(f, &ctx);

	// 60000081FEE5379D9EE6CD9A6E6CD9B3 This is contained in the binarytest.bin
	uint8_t values[128] = { 0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,1,1,1,0,0,1,0,1,0,0,1,1,0,1,1,1,1,0,0,1,1,1,0,1,1,0,0,1,1,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,0,0,1,1,0,1,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,1,1 };
	for (int i = 0; i < 128; i++) {
		value = get_bit(&ctx);
		if (value != values[i]) {
			printf("Error in value %d \n", i);
		}
	}
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return 0;
}