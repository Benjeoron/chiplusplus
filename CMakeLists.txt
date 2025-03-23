cmake_minimum_required(VERSION 3.26)
set(PROJECTNAME "main")
project(${PROJECTNAME} C CXX)

find_package(SDL3 REQUIRED)

add_executable(${PROJECTNAME} )

target_sources(${PROJECTNAME} 
PRIVATE
    main.cpp
)

target_link_libraries(${PROJECTNAME} SDL3::SDL3)