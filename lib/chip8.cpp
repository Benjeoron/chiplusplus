#include <thread>
#include <chrono>
#include <string>
#include <fstream> 
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <SDL3/SDL.h>
#include "chip8.h"

bool Chip8::Initialize(int vidScale, int pitch, char const* title) {
    this -> I = 0;
    this -> pc = PROGRAM_START;
    this -> delayTimer = 0;
    this -> soundTimer = 0;
    this -> sp = 0;
    uint8_t fontPtr = 0x050;
    this -> drawFlag = true;
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
    drawFlag = false;
}

void Chip8::SetKeys() {
    
}

void Chip8::RunCycle() {
    for (int i = 0; i < 11; i++) {
        SetKeys();
        this -> RunInstruction();
    }

    if (delayTimer > 0)	{
		delayTimer--;
	}

	if (soundTimer > 0) {
		soundTimer--;
	}
    RenderScreen();
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

static constexpr uint16_t kFirstNibbleMask =  0x000F;
static constexpr uint16_t kSecondNibbleMask = 0x00F0;
static constexpr uint16_t kThirdNibbleMask =  0x0F00;
static constexpr uint16_t kFourthNibbleMask = 0xF000;
static constexpr uint16_t kTwoNibbleMask =    0x00FF;
static constexpr uint16_t kThreeNibbleMask =  0x0FFF;

void Chip8::RunInstruction() {
    this -> opcode = (memory[pc] << 8) | memory[pc + 1]; //fetch
    pc += 2;

    switch (opcode & kFourthNibbleMask){
        case 0x0000:
            switch (opcode & kTwoNibbleMask) {
                case 0x00E0: for (int i = 0; i < sizeof(gfx); i++) gfx[i] = 0; break;
                case 0x00EE: sp -= 1; pc = stack[sp]; break;
            } break;
        case 0x1000: 
            pc = (opcode & kThreeNibbleMask); 
            break;
        case 0x2000: 
            stack[sp] = pc; sp += 1; 
            pc = (opcode & kThreeNibbleMask); 
            break;
        case 0x3000:
            if (V[(opcode & kThirdNibbleMask) >> 8] == (opcode & kTwoNibbleMask)) {
                pc += 2;
            } 
            break;
        case 0x4000:
            if (V[(opcode & kThirdNibbleMask) >> 8] != (opcode & kTwoNibbleMask)) {
                pc += 2;
            } 
            break;
        case 0x5000:
            if (V[(opcode & kThirdNibbleMask) >> 8] == V[(opcode & kSecondNibbleMask) >> 4]) {
                pc += 2;
            }
            break;
        case 0x6000: 
            V[(opcode & kThirdNibbleMask) >> 8] = (opcode & kTwoNibbleMask); 
            break;
        case 0x7000: 
            V[(opcode & kThirdNibbleMask) >> 8] += (opcode & kTwoNibbleMask); 
            break;
        case 0x8000:
            Opcode8XXX(opcode);
            break;
        case 0x9000:
            if (V[(opcode & kThirdNibbleMask) >> 8] != V[(opcode & kSecondNibbleMask) >> 4]) {
                pc += 2;
            }
            break;
        case 0xA000: I = opcode & kThreeNibbleMask; break;
        case 0xB000:
            pc = opcode - 0xB000 + V[0];
            break;
        case 0xC000: {
            uint8_t base = std::rand() % (256);
            V[(opcode & kThirdNibbleMask) >> 8] = base & (opcode & kTwoNibbleMask);
            break;
        } 
        case 0xD000: {
            OpcodeDXYN(opcode);
            break;
        }
        case 0xE000:
            break;
        case 0xF000:
            OpcodeFXXX(opcode);
            break;
    };

}

void Chip8::Opcode8XXX(uint16_t opcode) {
    switch(opcode & kFirstNibbleMask) {
        uint16_t x;
        case 0x0000: 
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kSecondNibbleMask) >> 4]; 
            break;
        case 0x0001:
            V[(opcode & kThirdNibbleMask) >> 8] |= V[(opcode & kSecondNibbleMask) >> 4];
            break; 
        case 0x0002:
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] & V[(opcode & kSecondNibbleMask) >> 4];
            break; 
        case 0x0003: 
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] ^ V[(opcode & kSecondNibbleMask) >> 4];
            break;
        case 0x0004:
            x = V[(opcode & kThirdNibbleMask) >> 8] + V[(opcode & kSecondNibbleMask) >> 4];
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] + V[(opcode & kSecondNibbleMask) >> 4];
            if (x > 0xFF) {
                V[0xF] = 1;
            } else {
                V[0xF] = 0;
            }
            break;
        case 0x0005:
            if (V[(opcode & kThirdNibbleMask) >> 8] > V[(opcode & kSecondNibbleMask) >> 4]) {
                V[0xF] = 1;
            } else if (V[(opcode & kThirdNibbleMask) >> 8] < V[(opcode & kSecondNibbleMask) >> 4]) {
                V[0xF] = 0;
            }
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] - V[(opcode & kSecondNibbleMask) >> 4];
            break;
        case 0x0006: 
            V[0xF] = V[(opcode & kSecondNibbleMask) >> 4] & 0b0000000000000001;
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] >> 1;
            break;
        case 0x0007:
            if (V[(opcode & kThirdNibbleMask) >> 8] < V[(opcode & kSecondNibbleMask) >> 4]) {
                V[0xF] = 1;
            } else if (V[(opcode & kThirdNibbleMask) >> 8] > V[(opcode & kSecondNibbleMask) >> 4]) {
                V[0xF] = 0;
            }
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kSecondNibbleMask) >> 4] - V[(opcode & kThirdNibbleMask) >> 8];   
            break;
        case 0x000E:
            V[0xF] = (V[(opcode & kSecondNibbleMask) >> 4] & 0b1000000000000000) >> 15;
            V[(opcode & kThirdNibbleMask) >> 8] = V[(opcode & kThirdNibbleMask) >> 8] << 1;    
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
                    if ((gfx[((y + row) * 64) + x + col] == 1) && ((sprite & currPixel) >> (7 - col)) == 1) {
                        V[15] = 1;
                        gfx[((y + row) * 64) + x + col] = 0;
                    } else if (((sprite & currPixel) >> (7 - col)) == 1) {
                        gfx[((y + row) * 64) + x + col] = 1;
                    }
                    currPixel = currPixel >> 1;
                }
            }
            drawFlag = true;
}

