cmake_minimum_required(VERSION 3.16)
project(dither)

set(CMAKE_CXX_STANDARD 17)

find_package(PNG REQUIRED) # On Ubuntu, $sudo apt install libpng-dev

add_executable(dither
        src/main.cpp
        ${PNG_INCLUDE_DIRS}/png.h
        src/PNG_Loader.cpp
        src/PNG_Loader.h
        src/PNG_RGBA.cpp
        src/PNG_RGBA.h
        src/PNG_structs.h
        src/ColorPalette.cpp
        src/ColorPalette.h src/PNG_RGB_Array.cpp src/PNG_RGB_Array.h)

target_link_libraries(dither ${PNG_LIBRARIES})