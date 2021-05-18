#include <iostream>
#include "../inireader.hpp"

int main() {
    ini inireader;
    inireader.Parse("../test.ini");
    if (!inireader.HasParseError()) {
        std::cout << "Version " << inireader.GetString("settings", "version") << "\n";
        inireader.AddKv("settings", "date", "monday");
        inireader.AddSection("emptysection");
        std::cout << inireader.Stringify() << "\n";
    } else {
        std::cout << "[ERROR] " << inireader.GetParseError() << "\n";
    }
    system("pause");
    return 0;
}