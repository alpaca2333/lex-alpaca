cmake_minimum_required(VERSION 3.5)
project(alex)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp nfa.h stdafx.h nfa.cpp regex.cpp regex.h)
add_executable(alex ${SOURCE_FILES})