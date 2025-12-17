add_library(chip8 chip8.h chip8.cpp)

target_link_libraries(chip8 PUBLIC Boost::headers SDL3::SDL3)