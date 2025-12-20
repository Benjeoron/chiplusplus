#define SDL_MAIN_USE_CALLBACKS 1
#define FRAME_RATE 60
#define FRAME_SYNC_CONSTANT 2
#define WINDOW_SCALE 15
#define VIDEO_PITCH 10
#define INSTRUCTIONS_PER_FRAME 11
#define MS_PER_SEC 1000.0
#define NS_PER_MS 1000000.0

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <ratio>
#include <thread>
#include <boost/algorithm/string.hpp>
#include "../lib/chip8.h"


Chip8 chip8 = Chip8(COSMAC_VIP);

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {   
    std::string filePath;
    if (argc < 2) {
        SDL_Log("No filepath given!");
        return SDL_APP_FAILURE;
    } else {
        filePath = argv[1];
        boost::algorithm::trim(filePath);

        if (!chip8.Initialize(WINDOW_SCALE, VIDEO_PITCH, 
                              ("Chip-8 Emulator: " + filePath).c_str())) {
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
        return SDL_APP_FAILURE;  //ends program
    } else if (event->type == SDL_EVENT_KEY_UP || event->type == SDL_EVENT_KEY_DOWN) {
        SDL_KeyboardEvent* kb_event = reinterpret_cast<SDL_KeyboardEvent*>(event);
        if (kb_event->scancode == SDL_SCANCODE_ESCAPE) {
            return SDL_APP_SUCCESS; 
        }
    }
    return SDL_APP_CONTINUE;
}

std::chrono::system_clock::time_point a, b = std::chrono::system_clock::now();

SDL_AppResult SDL_AppIterate(void *appstate) {
    a = std::chrono::system_clock::now();
    auto frame_diff = a - b;
    if (((frame_diff.count()) / NS_PER_MS) >= (MS_PER_SEC / FRAME_RATE)) {
        b = a;
        // std::cout << "Framerate: ";
        // std::cout << (1000.0 / ((frame_diff.count()) / 1000000.0)) << "hz" << std::endl;

        for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++) {
            if (chip8.draw_flag) {
                chip8.draw_flag = false;
                break;
            }
            chip8.RunCycle();
        }
        chip8.RenderScreen();
        chip8.UpdateTimers();
        
        SDL_Delay((MS_PER_SEC / FRAME_RATE) - FRAME_SYNC_CONSTANT 
                  - ((a - std::chrono::system_clock::now()).count() / NS_PER_MS));
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("%s", SDL_GetError());
    SDL_Quit();
}   