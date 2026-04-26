
# Tokenizer

A high-performance, zero-copy C++20 lexer that tokenizes 200 lines of C++ code in 11 milliseconds.

## Usage

```cpp
#include "Token.hpp"
#include <vector>

int main() {
    std::string input = "def a(a): print(a)";
    std::vector<std::string> values;
    std::vector<TokenType> types;
    
    token::tokenizer(input, values, types);
    
    for (size_t i = 0; i < values.size(); i++) {
        std::cout << "Type: " << types[i] << ", Value: " << values[i] << "\n";
    }
    return 0;
}
```

Features

· Zero-copy: returns std::string_view into the source string
· Header-only: single include, no dependencies
· Configurable: add new operators to wordTable array
· Fast: 11ms for 200 lines of C++ code
· C++20: constexpr, std::array, std::variant

Performance

Input Time
70 lines smali 0.014 sec
200 lines C++ 0.011 sec

Token Types

NUM, SUM, MIN, MUL, DEL, LP, RP, COM, POW, ENG, NE, GTOET, LTOET, ET, GT, LT, ASS, SEM, COL, SCO, PRE, SHLE, SHRI, E_FLOAT, UNKNOWN

Extending

Add new operators in two places:

1. Add to TokenType enum
2. Add to wordTable array

```cpp
{"&&", LOGICAL_AND}
```

Longer symbols must come before shorter ones (e.g., "::" before ":").

Requirements

· C++20 compiler
· No external dependencies

License

MIT

Author

senyyds12345