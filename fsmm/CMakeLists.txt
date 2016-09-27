cmake_minimum_required(VERSION 2.6)
project(fsmm)

add_executable(fsmm main.cpp)
set_property(TARGET fsmm PROPERTY CXX_STANDARD 11)

install(TARGETS fsmm RUNTIME DESTINATION bin)
