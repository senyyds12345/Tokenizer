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

## Inherit from the Parser class

If you find the tokenizer function too low-level, you can inherit from the Parser class. It provides various helper methods that streamline the parsing process.

When in use

```cpp
#include "Token.hpp"

class MyClass : Parser {
public:
    MyClass(const std::string& s) : Parser(s) {/*...*/}
    // ...
};
```

# New Features in v1.1.0

1. Expression Parsing with Shunting-yard (Dual-Stack Algorithm)

This version introduces a complete implementation of the Shunting-yard dual-stack algorithm for expression parsing. You can easily evaluate mathematical expressions or string operations by leveraging Parser's helper methods and defining your own rules.

2. Undefined Behavior (UB) Fix in ctype Functions

Previously, passing a potentially negative char (from std::string indexing) directly into isdigit or isspace could cause undefined behavior. This has been fixed by rigorously casting arguments to unsigned char before type judgment.

Usage Example: Expression Parser

Below is a sample implementation inheriting from Parser to evaluate expressions like 2 ^ 3 ^ 2 (with correct right-associativity) using the dual-stack algorithm:

```cpp
#include "Token.hpp"
#include <cmath>
#include <cstdio>

class ExprParser : public Parser {
private:
    static int prec(TokenType t) {
        switch (t) {
            case SUM: case MIN: return 5;
            case MUL: case DEL: return 6;
            case POW: return 7;
            default: return -1;
        }
    }

    void pushOpWithPrec(TokenType op) {
        bool rightAssoc = (op == POW);
        while (!ops.empty() && ops.top() != LP) {
            int topPrec = prec(ops.top());
            int curPrec = prec(op);
            if (topPrec > curPrec || (topPrec == curPrec && !rightAssoc))
                apply();
            else
                break;
        }
        ops.push(op);
    }

    void closeParen() {
        while (!ops.empty() && ops.top() != LP) apply();
        if (!ops.empty()) ops.pop(); // pop LP
    }

    void finish() {
        while (!ops.empty()) apply();
    }

    std::vector<Rule> rules = {
        {NUM, [this]() {
            pushVal(std::stod(std::string(cuVal)));
            eat();
        }},
        {STR, [this]() {
            std::string s(cuVal.substr(1, cuVal.size() - 2));
            pushVal(std::move(s));
            eat();
        }},
        {SUM, [this]() { pushOpWithPrec(SUM); eat(); }},
        {MIN, [this]() { pushOpWithPrec(MIN); eat(); }},
        {MUL, [this]() { pushOpWithPrec(MUL); eat(); }},
        {DEL, [this]() { pushOpWithPrec(DEL); eat(); }},
        {POW, [this]() { pushOpWithPrec(POW); eat(); }},
        {LP,  [this]() { ops.push(LP); eat(); }},
        {RP,  [this]() { closeParen(); eat(); }},
    };

public:
    using Parser::Parser;

    Value parseExpr() {
        while (cuTy != FIN && cuTy != RP)
            regex(rules);
        finish();
        Value r = std::move(vals.top()); vals.pop();
        return r;
    }
};
```

## Utility Functions

1. Constructor
Parser(const std::string& src)
Initializes the parser with source code, runs tokenizer to fill types and values vectors.

2. eat
void eat()
Consumes the current token and advances to the next one.

3. is_match
StatusCode is_match(size_t n, TokenType type, std::string_view value = "")
Checks if the token at specified index n matches the given type and optional value. Returns MATCH_STATUS_OK or MATCH_STATUS_FAILED.

4. expect
void expect(StatusCode match_return, const std::string& msg)
If match_return is MATCH_STATUS_FAILED, throws a std::runtime_error with the provided message.

5. peekPrev
TokenType peekPrev()
Returns the type of the previous token.

6. getType
TokenType getType(size_t n)
Returns the token type at the specified index. Returns UNK if out of bounds.

7. getVal
std::string_view getVal(size_t n)
Returns the token value at the specified index. Returns empty string if out of bounds.

8. getPos
size_t getPos()
Returns the current parsing position.

9. peek
template <TokenType type> bool peek() const
Checks if the next token matches the specified type.

10. regex
template <typename arr_t> void regex(const arr_t& action_t)
Iterates through an array of action rules. If the current token type matches a rule's type, executes the associated action.

# Stack Operation Functions (For Rule Actions)

When writing custom parsing logic in Rule actions, these helpers allow you to manipulate the dual stacks directly:

· apply(): Pops an operator and two operands, computes the result using the operator table, and pushes the result onto the value stack.

· pushVal(Value v): Pushes a value onto the value stack.

· pushOp(TokenType ty): Pushes an operator onto the operator stack.

· topVal(): Returns the top value of the value stack.

· topOp(): Returns the top operator of the operator stack.

· popVal(): Pops the top value from the value stack.

· popOp(): Pops the top operator from the operator stack.