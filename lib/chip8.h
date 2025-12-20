#ifndef CHIP8_H
#define CHIP8_H
#define GFX_WIDTH 64
#define GFX_HEIGHT 32
#define PROGRAM_START 0x200
#define KEYPAD_SIZE 15
#include <string>
#include <vector>
#include <SDL3/SDL.h>

enum Chip8Mode {
    COSMAC_VIP
};

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

// Provides a handy dandy conversion from the hexadecimal COSMAC VIP key index to the actual 
// physical bindings on modern QWERTY keyboards.
// Whereas the COSMAC VIP used the following scheme:
// 123C
// 456D
// 789E
// A0BF
// This binding uses the following one:
// 1234
// QWER
// ASDF
// ZXCV
static constexpr int keypad[] = {
    SDL_SCANCODE_X,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_C,
    SDL_SCANCODE_4,
    SDL_SCANCODE_R,
    SDL_SCANCODE_F,
    SDL_SCANCODE_V,
};

// Constants for byte masking
static constexpr uint16_t kFirstNibbleMask = 0x000F;
static constexpr uint16_t kSecondNibbleMask = 0x00F0;
static constexpr uint16_t kThirdNibbleMask = 0x0F00;
static constexpr uint16_t kFourthNibbleMask = 0xF000;
static constexpr uint16_t kTwoNibbleMask = 0x00FF;
static constexpr uint16_t kThreeNibbleMask = 0x0FFF;
static constexpr uint8_t kU8MSB = 0x80;
static constexpr int kU8MSBShift = 7;

class Chip8 {
    private:
        Chip8Mode mode;

        // Simulating the 4 KB of memory on most CHIP-8 implementations
        // 0x000 to 0x1FF interpreter and fonts
        // 0x050 to 0x0A0 4x5 pixel font set
        // 0x200 to 0xFFF program and work RAM
        uint8_t memory[4096] = {0};

        // The 16 V registers specified for the CHIP-8 language
        uint8_t v[16] = {0};

        // The I register
        uint16_t I; 
        
        // Stores the index of the next instruction in memory
        uint16_t pc;

        uint16_t stack[16] = {0};
        uint16_t sp;
        
        // These two count down every visual frame, aka 60hz
        uint8_t delay_timer;
        uint8_t sound_timer;

        // Represents the screen to be displayed.
        uint8_t gfx[64 * 32] = {0};
        
        int vid_scale;  // Per Google C++ guide, sometimes an int is just an int.

        // Typical SDL rendering setup
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;

        void Opcode8XYN(uint16_t opcode);
        void OpcodeDXYN(uint16_t opcode);
        void OpcodeFXNN(uint16_t opcode);
    public:
        // Stop running instructions for the current frame if this is set, then set it to be false.
        bool draw_flag;

        Chip8(Chip8Mode chip8_mode) : mode(chip8_mode) {}

        // MUST be run before use.
        bool Initialize(int vidScale, int pitch, char const* title);

        // Ditto as above.
        bool LoadProgram(std::string filePath);
        // Debug function for printing the loaded program.

        void PrintProgram();
        // To replicate CHIP-8 clock speeds, run at ~600hz
        void RunCycle();

        // Run at 60hz
        void UpdateTimers();

        // Run at 60hz
        void RenderScreen();

        ~Chip8();
};

#endif
