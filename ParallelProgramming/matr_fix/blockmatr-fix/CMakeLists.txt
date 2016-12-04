cmake_minimum_required(VERSION 3.6)
project(blockmatr)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        blockmatr.h
    main.cc)

add_executable(blockmatr ${SOURCE_FILES})