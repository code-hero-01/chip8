#ifndef SDL_H
#define SDL_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <chip8.h>

#define AMPLITUDE 28000
#define FREQUENCY 440.0
#define SAMPLE_RATE 44100
#define PI 3.14159265358979323846   

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_AudioDeviceID audio_device;
    int* sample_count;
} SDLContext;

bool sdl_init(SDLContext* sdl);
int sdl_process_input(uint8_t key[16], bool* running, int* released_key);
void sdl_render(SDLContext* sdl, const uint8_t gfx[64 * 32]);
void audio_callback(void* userdata, uint8_t* stream, int len);
void sdl_shutdown(SDLContext* sdl);

int map_key(SDL_Keycode key);

#endif
