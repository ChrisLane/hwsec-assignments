cmake_minimum_required(VERSION 3.9)
set(CMAKE_C_STANDARD 11)

project(present_ref C)

include_directories(/opt/ti/include)
include_directories(../driverlib)

add_executable(present_opt
        main.c
        crypto.c
)
