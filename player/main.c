#include <stdio.h>
#include "file_decoder.h"
//#include "entropic_decoder_test.h"
#include "entropic.h"

int main(void) {
	 
	//int state =	decode_file("image2.bin");
	//test_entropic();
	test_entropic_decoder();
	int state = decode_file("image2.bin");
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return 0;

}