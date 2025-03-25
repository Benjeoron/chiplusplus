#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "chippy.h"

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
        Chippy chip8 = Chippy();
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