void Chip8::OpcodeFXXX(uint16_t opcode) {
    switch (opcode & kTwoNibbleMask) {
        uint8_t n;
        case 0x0007:
            V[(opcode & kThirdNibbleMask) >> 8] = delayTimer;
            break;
        case 0x000A:
            SDL_Event *event;
            while (true) {
                SDL_WaitEvent(event);
                // if (event->type == SDL_EVENT_KEY_) {
                //     break;
                // }
            }

            V[(opcode & kThirdNibbleMask) >> 8] = 0; //KEYPRESS???
            break;
        case 0x0015:
            delayTimer = V[(opcode & kThirdNibbleMask) >> 8];
            break;
        case 0x0018:
            soundTimer = V[(opcode & kThirdNibbleMask) >> 8];
            break;
        case 0x001E:
            I += V[(opcode & kThirdNibbleMask) >> 8];
            break;
        case 0x0029:
            I = V[(opcode & kThirdNibbleMask) >> 8];
            break;
        case 0x0033:
            n = V[(opcode & kThirdNibbleMask) >> 8];
            memory[I] = (n / 100); memory[I + 1] = ((n / 10) % 10); memory[I + 2] = n % 10;
            break;
        case 0x0055:
            for (int i = I; i <= I + ((opcode & kThirdNibbleMask) >> 8); i++) {
                memory[i] = V[i - I];
            }
            I += 1 + ((opcode & kThirdNibbleMask) >> 8);
            break;
        case 0x0065:
            for (int i = I; i <= I + ((opcode & kThirdNibbleMask) >> 8); i++) {
                V[i - I] = memory[i];
            }
            I += 1 + ((opcode & kThirdNibbleMask) >> 8);
            break;
    }
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