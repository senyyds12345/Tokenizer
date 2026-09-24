# Tokenizer

**[User Guide ->](./USERGUIDE.md)**

A high-performance, zero-copy C++20 lexer and expression parser. It tokenizes 200 lines of C++ code in 11 milliseconds.

## Usage

```cpp
#include "Token.hpp"
#include <vector>
#include <iostream>

int main() {
    std::string input = "def a(a): print(a)";
    std::vector<TokenType> types;
    std::vector<std::string_view> values;
    
    // Note: The order is src, types, values
    token::tokenizer(input, types, values);
    
    for (size_t i = 0; i < values.size(); i++) {
        std::cout << "Type: " << types[i] << ", Value: " << values[i] << "\n";
    }
    return 0;
}
```

# Features

· Zero-copy: returns std::string_view into the source string.
· Header-only: single include, no dependencies.
· Configurable: add new operators to the wordTable array.
· Fast: 11ms for 200 lines of C++ code.
· C++20: uses constexpr, std::array, std::variant.
· Dual-Stack Parser: Includes an inheritable Parser class implementing the Shunting-yard algorithm for easy expression parsing.

# Performance

Input Time
70 lines smali 0.014 s
200 lines C++ 0.011 s

# Token Types

NUM, SUM, MIN, MUL, DEL, LP, RP, COM, POW, FIN, PUN, STR, ENG, NE, GE, LE, EQ, GT, LT, ASS, SEM, NOT, E_FLOAT, COL, SCO, LCB, RCB, PRE, SHLE, SHRI, AND, OR, AT, BAND, BOR, UNK

# Extending

Add new operators in two places:

1. Add to the TokenType enum.
2. Add to the wordTable array.

```cpp
{"&&", AND}
```

Important: Longer symbols must come before shorter ones (e.g., "::" before ":").

# Requirements

· C++20 compiler
· No external dependencies

# License

MIT

# Author

senyyds12345