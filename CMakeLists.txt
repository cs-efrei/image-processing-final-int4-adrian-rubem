cmake_minimum_required(VERSION 3.30)
project(image_processing C)

set(CMAKE_C_STANDARD 11)

add_executable(image_processing main.c bmp8.c)
