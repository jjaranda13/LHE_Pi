#pragma once

#include <SDL.h>
SDL_Window * window;
SDL_Renderer * renderer;
SDL_Texture * texture;

void init_player(int width, int height);
void play_frame(uint8_t * Y_data, uint8_t * U_data, uint8_t * V_data, int Y_width, int UV_width);
void handle_window();
void close_player();