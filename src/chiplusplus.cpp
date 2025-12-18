#define SDL_MAIN_USE_CALLBACKS 1
#define SDL_MAIN_CALLBACK_RATE 60
#define WINDOW_SCALE 15
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <ratio>
#include <thread>
#include <boost/algorithm/string.hpp>
#include "../lib/chip8.h"


Chip8 chip8 = Chip8();

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {   
    std::string filePath;
    if (argc < 2) {
        SDL_Log("No filepath given!");
        return SDL_APP_FAILURE;
    } else {
        filePath = argv[1];
        boost::algorithm::trim(filePath);

        if (!chip8.Initialize(WINDOW_SCALE, 10, ("Chip-8 Emulator: " + filePath).c_str())) {
            SDL_Log("Error initializing emulator");
            return SDL_APP_FAILURE;
        }

        if (!chip8.LoadProgram(filePath)) {
            SDL_Log("Filepath did not load!");
            return SDL_APP_FAILURE;
        }

    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT) {
        SDL_Log("%s", SDL_GetError());
        return SDL_APP_SUCCESS;  //ends program
    } else if (event->type == SDL_EVENT_KEY_UP || event->type == SDL_EVENT_KEY_DOWN) {
        SDL_KeyboardEvent* kbEvent = reinterpret_cast<SDL_KeyboardEvent*>(event);
        chip8.SetKeys();
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

std::chrono::system_clock::time_point a = std::chrono::system_clock::now();
std::chrono::system_clock::time_point b = std::chrono::system_clock::now();

SDL_AppResult SDL_AppIterate(void *appstate) {
    a = std::chrono::system_clock::now();
    auto val = a - b;
    if (((val.count()) / 1000000.0) >= (1000.0 / 60.0)) {
        // std::cout << "Time: ";
        // std::cout << ((val.count()) / 1000000.0) << "ms" <<std::endl;
        chip8.RunCycle();
        b = a;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("%s", SDL_GetError());
    SDL_Quit();
}   