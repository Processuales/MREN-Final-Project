cmake_minimum_required(VERSION 4.0)
project(Data_Structure_Viewer)

set(CMAKE_CXX_STANDARD 20)

add_executable(Data_Structure_Viewer src/main.c
        src/common.h
        src/parser.c
        src/parser.h
        src/linear_structures.c
        src/tree_structures.c
)
