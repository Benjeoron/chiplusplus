#ifndef CHIP8_H
#define CHIP8_H
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define PROGRAM_START 0x200
#include <string>
#include <SDL3/SDL.h>

static constexpr uint8_t font[] = {
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


class Chip8 {
    private:
        uint8_t memory[4096] = {0};
        //0x000 to 0x1FF interpreter and fonts
        //0x050 to 0x0A0 4x5 pixel font set
        //0x200 to 0xFFF program and work RAM
        uint8_t V[16] = {0};
        uint16_t I; 
        uint16_t pc;
        uint16_t opcode;
        
        uint8_t delayTimer;
        uint8_t soundTimer;
        
        uint16_t stack[16] = {0};
        uint16_t sp;
        uint8_t key_old[16] = {0};
        uint8_t key_new[16] = {0};

        uint8_t gfx[64 * 32] = {0};
        bool drawFlag;

        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        void RenderScreen();

        void RunInstruction();
        void Opcode8XXX(uint16_t opcode);
        void OpcodeDXYN(uint16_t opcode);
        void OpcodeFXXX(uint16_t opcode);

        int vidScale;
        
    public:
        bool Initialize(int vidScale, int pitch, char const* title);
        void SetKeys();
        bool LoadProgram(std::string filePath);
        void PrintProgram();
        void RunCycle();
        ~Chip8();
};

#endif
