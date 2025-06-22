#ifndef CHIP8_H
#define CHIP8_H
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define PROGRAM_START 0x200
#include <string>
#include <SDL3/SDL.h>
class Chip8{
    private:
        unsigned char memory[4096] = {0};
        //0x000 to 0x1FF interpreter and fonts
        //0x050 to 0x0A0 4x5 pixel font set
        //0x200 to 0xFFF program and work RAM
        unsigned char V[16] = {0};
        unsigned short I; 
        unsigned short pc;
        unsigned short opcode;
        
        unsigned char delayTimer;
        unsigned char soundTimer;
        
        unsigned short stack[16] = {0};
        unsigned short sp;
        unsigned char key[16] = {0};
        static constexpr unsigned char font[] = {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };


        unsigned char gfx[64 * 32] = {0};
        bool drawFlag;

        void runInstruction();

        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        void draw();
        int vidScale;
        
    public:
        bool initialize(int vidScale, int pitch, char const* title);
        void setKeys();
        bool loadProgram(std::string filePath);
        void printProgram();
        void runCycle();
        ~Chip8();
};

#endif
