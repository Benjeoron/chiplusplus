cmake_minimum_required(VERSION 3.26)
set(PROJECTNAME "chiplusplus")
project(${PROJECTNAME} C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(${PROJECTNAME} src/chiplusplus.cpp)
add_library(chip8 src/chip8.cpp src/chip8.h)

target_sources(${PROJECTNAME} 
    PRIVATE PRIVATE src/chiplusplus.cpp)

target_link_libraries(${PROJECTNAME} PRIVATE chip8 SDL3::SDL3)