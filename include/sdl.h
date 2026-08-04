#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <chip8.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
} SDLContext;

bool sdl_init(SDLContext* sdl);
void sdl_process_input(uint8_t key[16], bool* running);
void sdl_render(SDLContext* sdl, const uint8_t gfx[64 * 32]);
void sdl_shutdown(SDLContext* sdl);

uint8_t map_key(SDL_Keycode key);

#endif
