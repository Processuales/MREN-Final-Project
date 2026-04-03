cmake_minimum_required(VERSION 4.0)
project(MREN178_Project_Team4 C)

set(CMAKE_C_STANDARD 11)

add_executable(MREN178_Project_Team4 src/main.c
        src/common.h
        src/parser.c
        src/linear_structures.c
        src/tree_structures.c
)
