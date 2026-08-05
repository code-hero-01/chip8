#include "chip8.h"

void initialize(Chip8* chip8) {
    memset(chip8, 0, sizeof(*chip8)); // reset chip8 state
    chip8->pc     = 0x200;  // program counter starts at 0x200
    
    // load font-set
    memcpy(
        chip8->memory + FONT_START,
        chip8_fontset,
        sizeof(chip8_fontset)
    );
}


void emulate_cycle(Chip8* chip8) {
    // fetch opcode
    if (!fetch(chip8)) 
        return;
    
    // decode opcode and execute opcode
    execute(chip8);

    // update timers
    update_timers(chip8);
}

bool load_ROM(Chip8* chip8, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seekig in file\n");
        fclose(file);
        return false;
    }
    
    long file_size = ftell(file); 
    if (file_size < 0) {
        perror("Error getting file size");
        fclose(file);
        return false;
    }
    
    if (file_size > RAM_SIZE - ROM_START) {
        fprintf(stderr, "ROM size too large");
        fclose(file);
        return false;
    }

    rewind(file); 

    // read into chip8 memory
    size_t elements_read = fread(&chip8->memory[ROM_START], sizeof(uint8_t), file_size, file);

    if (elements_read < file_size) {
        if (ferror(file)) 
            perror("Error reading file");
        else 
            fprintf(stderr, "Unexpected end of ROM file\n");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

bool fetch(Chip8* chip8) {
    if (chip8->pc >= RAM_SIZE - 1) {
        fprintf(stderr, "PC out of bounds: 0x%04X\n", chip8->pc);
        return false;
    }
    
    chip8->opcode = 
        (chip8->memory[chip8->pc] << 8) | 
        chip8->memory[chip8->pc + 1];
    
    chip8->pc += 2;
    return true;
}

void execute(Chip8* chip8) {
    uint16_t opcode = chip8->opcode;

    // decode opcode
    uint16_t nnn = (opcode & 0x0FFF); // addr - A 12-bit value, the lowest 12 bits of the instruction
    uint8_t n = (opcode & 0x000F); // nibble - A 4-bit value, the lowest 4 bits of the instruction
    uint8_t x = (opcode & 0x0F00) >> 8; // A 4-bit value, the lower 4 bits of the high byte of the instruction
    uint8_t y = (opcode & 0x00F0) >> 4; // A 4-bit value, the upper 4 bits of the low byte of the instruction
    uint8_t kk = (opcode & 0x00FF); // byte - An 8-bit value, the lowest 8 bits of the instruction

    // execute opcode
    switch (opcode & 0xF000) {
        case 0x0000: 
            switch (opcode) {
                case 0x00E0: // CLS - Clear the display
                    memset(chip8->gfx, 0, sizeof(chip8->gfx));
                    break;

                case 0x00EE: // RET - Return from a subroutine
                {    
                    if (chip8->sp == 0) {
                        fprintf(stderr, "Stack underflow\n");
                        break;
                    }

                    chip8->sp--;
                    chip8->pc = chip8->stack[chip8->sp]; // set program counter to top of stack

                    break;
                }

                default: // 0nnn - SYS addr : Jump to a machine code routine at nnn.
                    // This instruction is only used on the old computers on which Chip-8 was originally implemented. It is ignored by modern interpreters.
                    break;
            }
            break;

        case 0x1000: // 1nnn - JP addr : Jump to location nnn
            chip8->pc = nnn;
            break;

        case 0x2000: // 2nnn - CALL addr : Call subroutine at nnn
        {
            // store current address at stack top
            if (chip8->sp >= STACK_SIZE) {
                fprintf(stderr, "Stack overflow\n");
                break;
            }
            
            chip8->stack[chip8->sp] = chip8->pc;
            chip8->sp++;
            
            // jump to subroutine address nnn
            chip8->pc = nnn;
            break;
        }
        
        case 0x3000: // 3xkk - SE Vx, byte : Skip next instruction if Vx = kk
        {
            if (chip8->V[x] != kk)
                chip8->pc += 2;
            break;
        }
        
        case 0x4000: // 4xkk - SNE Vx, byte : Skip next instruction if Vx != kk
        {
            if (chip8->V[x] != kk)
                chip8->pc += 2;
            break;
        }

        case 0x5000: // 5xy0 - SE Vx, Vy : Skip next instruction if Vx = Vy
            switch (n) {
                case 0:
                {
                    if (chip8->V[x] == chip8->V[y]) 
                        chip8->pc += 2;
                    break;
                }
                
                default:
                    fprintf(
                        stderr,
                        "Unknown/unsupported opcode: 0x%04X at PC 0x%04X\n",
                        chip8->opcode,
                        chip8->pc - 2
                    );
                    break;
            }

        case 0x6000: // 6xkk - LD Vx, byte : Set Vx = kk
            chip8->V[x] = kk;
            break;
        
        case 0x7000: // 7xkk - ADD Vx, byte : Set Vx = Vx + kk
            chip8->V[x] += kk;
            break;

        case 0x8000:
            switch (n) {
                case (0x0): // 8xy0 - LD Vx, Vy : Set Vx = Vy
                    chip8->V[x] = chip8->V[y];
                    break;
                
                case (0x1): // 8xy1 - OR Vx, Vy : Set Vx = Vx OR Vy
                    chip8->V[x] = chip8->V[x] | chip8->V[y];        
                    break;
                    
                case (0x2):  // 8xy2 - AND Vx, Vy : Set Vx = Vx AND Vy
                    chip8->V[x] = chip8->V[x] & chip8->V[y];  
                    break;
                
                case (0x3): // 8xy3 - XOR Vx, Vy : Set Vx = Vx XOR Vy
                    chip8->V[x] = chip8->V[x] ^ chip8->V[y]; 
                    break;
                
                case (0x4): // 8xy4 - ADD Vx, Vy : Set Vx = Vx + Vy, set VF = carry
                {
                    uint16_t result = chip8->V[x] + chip8->V[y];
                    uint8_t carry = result > 0xFF; // set carry if result > 255
                    
                    chip8->V[x] = result & 0xFF;
                    chip8->V[0xF] = carry;

                    break;
                }

                case (0x5): // 8xy5 - SUB Vx, Vy : Set Vx = Vx - Vy, set VF = NOT borrow
                {
                    uint8_t no_borrow = chip8->V[x] > chip8->V[y];
                    
                    chip8->V[x] -= chip8->V[y];
                    chip8->V[0xF] = no_borrow;

                    break;
                }
             
                case (0x6): // 8xy6 - SHR Vx {, Vy} : Set Vx = Vx SHR 1.
                {    
                    uint8_t LSB = chip8->V[x] & 1;
                    
                    chip8->V[x] = chip8->V[x] >> 1;
                    chip8->V[0xF] = LSB;

                    break;
                }

                case (0x7): // 8xy7 - SUBN : Vx, Vy Set Vx = Vy - Vx, set VF = NOT borrow
                {
                    uint8_t no_borrow = chip8->V[y] > chip8->V[x];
                    
                    chip8->V[x] = chip8->V[y] - chip8->V[x];
                    chip8->V[0xF] = no_borrow;
                    
                    break;
                }

                case (0xE): // 8xyE - SHL Vx {, Vy} : Set Vx = Vx SHL 1
                {
                    uint8_t MSB = chip8->V[x] & 0x80;
                    
                    chip8->V[x] = chip8->V[x] << 1;
                    chip8->V[0xF] = MSB;
                    
                    break;
                }
            }
            break;
        
        case 0x9000: // 9xy0 - SNE Vx, Vy : Skip next instruction if Vx != Vy.
            switch (n) {    
                case 0:
                    if (chip8->V[x] != chip8->V[y])
                        chip8->pc += 2;
                    break;

                default:
                    fprintf(
                        stderr,
                        "Unknown/unsupported opcode: 0x%04X at PC 0x%04X\n",
                        chip8->opcode,
                        chip8->pc - 2
                    );
                    break;
            }
            break;
        
        case 0xA000: // Annn - LD I, addr : Set I = nnn
            chip8->I = nnn;
            break;

        case 0xB000: // Bnnn - JP V0, addr : Jump to location nnn + V0
            if (nnn + chip8->V[0] > RAM_SIZE) {
                fprintf(stderr, "Memory access out of bounds\n");
                break;
            }    

            chip8->pc = nnn + chip8->V[0];
            break;
        
        case 0xC000: // Cxkk - RND Vx, byte : Set Vx = random byte AND kk
        {
            uint8_t random_byte = rand() & 0xFF;
            chip8->V[x] = random_byte & kk;
            break;
        }

        case 0xD000: // Dxyn - DRW Vx, Vy, nibble : Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
        {   
            uint8_t screen_x_start = chip8->V[x];
            uint8_t screen_y_start = chip8->V[y];

            chip8->V[0xF] = 0;

            // read n bytes from memory starting at address stored in I
            if (chip8->I + n > RAM_SIZE) {
                fprintf(stderr, "Memory access out of bounds\n");
                break;
            }

            for (uint8_t row = 0; row < n; row++) {
                uint8_t sprite_byte = chip8->memory[chip8->I + row];
                
                for (uint8_t bit = 0; bit < 8; bit++) {
                    uint8_t sprite_pixel = 
                        (sprite_byte & (0x80 >> bit)) != 0; // 1 is set, 0 is not set

                    uint8_t screen_x = (screen_x_start + bit) % 64;
                    uint8_t screen_y = (screen_y_start + row) % 32;

                    size_t screen_index = screen_y * 64 + screen_x;
                    if (sprite_pixel == 1 && chip8->gfx[screen_index] == 1)
                            chip8->V[0xF] = 1; // set collision flag
                        
                    chip8->gfx[screen_index] ^= sprite_pixel; // xor
                } 
            }

            break;
        }

        case 0xE000:
            switch (kk) {
                case 0x9E: // Ex9E - SKP Vx : Skip next instruction if key with the value of Vx is pressed
                {
                    uint8_t val = chip8->V[x];

                    if (val >= 16) {
                        fprintf(stderr, "Invalid key value: 0x%02X\n", val);
                        break;
                    }

                    if (chip8->key[val])
                        chip8->pc += 2;
                    
                    break;
                }
                
                case 0xA1: // ExA1 - SKNP Vx : Skip next instruction if key with the value of Vx is not pressed
                {
                    uint8_t val = chip8->V[x];

                    if (val >= 16) {
                        fprintf(stderr, "Invalid key value: 0x%02X\n", val);
                        break;
                    }

                    if (!chip8->key[val])
                        chip8->pc += 2;
                    
                    break;
                }

                default:
                    fprintf(
                        stderr,
                        "Unknown/unsupported opcode: 0x%04X at PC 0x%04X\n",
                        chip8->opcode,
                        chip8->pc - 2
                    );
                    break;
            }
            break;

        case 0xF000:
            switch (kk) {
                case 0x07: // Fx07 - LD Vx, DT : Set Vx = delay timer value
                    chip8->V[x] = chip8->delay_timer;
                    break;
                
                case 0x0A: // Fx0A - LD Vx, K : Wait for a key press, store the value of the key in Vx
                {
                    chip8->waiting_for_key_press = true;
                    chip8->waiting_register = x;
                    break;
                }

                case 0x15: // Fx07 - LD DT, Vx : Set delay timer = Vx
                    chip8->delay_timer = chip8->V[x];
                    break;

                case 0x18: // Fx018 - LD ST, Vx : Set sound timer = Vx
                    chip8->sound_timer = chip8->V[x];
                    break;

                case 0x1E: // Fx1E - ADD I, Vx : Set I = I + Vx
                    chip8->I += chip8->V[x];
                    break;

                case 0x29: // Fx29 - LD F, Vx : Set I = location of sprite for digit Vx
                {
                    if (chip8->V[x] > 0xF) {
                        fprintf(stderr, "Font index out of bounds\n");
                        break;
                    }
                    chip8->I = FONT_START + chip8->V[x] * 5;
                    break;
                }
                
                case 0x33: // Fx33 - LD B, Vx : Store BCD representation of Vx in memory locations I, I+1, and I+2
                {
                    // uint8_t val = chip8->V[x];
                    // uint8_t ones = val % 10;
                    // val = val / 10;
                    // uint8_t tens = val % 10;
                    // val = val / 10;
                    // uint8_t hundreds = val % 10;

                    // chip8->memory[chip8->I] = hundreds;
                    // chip8->memory[chip8->I + 1] = tens;
                    // chip8->memory[chip8->I + 2] = ones;

                    chip8->memory[chip8->I] = (chip8->V[x]%1000)/100;
                    chip8->memory[chip8->I+1] = (chip8->V[x]%100)/10;
                    chip8->memory[chip8->I+2] = (chip8->V[x]%10);

                    break;
                }

                case 0x55: // Fx55 - LD [I], Vx : Store registers V0 through Vx in memory starting at location I
                {
                    if (chip8->I + x >= RAM_SIZE) {
                        fprintf(stderr, "Memory access out of bounds\n");
                        break;
                    } 
                    
                    for (uint8_t i = 0; i <= x; i++) 
                        chip8->memory[chip8->I + i] = chip8->V[i];
                    
                    break;
                }
                
                case 0x65: // Fx65 - LD Vx, [I] : Read registers V0 through Vx from memory starting at location I
                {
                    if (chip8->I + x >= RAM_SIZE) {
                        fprintf(stderr, "Memory access out of bounds\n");
                        break;
                    }    

                    for (uint8_t i = 0; i <= x; i++) 
                        chip8->V[i] = chip8->memory[chip8->I + i];

                    break;
                }

                default:
                    fprintf(
                        stderr,
                        "Unknown/unsupported opcode: 0x%04X at PC 0x%04X\n",
                        chip8->opcode,
                        chip8->pc - 2
                    );
                    break;
            }
            break;

        default:
            fprintf(
                stderr,
                "Unknown/unsupported opcode: 0x%04X at PC 0x%04X\n",
                chip8->opcode,
                chip8->pc - 2
            );
            break;
    }
}

void update_timers(Chip8* chip8)
{
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;

    if (chip8->sound_timer > 0)
        chip8->sound_timer--;
}