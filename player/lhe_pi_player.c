#include <stdio.h>
#include "entropic_decoder_test.h"
#include "quantizer_decoder_test.h"
int main(){

	printf("Hello Wold \n");

	test_entropic_decoder();
	test_quantizer_decoder();

	int terminal;
	scanf_s("%d", &terminal);
	return 0;

}