cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(longnum_lib C)

add_library(longnum SHARED longnum.c)
add_library(longnum-static STATIC longnum.c)
