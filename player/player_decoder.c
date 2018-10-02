#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include"player_decoder.h"

void init_player(int width, int height, bool is_fullscreen) {

    uint32_t window_flags = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {

		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return;
	}

	if (is_fullscreen) {

        window_flags = window_flags | SDL_WINDOW_FULLSCREEN;
    }

	window = SDL_CreateWindow("LHE Rpi Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
	if (window == NULL) {

		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window SDL: %s", SDL_GetError());
		return;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL) {

		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer SDL: %s", SDL_GetError());
		return;
	}
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderPresent(renderer);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	return;
}


void play_frame(uint8_t * Y_data, uint8_t * U_data, uint8_t * V_data, int Y_width, int UV_width) {

	int status = SDL_UpdateYUVTexture(texture, NULL, Y_data, Y_width, U_data, UV_width, V_data, UV_width);
	if (status != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't update texture SDL: %s", SDL_GetError());
	}
	status = SDL_RenderCopy(renderer, texture, NULL, NULL);
	if (status != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't update renderer SDL: %s", SDL_GetError());
	}
	SDL_RenderPresent(renderer);
}

void handle_window() {
	SDL_Event e;

	while (SDL_PollEvent(&e) != 0) {
		switch ( e.type) {
		case SDL_QUIT:

			exit(EXIT_SUCCESS);
			break;
		default:
			break;
		}
	}
	return;
}

void close_player() {

	SDL_DestroyTexture(texture);
	texture = NULL;

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	SDL_Quit();
	return;
}
