# jp::lexer

A high-performance, zero-copy C++20 lexer that tokenizes 200 lines of C++ code in 11 milliseconds.

## Features

- Zero-copy: returns std::string_view into the source string
- Header-only: single include, no dependencies
- Configurable: add new operators by appending to the wordTable array
- Fast: parses smali, Python, SQL, and C++ in milliseconds
- Modern C++20: constexpr, std::array, std::variant

## Quick Start

```cpp
#include "Token.hpp"
#include <iostream>

int main() {
    std::string input = "def a(a): print(a)";
    Lexer lex(input);
    
    Token* tok;
    while ((tok = lex.nextToken())->type != END) {
        std::cout << "Type: " << tok->type << ", Value: " << tok->value << "\n";
    }
    return 0;
}
```

Performance

Input Time
70 lines of smali 0.014 sec
200 lines of C++ 0.011 sec

The lexer runs faster than human perception. Execution time is dominated by printf overhead, not tokenization.

Supported Token Types

NUM, SUM, MIN, MUL, DEL, LP, RP, COM, POW, ENG, NE, GTOET, LTOET, ET, GT, LT, ASS, SEM, COL, SCO, PRE, SHLE, SHRI

Extending

Add new operators in two places:

1. Add to the TokenType enum
2. Add to the wordTable array

```cpp
{"&&", LOGICAL_AND},  // example
```

Longer symbols must come before shorter ones (e.g., "::" before ":").

Requirements

· C++20 compiler
· No external dependencies

License

MIT License

Author

senyyds12345
