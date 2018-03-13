cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(longnum_sam C)

include_directories(/opt/ti/include)
include_directories(../longnum_lib)
include_directories(../driverlib)

add_executable(longnum_sam
        main.c
        crypto.c
        )