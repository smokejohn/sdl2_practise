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

# custom command to copy resource files to build directory later
# add_custom_command(
#     TARGET sdl2test POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#             ${CMAKE_SOURCE_DIR}../resources/hello_world.bmp
#             ${CMAKE_CURRENT_BINARY_DIR}/resources/hello_world.bmp
#     )
