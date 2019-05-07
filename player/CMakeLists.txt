cmake_minimum_required (VERSION 3.2)


set(CMAKE_C_COMPILE_FLAGS "${CMAKE_C_COMPILE_FLAGS} -std=gnu11 -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og -ggdb -fprofile-arcs -ftest-coverage -pg -fsanitize=address -fsanitize=leak -fsanitize=undefined")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fopenmp")

project (lhe_pi_player C)

add_executable(lhe_pi_player entropic_decoder.c quantizer_decoder.c upsampler_decoder.c player_decoder.c decoder.c get_bits.c image_yuv.c main.c)
target_link_libraries(lhe_pi_player -lSDL2 -lm -fopenmp)