cmake_minimum_required(VERSION 3.6)
project(alex)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    fa.cpp
    fa.h
    main.cpp
    regex.cpp
    regex.h
    script.cpp
    script.h
    stdafx.h)

add_executable(alex ${SOURCE_FILES})