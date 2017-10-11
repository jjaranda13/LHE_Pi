#include <SDL.h>
#include"player_decoder.h"

void init_player(int width, int height) {

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
		return;
	}
	window = SDL_CreateWindow("LHE Rpi Player",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, width, height,0);
	if (window == NULL) {

		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window SDL: %s", SDL_GetError());
		return;
	}
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {

		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer SDL: %s", SDL_GetError());
		return;
	}
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderPresent(renderer);
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	return;
}


void play_frame(uint8_t * Y_data, uint8_t * U_data, uint8_t * V_data, int Y_width,int UV_width) {

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