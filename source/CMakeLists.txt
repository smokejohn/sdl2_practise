cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(sdl2_practise LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
message("${CMAKE_MODULE_PATH}")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
# find_package(SDL2_ttf REQUIRED)

set(Sources
    main.cpp
    )

add_executable(sdl2test
    ${Sources}
    )

target_link_libraries(sdl2test 
    PRIVATE SDL2::SDL2 
    PRIVATE SDL2_image)
    # PRIVATE SDL2_ttf)

# custom command to copy resource files to build directory later
# add_custom_command(
#     TARGET sdl2test POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy
#             ${CMAKE_SOURCE_DIR}../resources/hello_world.bmp
#             ${CMAKE_CURRENT_BINARY_DIR}/resources/hello_world.bmp
#     )
