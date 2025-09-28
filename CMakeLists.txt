cmake_minimum_required(VERSION 4.0)
project(game_of_life_c C)

set(CMAKE_C_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(game_of_life_c src/main.c)

target_link_libraries(game_of_life_c SDL3)
