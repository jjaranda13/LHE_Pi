#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <Windows.h>
#include "player_decoder.h"

void test_player_decoder() {

	uint8_t * Ydata =(uint8_t *) malloc(sizeof(uint8_t)* 262144);
	uint8_t * Udata = (uint8_t *)malloc(sizeof(uint8_t) * 65536);
	uint8_t * Vdata = (uint8_t *)malloc(sizeof(uint8_t) * 65536);

	for (int i = 0; i < 262144; i++) {
		Ydata[i] = 0;
	}
	for (int i = 0; i < 65536; i++) {
		Udata[i] = 255;
		Vdata[i] = 255;
	}

	init_player(512, 512);
	for (int i = 0; i < 10; i++) {
		play_frame(Ydata, Udata, Vdata, 512, 256);
		for (int i = 0; i < 262144; i++) {
			if (Ydata[i] == 0) {
				Ydata[i] = 255;
			}
			else {
				Ydata[i] = 0;
			}
		}
		Sleep(1000);
	}
	close_player();
	return;
}