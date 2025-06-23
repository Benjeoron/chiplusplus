#define SDL_MAIN_USE_CALLBACKS 1
#define WINDOW_SCALE 15
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <boost\algorithm\string.hpp>
#include "chip8.h"
using namespace boost::algorithm;

Chip8 chip8 = Chip8();

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{   
    // for (int i = 0; i < argc; i++) std::cout << argv[i] <<std::endl;
    std::string filePath;
    if (argc < 2) {
        SDL_Log("No filepath given!");
        return SDL_APP_FAILURE;
    } else {
        filePath = argv[1];
        trim(filePath);
        if (filePath.compare("--ibm") == 0 || filePath.compare("-i") == 0) {
            filePath = "../bin/IBM Logo.ch8";
        }


        if (!chip8.initialize(WINDOW_SCALE, 10, ("Chip-8 Emulator: " + filePath).c_str())) {
            SDL_Log("Error initializing emulator");
            return SDL_APP_FAILURE;
        }

        if (!chip8.loadProgram(filePath)) {
            SDL_Log("Filepath did not load!");
            return SDL_APP_FAILURE;
        }

    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        SDL_Log("%s", SDL_GetError());
        return SDL_APP_SUCCESS;  //ends program
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    chip8.runCycle();
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) 
{
    SDL_Log("%s", SDL_GetError());
    SDL_Quit();
}   