#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./emulator <rom_file>\n");
        return 1;
    }
    
    srand(time(NULL)); // seed the random number generator
    Chip8 chip8;
    
    initialize(&chip8);

    if (!loadROM(&chip8, argv[1])) {
        fprintf(stderr, "Failed to load rom\n");
        return 1;
    }

    printf("%02X %02X %02X %02X\n",
       chip8.memory[0x200],
       chip8.memory[0x201],
       chip8.memory[0x202],
       chip8.memory[0x203]);

    while (1) {
        if (!chip8.waiting_for_key_press)
            emulateCycle(&chip8);
            
    }

    return 0;
}

//  1. #include 
//  2. #include   // OpenGL graphics and input
//  3. #include "chip8.h" // Your cpu core implementation
//  4. 
//  5. chip8 myChip8;
//  6. 
//  7. int main(int argc, char **argv) 
//  8. {
//  9.   // Set up render system and register input callbacks
// 10.   setupGraphics();
// 11.   setupInput();
// 12. 
// 13.   // Initialize the Chip8 system and load the game into the memory  
// 14.   myChip8.initialize();
// 15.   myChip8.loadGame("pong");
// 16. 
// 17.   // Emulation loop
// 18.   for(;;)
// 19.   {
// 20.     // Emulate one cycle
// 21.     myChip8.emulateCycle();
// 22. 
// 23.     // If the draw flag is set, update the screen
// 24.     if(myChip8.drawFlag)
// 25.       drawGraphics();
// 26. 
// 27.     // Store key press state (Press and Release)
// 28.     myChip8.setKeys();	
// 29.   }
// 30. 
// 31.   return 0;
// 32. }