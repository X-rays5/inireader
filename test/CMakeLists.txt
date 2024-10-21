cmake_minimum_required(VERSION 3.16)
project(test_inireader)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/googletest)
enable_testing()

add_executable(${PROJECT_NAME} test_main.cpp test_parse.cpp test_add.cpp test_remove.cpp test_has.cpp test_edit.cpp test_conversion.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} gtest gtest_main)

add_test(inireader ${PROJECT_NAME})
