#include <thread>
#include <chrono>
#include <string>
#include <fstream> 
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <SDL3/SDL.h>
#include "chip8.h"

static bool CheckKey(int scancode);

bool Chip8::Initialize(int vidScale, int pitch, char const* title) {
    this -> I = 0;
    this -> pc = PROGRAM_START;
    this -> delayTimer = 0;
    this -> soundTimer = 0;
    this -> sp = 0;
    uint8_t fontPtr = 0x050;
    this -> vidScale = vidScale;
    for (int i = 0; i < 80; i++) {
        memory[fontPtr + i] = font[i];
    }

    if (!SDL_CreateWindowAndRenderer(title, 
        GFX_WIDTH * vidScale, GFX_HEIGHT * vidScale, 0, &window, &renderer)) {
            SDL_Log("Unable to create window and renderer %s", SDL_GetError());
            return false;
    }

    if (!SDL_SetRenderScale(renderer, vidScale, vidScale)) {
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
    if (delayTimer > 0)	{
		delayTimer--;
	}

	if (soundTimer > 0) {
		soundTimer--;
	}
}

void Chip8::RunCycle() {
    this -> RunInstruction();
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

void Chip8::RunInstruction() {
    uint16_t opcode = (memory[pc] << 8) | memory[pc + 1]; //fetch
    uint8_t nib2 = (opcode & kSecondNibbleMask) >> 4;
    uint8_t nib3 = (opcode & kThirdNibbleMask) >> 8;
    uint8_t nib4 = (opcode & kFourthNibbleMask) >> 12;
    uint8_t first_byte = opcode & kTwoNibbleMask;

    pc += 2;
    switch (nib4){
        case 0x0:
            switch (first_byte) {
                case 0x00E0: 
                    for (int i = 0; i < sizeof(gfx); i++) {
                        gfx[i] = 0;
                    } 
                    break;
                case 0x00EE: 
                    sp -= 1; pc = stack[sp]; 
                    break;
            } break;
        case 0x1: 
            pc = (opcode & kThreeNibbleMask); 
            break;
        case 0x2: 
            stack[sp] = pc; sp += 1; 
            pc = (opcode & kThreeNibbleMask); 
            break;
        case 0x3:
            if (V[nib3] == (first_byte)) {
                pc += 2;
            } 
            break;
        case 0x4:
            if (V[nib3] != (first_byte)) {
                pc += 2;
            } 
            break;
        case 0x5:
            if (V[nib3] == V[nib2]) {
                pc += 2;
            }
            break;
        case 0x6: 
            V[nib3] = (first_byte); 
            break;
        case 0x7: 
            V[nib3] += (first_byte); 
            break;
        case 0x8:
            Opcode8XYN(opcode);
            break;
        case 0x9:
            if (V[nib3] != V[nib2]) {
                pc += 2;
            }
            break;
        case 0xA: 
            I = opcode & kThreeNibbleMask; 
            break;
        case 0xB:
            pc = opcode - 0xB000 + V[0];
            break;
        case 0xC: {
            uint8_t base = std::rand() % (256);
            V[nib3] = base & (first_byte);
            break;
        } 
        case 0xD: {
            OpcodeDXYN(opcode);
            break;
        }
        case 0xE:
            switch (first_byte) {
                case 0x009E:
                    if (CheckKey(keypad[V[nib3]])) {
                        pc += 2;
                    }
                    break;
                case 0x00A1:
                    if (!CheckKey(keypad[V[nib3]])) {
                        pc += 2;
                    }
                    break;
            }
            break;
        case 0xF:
            OpcodeFXNN(opcode);
            break;
    };

}

void Chip8::Opcode8XYN(uint16_t opcode) {
    uint8_t X = (opcode & kThirdNibbleMask) >> 8;
    uint8_t Y = (opcode & kSecondNibbleMask) >> 4;

    switch(opcode & kFirstNibbleMask) {
        uint16_t x;
        case 0x0000: 
            V[X] = V[Y]; 
            break;
        case 0x0001:
            V[X] |= V[Y];
            break; 
        case 0x0002:
            V[X] &= V[Y];
            break; 
        case 0x0003: 
            V[X] ^= V[Y];
            break;
        case 0x0004:
            x = V[X] + V[Y];
            if (x > 0xFF) {
                V[0xF] = 1;
            } else {
                V[0xF] = 0;
            }
            V[X] += V[Y];
            break;
        case 0x0005:
            if (V[X] > V[Y]) {
                V[0xF] = 1;
            } else if (V[X] < V[Y]) {
                V[0xF] = 0;
            }
            V[X] -= V[Y];
            break;
        case 0x0006: 
            V[0xF] = V[Y] & 1;
            V[X] = V[X] >> 1;
            break;
        case 0x0007:
            if (V[X] < V[Y]) {
                V[0xF] = 1;
            } else if (V[X] > V[Y]) {
                V[0xF] = 0;
            }
            V[X] = V[Y] - V[X];   
            break;
        case 0x000E:
            V[0xF] = (V[Y] & kU8MSB) >> kU8MSBShift;
            V[X] <<= 1;    
            break;
    }
}

void Chip8::OpcodeDXYN(uint16_t opcode) {
    uint8_t x = V[(opcode & kThirdNibbleMask) >> 8] & 63;
    uint8_t y = V[(opcode & kSecondNibbleMask) >> 4] & 31;
    V[0xF] = 0;

    for (int row = 0; row < (opcode & kFirstNibbleMask); row++) {
        uint8_t sprite = memory[I + row];
        if (y + row >= 32) {
            break;
        }
        uint8_t currPixel = 0b10000000;
        for (int col = 0; col < 8; col++) {
            if (x + col >= 64) {
                break;
            }
            if ((gfx[((y + row) * 64) + x + col] == 1) && 
                ((sprite & currPixel) >> (7 - col)) == 1) {
                V[15] = 1;
                gfx[((y + row) * 64) + x + col] = 0;
            } else if (((sprite & currPixel) >> (7 - col)) == 1) {
                gfx[((y + row) * 64) + x + col] = 1;
            }
            currPixel >>= 1;
        }
    }
}

void Chip8::OpcodeFXNN(uint16_t opcode) {
    uint8_t X = (opcode & kThirdNibbleMask) >> 8;
    bool key_pressed = false;
    bool res;

    switch (opcode & kTwoNibbleMask) {
        uint8_t n;
        case 0x0007:
            V[X] = delayTimer;
            break;
        case 0x000A:
            SDL_Event event;
            res = SDL_PollEvent(&event);
            if (res && event.type == SDL_EVENT_KEY_UP) {
                for (int i = 0; i < KEYPAD_SIZE; i++) {
                    if (reinterpret_cast<SDL_KeyboardEvent*>(&event)->scancode == keypad[i]) {
                        V[X] = i;
                        key_pressed = true;
                        break;
                    }
                }
            }
            if (!key_pressed) {
                pc -= 2;
            }   
            break;
        case 0x0015:
            delayTimer = V[X];
            break;
        case 0x0018:
            soundTimer = V[X];
            break;
        case 0x001E:
            I += V[X];
            break;
        case 0x0029:
            I = V[X];
            break;
        case 0x0033:
            n = V[X];
            memory[I] = (n / 100); 
            memory[I + 1] = ((n / 10) % 10); 
            memory[I + 2] = n % 10;
            break;
        case 0x0055:
            for (int i = I; i <= I + X; i++) {
                memory[i] = V[i - I];
            }
            I += 1 + (X);
            break;
        case 0x0065:
            for (int i = I; i <= I + X; i++) {
                V[i - I] = memory[i];
            }
            I += 1 + X;
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