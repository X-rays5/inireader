# inireader
A simple c++17 header only ini parser/writer.

For examples look at the [wiki](https://github.com/X-rays5/inireader/wiki)
# Basic usage

```cpp
#include <iostream>
#include <fstream>
#include <inireader/inireader.hpp>

/*
config.ini:

foo=bar
[Numbers]
float=3.14
*/

int main() {
    ini::Parser ini_file;
    try {
        ini_file.Parse("config.ini");
        ini_file.GetRootSection()["foo"] = "bar";
        auto float_val = ini_file["Numbers"]["float"].as<float>();

        std::cout << ini_file.GetRootSection()["foo"].as<std::string>() << " " << float_val << "\n";

        ini_file.GetRootSection().Add("foo", "bar");
        ini_file.AddSection("Hello World").Add("foo", "bar");

        std::ofstream writer("config.ini");
        if (writer.is_open())
            writer << ini_file.Stringify();
        writer.close();
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```
# CMake usage
```cmake
cmake_minimum_required(VERSION 3.24)
project(example)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(inireader)

add_executable(${PROJECT_NAME} main.cpp)

# Include the inireader headers via the cmake INTERFACE library.
target_link_libraries(${PROJECT_NAME} PRIVATE inireader::inireader)

```
