cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(sdl2_practise LANGUAGES CXX)

find_package(SDL2 REQUIRED)

set(Sources
    main.cpp
    )

add_executable(sdl2test
    ${Sources}
    )

target_link_libraries(sdl2test PRIVATE SDL2::SDL2)
