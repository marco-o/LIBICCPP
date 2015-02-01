cmake_minimum_required(VERSION 2.6)
project(libicc++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ferror-limit=4")
include_directories(src test)

add_executable(libicc++test src/iccpp_profile.cpp test/function_test.cpp 
               test/test_main.cpp test/profiles_test.cpp test/iccpp_lut_funct_test.cpp)