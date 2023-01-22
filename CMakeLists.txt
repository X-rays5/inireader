cmake_minimum_required(VERSION 3.16)
project(inireader)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

target_include_directories(${PROJECT_NAME} INTERFACE include)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  message(STATUS "Loading test/CMakeLists.txt")
  add_subdirectory(test)
endif()