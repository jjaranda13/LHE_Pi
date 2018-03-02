#include <stdio.h>
#include "decoder.h"
#include "entropic_decoder_test.h"

int main(void) {
	 
	//int state =	decode_file("image2.bin");
	//test_entropic();
	//test_entropic_decoder();
	//int state = decode_file("image2.bin");
	int state = decode_stream_file(320, 240, "C:/Users/quinta/Desktop/extracted3.bin");
	printf("Press Enter to Continue");
	while (getchar() != '\n');
	return ;

}

