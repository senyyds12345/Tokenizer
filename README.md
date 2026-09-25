# Tokenizer

**[User Guide ->](./USERGUIDE.md)**

A high-performance, zero-copy C++20 lexer and expression parser toolkit. It tokenizes 200 lines of C++ code in 11 milliseconds, and provides an inheritable `Parser` base class with a built-in dual-stack expression evaluator plus a lightweight AST builder.

## Features

- **Zero-copy**: every token is a `std::string_view` pointing into the original source string.
- **Header-only**: single `#include "Token.hpp"`, no external dependencies.
- **Configurable**: add new operators by editing the `wordTable` array.
- **Fast**: 11 ms for 200 lines of C++ code.
- **C++20**: uses `constexpr`, `std::array`, `std::variant`, `std::string_view`.
- **Dual-Stack Parser**: inheritable `Parser` class implementing the shunting-yard algorithm.
- **AST Builder**: `AST` class with factory methods and variable environment, ready for user-defined evaluation.
- **Type-safe values**: `std::variant<double, std::string, bool>` for runtime values.

## Quick Start

```cpp
#include "Token.hpp"
#include <iostream>
#include <vector>

int main() {
    std::string input = "1 + 2 * 3";
    std::vector<TokenType> types;
    std::vector<std::string_view> values;

    // Order: src, types, values
    token::tokenizer(input, types, values);

    for (size_t i = 0; i < values.size(); i++) {
        std::cout << "Type: " << (int)types[i]
                  << ", Value: " << values[i] << "\n";
    }
    return 0;
}
```

## Usage

### 1. Lexer

```cpp
std::string src = R"(let x = 3.14)";
std::vector<TokenType> types;
std::vector<std::string_view> values;
token::tokenizer(src, types, values);
```

**Note**: the source string must outlive the token stream. All `std::string_view` values point into it.

### 2. Dual-Stack Expression Parser

Derive from `Parser` and register rules for each token type:

```cpp
class MyParser : public Parser {
    void pushOpWithPrec(TokenType op) {
        bool right = (op == POW);
        while (!ops.empty() && ops.top() != LP) {
            int tp = prec(ops.top()), cp = prec(op);
            if (tp > cp || (tp == cp && !right)) apply();
            else break;
        }
        ops.push(op);
    }

    std::vector<Rule> rules = {
        {NUM, [this]() { pushVal(std::stod(std::string(cuVal))); eat(); }},
        {SUM, [this]() { pushOpWithPrec(SUM); eat(); }},
        {MUL, [this]() { pushOpWithPrec(MUL); eat(); }},
        // ...
    };

public:
    using Parser::Parser;
    Value run() {
        while (cuTy != FIN) regex(rules);
        while (!ops.empty()) apply();
        Value r = std::move(vals.top()); vals.pop();
        return r;
    }
};
```

Stack operations available inside `Rule::action`:

| Function | Purpose |
|----------|---------|
| `pushVal(v)` | push a value |
| `pushOp(op)` | push an operator |
| `apply()` | pop operator + two values, push result |
| `topVal()` / `popVal()` | peek / pop value stack |
| `topOp()` / `popOp()` | peek / pop operator stack |

### 3. AST

`AST` provides node factories, a variable environment, and a root holder. Users implement their own `eval`:

```cpp
class MyLang : public Parser {
public:
    Value eval(const ASTNode* node) {
        if (auto* n = std::get_if<Number>(&node->type)) return n->val;
        if (auto* s = std::get_if<Str>(&node->type))    return s->val;
        if (auto* v = std::get_if<Var>(&node->type))    return ast.getVar(v->name);
        if (auto* o = std::get_if<Op>(&node->type)) {
            Value l = eval(o->left.get());
            Value r = eval(o->right.get());
            return opTable.at(o->op)(l, r);
        }
        throw std::runtime_error("unknown node");
    }
};
```

Factory methods:

```cpp
ast.make_number(3.14);
ast.make_str("hello");
ast.make_var("x");
ast.make_op(SUM, left, right);
```

## Token Types

```
NUM, SUM, MIN, MUL, DEL, LP, RP, COM, POW, FIN,
PUN, STR, ENG, NE, GE, LE, EQ, GT, LT, ASS, SEM,
NOT, E_FLOAT, COL, SCO, LCB, RCB, PRE, SHLE, SHRI,
AND, OR, AT, BAND, BOR, UNK
```

## Performance

| Input | Time |
|-------|------|
| 70 lines smali | 0.014 s |
| 200 lines C++ | 0.011 s |

## Extending

Add new operators in two places:

1. Add to the `TokenType` enum.
2. Add to the `wordTable` array:

```cpp
{"&&", AND}
```

**Important**: longer symbols must come before shorter ones (e.g. `"::"` before `":"`).

## Requirements

- C++20 compiler (GCC 10+, Clang 10+, MSVC 19.30+)
- No external dependencies

## License

MIT

## Author

senyyds12345