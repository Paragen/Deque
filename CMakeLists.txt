cmake_minimum_required(VERSION 3.6)
project(Deque)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")
set(SOURCE_FILES main.cpp deque.h dumb_external_deque.h util.h external_deque.h)
add_executable(Deque ${SOURCE_FILES})
target_link_libraries(Deque gmp)