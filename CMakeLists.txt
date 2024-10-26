cmake_minimum_required(VERSION 3.27)
project(UTP_project_board_game_)

set(CMAKE_CXX_STANDARD 20)

set(JAVA_HOME "C:/Program Files/Java/jdk-23")
include_directories(${JAVA_HOME}/include)
include_directories(${JAVA_HOME}/include/win32)


add_library(UTP_project_board_game_ SHARED library.cpp
        GoBoard.h
)
target_link_options(UTP_project_board_game_ PRIVATE "-static")
