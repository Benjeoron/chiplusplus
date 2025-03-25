#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include "chippy.h"

int main(int argc, char *argv[]) {
    std::string filePath;
    if (argc == 1) {
        std::cerr << "No filepath or option given!";
        return 1;
    } else {
        if (argv[1] == "--ibm") {
            filePath = "../lib/IBM Logo.ch8";
        }
        Chippy chip8 = Chippy();
        chip8.initialize();
        if (chip8.loadProgram(filePath)) {
            while (true) {
                chip8.runCycle();
            }
        }
        return 0;
    }    
}


