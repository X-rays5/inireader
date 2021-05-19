# inireader
My first parser after 6 months of coding.

# Basic usage

```cpp
#include <iostream>
#include "inireader.hpp"

int main() {
    ini inireader;
    inireader.Parse("test.ini");
    if (!inireader.HasParseError()) {
        std::string weep = inireader.GetString("boop", "beep");
        inireader.AddKv("boop", "woop", "ai");
        std::cout << inireader.Stringify() << "\n";
    } else {
        std::cout << "[ERROR] " << inireader.GetParseError() << "\n";
    }
    return 0;
}
```
For further examples look at the [wiki](https://github.com/X-rays5/inireader/wiki)

# Know issues
A comment on a line where a comment declare character has been escaped won't work.  
Global kv aren't parsed right  
Some escapes don't work properly  
