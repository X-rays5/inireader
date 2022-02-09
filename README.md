# inireader
Simple ini parser that uses regex 

For examples look at the [wiki](https://github.com/X-rays5/inireader/wiki)
# Basic usage

```cpp
#include <iostream>
#include <fstream>
#include "inireader.hpp"

int main() {
    ini::Parser ini_file;
    try {
        ini_file.load("config.ini");
        ini_file.GetRootSection()["foo"] = "bar";
        auto float_val = ini_file["Numbers"]["float"].as<float>();

        std::cout << ini_file.GetRootSection()["foo"] << " " << float_val << "\n";

        ini_file.GetRootSection().Add("foo", "bar");
        ini_file.AddSection("Hello World").Add("foo", "bar");

        std::ofstream writer("config.ini");
        if (writer.is_open())
            writer << ini_file.Stringify();
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```
