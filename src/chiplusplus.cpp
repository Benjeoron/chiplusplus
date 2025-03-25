#include <thread>
#include <chrono>
#include <string>
#include <fstream> 
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "chip8.h"

bool Chip8::initialize() {
    this -> I = 0;
    this -> pc = 0x200;
    this -> delayTimer = 0;
    this -> soundTimer = 0;
    this -> sp = 0; 
    unsigned char fontPtr = 0x050;
    for (int i = 0; i < 80; i++) {
        memory[fontPtr + i] = font[i];
    }
    this -> drawFlag = true;
    if (!SDL_CreateWindowAndRenderer("examples/renderer/points", 640, 320, 0, &window, &renderer)) {
        return false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);  /* black, full alpha */
    SDL_RenderClear(renderer);  /* start with a blank canvas. */
    return true;
}

void Chip8::setKeys() {

}

void Chip8::runCycle() {
    for (int i = 0; i < 11; i++) {
        this -> runInstruction();
    }
}

bool Chip8::loadProgram(std::string filePath) {
    std::ifstream input = std::ifstream(filePath, std::ios::binary);
    if (!input.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return false;
    } else {

        input.seekg (0, input.end);
        int length = input.tellg();
        input.seekg (0, input.beg);

        char * buffer = new char [length];
        input.read(buffer, length);

        unsigned short loadPtr = 0x200;
        for (int i = 0; i < length; i++) {
            memory[loadPtr + i] = buffer[i];
        }

        delete[] buffer;
        return true;
    }
}

void Chip8::runInstruction() {
    this -> opcode = memory[pc] << 8 | memory[pc + 1]; //fetch
    pc += 2;
    
    switch (opcode & 0xF000){
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0:
                    memset(gfx, 0, sizeof(gfx));
                    break;
                case 0x00EE:
                    sp -= 1;
                    pc = stack[sp];
                    break;
            }
            break;
        case 0x1000:
            pc = opcode - 0x1000;
            break;
        case 0x2000:
            sp += 1;
            stack[sp + 1] = opcode - 0x2000;
            break;
        case 0x3000:
            if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 2;
            }
            break;
        case 0x4000:
            if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 2;
            }
            break;
        case 0x5000:
            if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            break;
        case 0x6000:
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;
        case 0x7000:
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;
        case 0x8000:
            switch(opcode & 0x000F) {
                unsigned char temp;
                case 0x0000:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0001:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                    break; 
                case 0x0002:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                    break; 
                case 0x0003: 
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4];
                    break;
                case 0x0004:
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] + V[(opcode & 0x00F0) >> 4];
                    if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    break;
                case 0x0005:
                    if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4]) {
                        temp = 0;
                    } else {
                        temp = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] - V[(opcode & 0x00F0) >> 4];
                    V[15] = temp;
                    break;
                case 0x0006: 
                    V[0xF] = V[(opcode & 0x00F0) >> 4] & 0b0000000000000001;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] >> 1;
                    break;
                case 0x0007:
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) {
                        temp = 0;
                    } else {
                        temp = 1;
                    }
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[opcode & 0x0F00];
                    V[15] = temp;    
                    break;
                case 0x000E:
                    V[0xF] = V[(opcode & 0x00F0) >> 4] & 0b1000000000000000;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] << 1;    
                    break;
            }
            break;
        case 0x9000:
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            }
            break;
        case 0xA000:
            I = opcode - 0xA000;
            break;
        case 0xB000:
            pc = opcode - 0xB000 + V[0];
            break;
        case 0xC000: {
            unsigned char base = std::rand() % (256);
            V[(opcode & 0x0F00) >> 8] = base & (opcode & 0x00FF);
            break;
        } 
        case 0xD000:
            //TODO
            unsigned char x = V[(opcode & 0x0F00) >> 8] & 63;
            unsigned char y = V[(opcode & 0x00F0) >> 4] & 31;
            V[15] = 0;
            for (int i = 0; i < opcode & 0x000F; i++) {
                unsigned char sprite = memory[I + i];
                if (y + i == 32) {
                    break;
                }
                for (int j = 0; j < 8; j++) {
                    if (x + j == 64) {
                        break;
                    }
                    if (gfx[(y + i) * 64 + x + j] == 1 && (sprite & 0b10000000) == 1) {
                        V[15] = 1;
                        gfx[(y + i) * 64 + x + j] = 0;
                    } else if ((sprite & 0b10000000) == 1) {
                        gfx[(y + i) * 64 + x + j] = 1;
                    }
                    sprite = sprite << 1;
                }
            }
            drawFlag = true;
            break;
        case 0xE000:
            break;
        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    V[(opcode & 0x0F00) >> 8] = delayTimer;
                    break;
                case 0x000A:
                    V[(opcode & 0x0F00) >> 8] = 0; //KEYPRESS???
                    break;
                case 0x0015:
                    delayTimer = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0018:
                    soundTimer = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x001E:
                    I += V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0029:
                    I = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0033:
                    memory[I], memory [I + 1], memory[I + 2] = V[(opcode & 0x0F00) >> 8];
                    break;
                case 0x0055:
                    for (int i = I; i <= I + ((opcode & 0x0F00) >> 8); i++) {
                        memory[i] = V[i - I];
                    }
                    I += 1 + ((opcode & 0x0F00) >> 8);
                    break;
                case 0x0065:
                    for (int i = I; i <= I + ((opcode & 0x0F00) >> 8); i++) {
                        V[i - I] = memory[i];
                    }
                    I += 1 + ((opcode & 0x0F00) >> 8);
                    break;
            }
            break;
        

    };

}

//DEBUG
void Chip8::printProgram() {
    for (int i = 0x200; i < 4096; i++) {
        std::cout << std::hex << (int) memory[i] << " ";
        if (i % 80 == 0) {
            std::cout << std::endl;
        }
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    std::string filePath;
    if (argc == 1) {
        std::cerr << "No filepath or option given!";
        SDL_Quit();
        return 1;
    } else {
        if (argv[1] == "--ibm") {
            filePath = "../lib/IBM Logo.ch8";
        }
        Chip8 chip8 = Chip8();
        chip8.initialize();
        // if (chip8.loadProgram(filePath)) {
        //     while (true) {
        //         chip8.runCycle();
        //     }
        // }
        SDL_Quit();
        return 0;
    }    
}