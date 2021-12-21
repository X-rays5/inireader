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
    ini_file.Parse("test.ini");
    if (!ini_file.HasParseError()) {
        std::string root = ini_file.GetDefault("dsff_SDFsd");
        std::string float_val = ini_file["Numbers"]["float4"];

        std::cout << root << " " << float_val << "\n";

        ini_file.AddKVDefault("oh", "yes");
        ini_file.AddKV("hi", "oh", "yes");

        std::ofstream writer("test.ini");
        if (writer.is_open())
            writer << ini_file.Stringify();

    } else {
        std::cout << ini_file.GetParseError() << "\n";
    }
    return 0;
}
```
