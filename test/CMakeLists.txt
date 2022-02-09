cmake_minimum_required(VERSION 3.18)
project(test_inireader)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_inireader test.cpp ../include/inireader/conversion.hpp)

target_link_libraries(test_inireader gtest gtest_main)
add_test(test_inireader test_inireader)