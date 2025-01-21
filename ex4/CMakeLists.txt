cmake_minimum_required(VERSION 3.22)
project(ex4)

set(CMAKE_CXX_STANDARD 11)

add_executable(ex4 MemoryConstants_test2.h PhysicalMemory.h PhysicalMemory.h PhysicalMemory.cpp VirtualMemory.h
        VirtualMemory.cpp test2_write_one_page_twice_and_read.cpp)
