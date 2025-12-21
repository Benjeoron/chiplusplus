#include <thread>
#include <chrono>
#include <string>
#include <fstream> 
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <SDL3/SDL.h>
#include "chip8.h"

static inline uint8_t Nibble2(uint16_t opcode) {return (opcode & kSecondNibbleMask) >> 4;}
static inline uint8_t Nibble3(uint16_t opcode) {return (opcode & kThirdNibbleMask) >> 8;}
static inline uint8_t Nibble4(uint16_t opcode) {return (opcode & kFourthNibbleMask) >> 12;}

static bool CheckKey(int scancode);

bool Chip8::Initialize(int vid_scale, int pitch, char const* title) {
    this->I = 0;
    this->pc = PROGRAM_START;
    this->delay_timer = 0;
    this->sound_timer = 0;
    this->sp = 0;
    uint8_t fontPtr = 0x050;
    this->vid_scale = vid_scale;
    this->draw_flag = false;
    this->input_blocked = false;

    
    for (int i = 0; i < 80; i++) {
        memory[fontPtr + i] = font[i];
    }

    if (!SDL_CreateWindowAndRenderer(title, 
        GFX_WIDTH * vid_scale, GFX_HEIGHT * vid_scale, 0, &window, &renderer)) {
            SDL_Log("Unable to create window and renderer %s", SDL_GetError());
            return false;
    }

    if (!SDL_SetRenderScale(renderer, vid_scale, vid_scale)) {
        SDL_Log("Unable to set renderer scale %s", SDL_GetError());
        return false;
    }
    
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
        SDL_Log("Unable to set renderer draw color %s", SDL_GetError());
        return false;
    }

    if (!SDL_RenderClear(renderer)) {
        SDL_Log("Unable to clear renderer %s", SDL_GetError());
        return false;
    }

    return true;
}

void Chip8::RenderScreen() {
    if (!SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)) {
        SDL_Log("Unable to set renderer draw color %s", SDL_GetError());
        return;
    }

    if (!SDL_RenderClear(renderer)) {
        SDL_Log("Unable to clear renderer %s", SDL_GetError());
        return;
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            SDL_FRect rect;
            rect.x = j;
            rect.y = i;
            rect.w = 1;
            rect.h = 1;
            uint8_t px = gfx[i * 64 + j];
            SDL_SetRenderDrawColor(renderer, 255 * px, 255 * px, 255 * px, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    SDL_RenderPresent(renderer);
}

void Chip8::UpdateTimers() {
    if (delay_timer > 0)	{
		delay_timer--;
	}

	if (sound_timer > 0) {
		sound_timer--;
	}
}

bool Chip8::LoadProgram(std::string filePath) {
    using namespace std;
    ifstream input = ifstream(filePath, ios::binary);
    if (!input.is_open()) {
        cerr << "Error opening file!" << endl;
        return false;
    } else {
        input.seekg (0, input.end);
        int length = input.tellg();
        input.seekg (0, input.beg);

        char* buffer = new char [length];
        input.read(buffer, length);

        uint16_t loadPtr;
        for (loadPtr = PROGRAM_START; loadPtr - PROGRAM_START < length; loadPtr++) {
            memory[loadPtr] = buffer[loadPtr - PROGRAM_START];
        }

        delete[] buffer;
        return true;
    }
}

void Chip8::RunCycle() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1]; //fetch
    pc += 2;

    switch (Nibble4(opcode)){
        case 0x0:
            Opcode0XXX(opcode);
            break;
        case 0x1:
            Opcode1XXX(opcode); 
            break;
        case 0x2:
            Opcode2XXX(opcode); 
            break;
        case 0x3:
            Opcode3XXX(opcode); 
            break;
        case 0x4:
            Opcode4XXX(opcode);
            break;
        case 0x5:
            Opcode5XXX(opcode);
            break;
        case 0x6: 
            Opcode6XXX(opcode);
            break;
        case 0x7: 
            Opcode7XXX(opcode);
            break;
        case 0x8:
            Opcode8XXX(opcode);
            break;
        case 0x9:
            Opcode9XXX(opcode);
            break;
        case 0xA:
            OpcodeAXXX(opcode);
            break;
        case 0xB:
            OpcodeBXXX(opcode);
            break;
        case 0xC: {
            OpcodeCXXX(opcode);
            break;
        } 
        case 0xD: {
            OpcodeDXXX(opcode);
            draw_flag = true;
            break;
        }
        case 0xE:
            OpcodeEXXX(opcode);
            break;
        case 0xF:
            OpcodeFXXX(opcode);
            break;
    };
}

