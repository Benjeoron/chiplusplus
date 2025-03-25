cmake_minimum_required(VERSION 3.26)
set(PROJECTNAME "chiplusplus")
project(${PROJECTNAME} C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED)

add_executable(${PROJECTNAME} src/chiplusplus.cpp)

target_sources(${PROJECTNAME} 
    PRIVATE PRIVATE src/chiplusplus.cpp)

target_include_directories(${PROJECTNAME} PRIVATE include)

target_link_libraries(${PROJECTNAME} SDL3::SDL3)