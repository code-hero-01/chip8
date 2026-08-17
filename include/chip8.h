#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <string.h> 
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "fontset.h"
#include "sdl.h"

#define FONT_START 0x50
#define ROM_START 0x200
#define RAM_SIZE 4096
#define STACK_SIZE 16

typedef struct {
    uint8_t memory[RAM_SIZE]; // 4KB RAM
    uint8_t V[16]; // general purpose registers
    uint16_t I; // index register
    uint16_t pc; // program counter register
    uint16_t opcode;
    uint16_t stack[STACK_SIZE];
    uint16_t sp; // stack pointer
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t key[16]; // keypad (0x0-0xF)
    uint8_t gfx[64 * 32]; // graphics: screen - 2048 pixels (64 x 32)

    bool waiting_for_key_press;
    uint8_t waiting_register; 
    int sample_count;
} Chip8;

void initialize(Chip8* chip8);
bool load_ROM(Chip8* chip8, const char* filename);
void emulate_cycle(Chip8* chip8);
bool fetch(Chip8* chip8);
void execute(Chip8* chip8);
void update_timers(Chip8* chip8);

#endif