void Chip8::Opcode0XXX(uint16_t opcode) {
    switch (opcode & kTwoNibbleMask) {
    case 0x00E0: 
        for (int i = 0; i < sizeof(gfx); i++) {
            gfx[i] = 0;
        } 
        break;
    case 0x00EE: 
        sp -= 1; 
        pc = stack[sp]; 
        break;
    }
}

void Chip8::Opcode1XXX(uint16_t opcode) {
    pc = (opcode & kThreeNibbleMask); 
}

void Chip8::Opcode2XXX(uint16_t opcode) {
    stack[sp] = pc; sp += 1; 
    pc = (opcode & kThreeNibbleMask); 
}

void Chip8::Opcode3XXX(uint16_t opcode) {
    if (v[Nibble3(opcode)] == (opcode & kTwoNibbleMask)) {
        pc += 2;
    }
}

void Chip8::Opcode4XXX(uint16_t opcode) {
    if (v[Nibble3(opcode)] != (opcode & kTwoNibbleMask)) {
        pc += 2;
    } 
}

void Chip8::Opcode5XXX(uint16_t opcode) {
    if (v[Nibble3(opcode)] == v[Nibble2(opcode)]) {
        pc += 2;
    }
}

void Chip8::Opcode6XXX(uint16_t opcode) {
    v[Nibble3(opcode)] = (opcode & kTwoNibbleMask); 
}

void Chip8::Opcode7XXX(uint16_t opcode) {
    v[Nibble3(opcode)] += (opcode & kTwoNibbleMask); 
}

void Chip8::Opcode8XXX(uint16_t opcode) {
    uint8_t x = (opcode & kThirdNibbleMask) >> 8;
    uint8_t y = (opcode & kSecondNibbleMask) >> 4;

    switch(opcode & kFirstNibbleMask) {
        uint16_t vx;
        uint16_t vy;
        case 0x0000: 
            v[x] = v[y]; 
            break;
        case 0x0001:
            v[x] |= v[y];
            if (mode == COSMAC_VIP) {
                v[0xF] = 0;
            }
            break; 
        case 0x0002:
            v[x] &= v[y];
            if (mode == COSMAC_VIP) {
                v[0xF] = 0;
            }
            break; 
        case 0x0003:
            v[x] ^= v[y];
            if (mode == COSMAC_VIP) {
                v[0xF] = 0;
            }
            break;
        case 0x0004:
            vx = v[x] + v[y];
            v[x] += v[y];
            if (vx > 0xFF) {
                v[0xF] = 1;
            } else {
                v[0xF] = 0;
            }
            break;
        case 0x0005:
            vx = v[x];
            v[x] -= v[y];
            if (vx >= v[y]) {
                v[0xF] = 1;
            } else if (vx < v[y]) {
                v[0xF] = 0;
            }
            break;
        case 0x0006:
            if (mode == COSMAC_VIP) {
                vy = v[y];
                v[x] = v[y] >> 1;
                v[0xF] = vy & 0x1;
            } else {
                vx = v[x];
                v[x] = v[x] >> 1;
                v[0xF] = vx & 0x1;
            }
            break;
        case 0x0007:
            v[x] = v[y] - v[x];
            if (v[x] < v[y]) {
                v[0xF] = 1;
            } else if (v[x] > v[y]) {
                v[0xF] = 0;
            }
            break;
        case 0x000E:
            if (mode == COSMAC_VIP) {
                vy = v[y];
                v[x] = v[y] << 1; 
                v[0xF] = (vy & kU8MSB) >> kU8MSBShift;
            } else {
                vx = v[x];
                v[x] = v[x] << 1; 
                v[0xF] = (vx & kU8MSB) >> kU8MSBShift;
            }
            break;
    }
}

void Chip8::Opcode9XXX(uint16_t opcode) {
    if (v[Nibble3(opcode)] != v[Nibble2(opcode)]) {
        pc += 2;
    }
}

void Chip8::OpcodeAXXX(uint16_t opcode) {
    I = opcode & kThreeNibbleMask;
}

