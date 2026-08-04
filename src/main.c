#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"
#include "sdl.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./emulator <rom_file>\n");
        return 1;
    }
    
    srand(time(NULL)); // seed the random number generator
    Chip8 chip8;
    
    printf("Initializing CHIP-8\n");
    initialize(&chip8);

    printf("Loading ROM\n");
    if (!load_ROM(&chip8, argv[1])) {
        fprintf(stderr, "Failed to load rom\n");
        return 1;
    }

    SDLContext sdl;
    printf("Initializing SDL\n");
    if (!sdl_init(&sdl)) {
        fprintf(stderr, "Failed to initialize SDL2\n");
        return 1;
    }
    
    double cpu_timer = 0.0;
    double timer_timer = 0.0;

    const double cpu_interval = 1.0 / 700.0; // seconds per instruction (for 700Hz CPU)
    const double timer_interval = 1.0 / 60.0;
    
    bool running = true;
    printf("Entering main loop\n");
    
    uint64_t last_time = SDL_GetPerformanceCounter();
    while (running) {
        uint64_t current_time = SDL_GetPerformanceCounter();

        double elapsed =
            (double)(current_time - last_time) / SDL_GetPerformanceFrequency();

        last_time = current_time;
        
        cpu_timer += elapsed;
        timer_timer += elapsed;

        sdl_process_input(chip8.key, &running);
        while (cpu_timer >= cpu_interval) {
            if (!chip8.waiting_for_key_press)
                emulate_cycle(&chip8);
            cpu_timer -= cpu_interval;
        }

        while (timer_timer >= timer_interval) {
            update_timers(&chip8);
            timer_timer -= timer_interval;
        }
        
        sdl_render(&sdl, chip8.gfx);  
    }

    sdl_shutdown(&sdl);
    return 0;
}
