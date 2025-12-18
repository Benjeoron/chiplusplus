add_executable(${PROJECTNAME} chiplusplus.cpp)

target_link_libraries(
    chiplusplus PRIVATE
    chip8 PRIVATE
    Boost::headers
    SDL3::SDL3
    )