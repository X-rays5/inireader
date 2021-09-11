#include <iostream>
#include "../inireader.hpp"
#include <filesystem>

int main() {
    ini::Parser ini_file;
    ini_file.Parse("test.ini");
    if (!ini_file.HasParseError()) {
        std::string root = ini_file.GetDefault("dsff_SDFsd");
        std::string float_val = ini_file["Numbers"]["float4"];

        std::cout << root << " " << float_val << "\n";
    } else {
        std::cout << ini_file.GetParseError() << "\n";
    }
    return 0;
}