void Chip8::OpcodeBXXX(uint16_t opcode) {
    if (mode == COSMAC_VIP) {
        pc = (opcode & kThreeNibbleMask) + v[0x0];
    } else {
        pc = (opcode & kThreeNibbleMask) + v[Nibble3(opcode)];
    }
}

void Chip8::OpcodeCXXX(uint16_t opcode) {
    uint8_t base = std::rand() % (256);
    v[Nibble3(opcode)] = base & (opcode & kTwoNibbleMask);
}

void Chip8::OpcodeDXXX(uint16_t opcode) {
    uint8_t x = v[(opcode & kThirdNibbleMask) >> 8] % 64;
    uint8_t y = v[(opcode & kSecondNibbleMask) >> 4] % 32;
    v[0xF] = 0;

    for (int row = 0; row < (opcode & kFirstNibbleMask); row++) {
        uint8_t sprite = memory[I + row];
        uint8_t yline = y + row;
        if (y + row >= 32) {
            break;
        }
        uint8_t currPixel = kU8MSB;
        for (int col = 0; col < 8; col++) {
            uint8_t xline = x + col;
            if (xline >= 64) {
                break;
            }
            if ((gfx[(yline * 64) + xline] == 1) && 
                ((sprite & currPixel) >> (7 - col)) == 1) {
                v[0xF] = 1;
                gfx[(yline * 64) + xline] = 0;
            } else if (((sprite & currPixel) >> (7 - col)) == 1) {
                gfx[(yline * 64) + xline] = 1;
            }
            currPixel >>= 1;
        }
    }
}

void Chip8::OpcodeEXXX(uint16_t opcode) {
    switch (opcode & kTwoNibbleMask) {
        case 0x009E:
            if (CheckKey(keypad[v[Nibble3(opcode)]])) {
                pc += 2;
            }
            break;
        case 0x00A1:
            if (!CheckKey(keypad[v[Nibble3(opcode)]])) {
                pc += 2;
            }
            break;
    }
}

void Chip8::OpcodeFXXX(uint16_t opcode) {
    uint8_t x = (opcode & kThirdNibbleMask) >> 8;
    bool res;
    const bool* keys;

    switch (opcode & kTwoNibbleMask) {
        uint8_t n;
        case 0x0007:
            v[x] = delay_timer;
            break;
        case 0x000A:
            input_blocked = true;
            keys = SDL_GetKeyboardState(nullptr);
            
            if (mode == COSMAC_VIP) {
                for (int i = 0; i < KEYPAD_SIZE; i++) {
                    keypad_state[i] = keys[keypad[i]];
                    if (keypad_state[i] && input_blocked) {
                        input_blocked = false;
                        SDL_Delay((1000/60)*8);
                        while (keys[keypad[i]]) {
                            SDL_PumpEvents();
                        }
                        v[x] = i;
                    }
                }
            } else {
                for (int i = 0; i < KEYPAD_SIZE; i++) {
                    keypad_state[i] = keys[keypad[i]];
                    if (keypad_state[i] && input_blocked) {
                        input_blocked = false;
                        v[x] = i;
                    }
                }
            }

            if (input_blocked) {
                pc -= 2;
            }
            break;
        case 0x0015:
            delay_timer = v[x];
            break;
        case 0x0018:
            sound_timer = v[x];
            break;
        case 0x001E:
            I += v[x];
            break;
        case 0x0029:
            I = v[x];
            break;
        case 0x0033:
            n = v[x];
            memory[I] = (n / 100); 
            memory[I + 1] = ((n / 10) % 10); 
            memory[I + 2] = n % 10;
            break;
        case 0x0055:
            for (int i = I; i <= I + x; i++) {
                memory[i] = v[i - I];
            }
            I += x;
            if (mode == COSMAC_VIP) {
                I += 1;
            }
            break;
        case 0x0065:
            for (int i = I; i <= I + x; i++) {
                v[i - I] = memory[i];
            }
            I += x;
            if (mode == COSMAC_VIP) {
                I += 1;
            }
            break;
    }
}

static bool CheckKey(int scancode) {
    int numkeys;
    const bool* keys = SDL_GetKeyboardState(&numkeys);
    return keys[scancode];
}

Chip8::~Chip8() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

//DEBUG
void Chip8::PrintProgram() {
    for (int i = PROGRAM_START; i < 4096; i++) {
        std::cout << std::hex << (int) memory[i] << " ";
        if (i % 80 == 0) {
            std::cout << std::endl;
        }
    }
}
