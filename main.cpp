#include <unordered_map>
#include <iostream>
#include <cmath>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

const void* MEM_START = malloc(4096);
constexpr int kScreenWidth{ 128 };
constexpr int kScreenHeight{ 64 };
const std::unordered_map<char, int> HEX_VALS({{'0', 0}, {'1', 1}, {'2',2}, {'3', 3}, {'4', 4}, 
        {'5', 5}, {'6', 6}, {'7', 7}, {'8', 8}, {'9', 9}, {'A', 10}, {'B', 11}, {'C', 12},
        {'D', 13}, {'E', 14}, {'F', 15}});

/* Global Variables */
//The window we'll be rendering to
SDL_Window* gWindow{ nullptr };
    
//The surface contained by the window
SDL_Surface* gScreenSurface{ nullptr };

//The image we will load and show on the screen
SDL_Surface* gHelloWorld{ nullptr };

int hexToDec(std::string hex) {
    int ret = 0;
    int len = hex.size();
    for (char c : hex) {
        len -= 1;
        ret += HEX_VALS.at(c) * std::pow(16, len);
    }
    return ret;
}

bool init() {
    //Initialization flag
    bool success{ true };

    //Initialize SDL
    if( !SDL_Init( SDL_INIT_VIDEO ) )
    {
        SDL_Log( "SDL could not initialize! SDL error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        if( gWindow = SDL_CreateWindow( "SDL3 Tutorial: Hello SDL3", kScreenWidth, kScreenHeight, 0 ); gWindow == nullptr )
        {
            SDL_Log( "Window could not be created! SDL error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
        }
    }

    return success;
}

int main(int argc, char *argv[]) {
    // for(int i = 1; i < argc; i++)  {
    //     std::cout << hexToDec(argv[i]);
    // }
    
    init();
    //The main loop
    bool quit{ false };

    //The event data
    SDL_Event e;
    SDL_zero( e );
    while( quit == false )
    {
        //Get event data
        while( SDL_PollEvent( &e ) )
        {
            //If event is quit type
            if( e.type == SDL_EVENT_QUIT )
            {
                //End the main loop
                quit = true;
            }

            SDL_FillSurfaceRect( gScreenSurface, nullptr, SDL_MapSurfaceRGB( gScreenSurface, 0xFF, 0xFF, 0xFF ) );

            //Render image on screen
            SDL_BlitSurface(gScreenSurface, nullptr, gScreenSurface, nullptr );
        }
    }
    return 0;
}
// void* getAdd(std::string hex) {

// }

