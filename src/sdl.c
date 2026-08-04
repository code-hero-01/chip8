#include "sdl.h"

bool sdl_init(SDLContext* sdl) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    sdl->window = SDL_CreateWindow(
        "CHIP-8",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640,
        320,
        SDL_WINDOW_SHOWN
    );

    if (sdl->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());

        SDL_DestroyWindow(sdl->window);
        SDL_Quit();

        return false;
    }

    return true;
}

void sdl_process_input(uint8_t key[16], bool* running) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                *running = false;
                break;
            
            case SDL_KEYDOWN:
            {    
                uint8_t chip8_key = map_key(event.key.keysym.sym);
                if (chip8_key != -1)
                    key[chip8_key] = 1;

                break;
            }

            case SDL_KEYUP:
            {    
                uint8_t chip8_key = map_key(event.key.keysym.sym);
                if (chip8_key != -1)
                    key[chip8_key] = 0;

                break;
            }
        }
    }
}

void sdl_render(SDLContext* sdl, const uint8_t gfx[64 * 32]) { 
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl->renderer);

    SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (gfx[y * 64 + x]) {
                SDL_Rect pixel = {
                    x * 10, y * 10, 10, 10
                };

                SDL_RenderFillRect(sdl->renderer, &pixel);
            }
        }
    }

    SDL_RenderPresent(sdl->renderer);
}

void sdl_shutdown(SDLContext* sdl) {
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

uint8_t map_key(SDL_Keycode key)
{
    switch (key) {
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;

        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;

        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;

        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;

        default: return -1;
    }
}