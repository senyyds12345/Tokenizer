# Start quickly

`Token.hpp` provides a `token::tokenizer` function in the `token` namespace. It requires three parameters:

1. The input string to be tokenized.
2. A container (e.g., `std::vector<TokenType>`) that supports `push_back` to store token types.
3. A container (e.g., `std::vector<std::string_view>`) that supports `push_back` to store token values.

The function fills both containers with the corresponding token data, and you can then process them as needed.

## Sample code

```cpp
#include <vector>
#include "Token.hpp"

int main() {
    std::vector<TokenType> types;
    std::vector<std::string_view> values;
    token::tokenizer("1 + 1 = 2", types, values);
    // ...
}
```

# Inherit from the Parser class

If you find the tokenizer function too low-level, you can inherit from the Parser class. It provides various helper methods that streamline the parsing process.

## When in use

```cpp
#include "Token.hpp"

class MyClass : Parser {
    MyClass(const std::string& s) : Parser(s) {/*...*/}
    // ...
};
```
## Utility Functions

**Constructor**  
`Parser(const std::string& src)`  
Initializes the parser with source code, runs tokenizer to fill `types` and `values` vectors.

---

**eat**  
`void eat()`  
Consumes the current token and advances to the next one.

---

**is_match**  
`StatusCode is_match(TokenType type, std::string_view value = "", int n = -1)`  
Checks if the token at specified position matches the given type and optional value. Returns `MATCH_STATUS_OK` or `MATCH_STATUS_FAILED`.

---

**except**  
`void except(StatusCode match_return, const std::string& msg)`  
If `match_return` is `MATCH_STATUS_FAILED`, throws a `std::runtime_error` with the provided message.

---

**peekPrev**  
`TokenType peekPrev()`  
Returns the type of the previous token.

---

**getType**  
`TokenType getType(int n)`  
Returns the token type at the specified index. Returns `UNKNOWN` if out of bounds.

---

**getVal**  
`std::string_view getVal(int n)`  
Returns the token value at the specified index. Returns empty string if out of bounds.

---

**getPos**  
`int getPos()`  
Returns the current parsing position.

---

**peek**  
`template <TokenType type> bool peek() const`  
Checks if the next token matches the specified type.

---

**regex**  
`template <typename arr_t> void regex(const arr_t& action_t)`  
Iterates through an array of action rules. If the current token type matches a rule's type, executes the associated action.