#ifndef Token_H

#define Token_H

#include <cstddef>
#include <functional>
#include <cstring>
#include <memory>
#include <stack>
#include <cmath>
#include <string>
#include <cctype>
#include <array>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using Value = std::variant<
    double,
    std::string,
    bool
>;

enum TokenType
{
    NUM,  // 数字(包括小数)
    SUM,  // +
    MIN,  // -
    MUL,  // * or ×
    DEL,    // / or ÷
    LP,    // (
    RP,    // )
    COM,  // ,
    POW,  // ^
    FIN, // end of file
    PUN,   // .
    STR,  // "a"
    ENG,   // hello or _hello or hello123
    NE,   // !=
    GE,   // >=
    LE,   // <=
    EQ,   // ==
    GT,   // >
    LT,    // <
    ASS,   // =
    SEM,    // ;
    NOT,    // !
    E_FLOAT,    // Error float(1.2.3.4)
    COL,  // :
    SCO,  // ::
    LCB,   // {
    RCB,   // }
    PRE,  // #
    SHLE,   // <<
    SHRI,   // >>
    AND,   // &&
    OR,   // ||
    AT,   // @
    BAND,  // &
    BOR,   // |
    UNK   // unknown
};

enum StatusCode
{
    MATCH_STATUS_FAILED,
    MATCH_STATUS_OK
};

namespace token {
    template <typename T1, typename T2>
    void tokenizer(const std::string& src, T1& type, T2& value) noexcept;
}

struct OpEntry
{
    std::string_view sym;
    TokenType type;
};

// Token structure
struct Token 
{
    TokenType type;
    std::string_view value;
};

// Parser::regex Rule
struct Rule
{
    TokenType type;
    std::function<void()> action;
};

// AST Node
struct ASTNode;

struct Number {
    double val;
    Number() : val(0) {}
    Number(double v) : val(v) {}
};

struct Op {
    TokenType op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    Op(TokenType o, std::unique_ptr<ASTNode> le, std::unique_ptr<ASTNode> ri)
        : op(o), left(std::move(le)), right(std::move(ri)) {}
};

struct Str {
    std::string val;
    Str(std::string v)
        : val(std::move(v)) {}
};

struct Var {
    std::string name;
    Var(std::string n)
        : name(std::move(n)) {}
};

struct ASTNode {
    std::variant<Number, Op, Str, Var> type;
};

class AST {
private:
    std::unordered_map<std::string, Value> env;
public:
    std::unique_ptr<ASTNode> root;

    void setRoot(std::unique_ptr<ASTNode> r) {
        root = std::move(r);
    }

    const ASTNode* getRoot() {
        return root.get();
    }

    Value& getVar(const std::string& name, const std::string& e_msg = "unknown variable: ") {
        auto it = env.find(name);
        if (it == env.end())
            throw std::runtime_error(e_msg + name);
        return it->second;
    }

    void setVar(std::string name, Value val) {
        if (!name.empty())
            env[std::move(name)] = std::move(val);
    }

    std::unique_ptr<ASTNode> make_number(double val) {
        auto node = std::make_unique<ASTNode>();
        node->type = Number(val);
        return node;
    }

    std::unique_ptr<ASTNode> make_op(TokenType op, std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right) {
        auto node = std::make_unique<ASTNode>();
        node->type = Op(op, std::move(left), std::move(right));
        return node;
    }

    std::unique_ptr<ASTNode> make_str(std::string s) {
        auto node = std::make_unique<ASTNode>();
        node->type = Str(std::move(s));
        return node;
    }

    std::unique_ptr<ASTNode> make_var(std::string name) {
        auto node = std::make_unique<ASTNode>();
        node->type = Var(std::move(name));
        return node;
    }

};

class Lexer
{
private:
    size_t pos;
    Token token;
    const std::string& src;
    static constexpr inline const std::array<OpEntry, 31> wordTable = {{
        {">=", GE},
        {"<=", LE},
        {"!=", NE},
        {"==", EQ},
        {"::", SCO},
        {"<<", SHLE},
        {">>", SHRI},
        {"&&", AND},
        {"||", OR},
        {"+", SUM},
        {"*", MUL},
        {"×", MUL},
        {"÷", DEL},
        {"|", BOR},
        {"^", POW},
        {"&", BAND},
        {"/", DEL},
        {"(", LP},
        {")", RP},
        {",", COM},
        {".", PUN},
        {"<", LT},
        {"#", PRE},
        {">", GT},
        {"=", ASS},
        {";", SEM},
        {":", COL},
        {"@", AT},
        {"{", LCB},
        {"}", RCB},
        {"!", NOT}
    }};
    static constexpr inline const std::array<char, 8> negCtx = {{
        '+', '-', '*', '/', '^',
        '(', ',', '='
    }};
public:
    Lexer(const std::string& src) : pos(0), src(src)
    {
        token.value = {};
        token.type = UNK;
    }
    void parserNumber(int& count)
    {
        while (pos < src.size() && (isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) {
            if (src[pos] == '.')
            {
                count++;
            }
            if (count >= 2)
            {
                token.type = E_FLOAT;
            }
            pos++;
        }
    }
    
    Token* nextToken()
    {
        // 这段逻辑从别的Lexer偷的(
        token.type = UNK;
        token.value = {};
        while (pos < src.size() && isspace(static_cast<unsigned char>(src[pos]))) pos++;
        
        if (pos >= src.size())
        {
            token.type = FIN;
            return &token;
        }
        
        if (isdigit(static_cast<unsigned char>(src[pos])))
        {
            token.type = NUM;
            size_t start = pos;
            int count = 0;
            parserNumber(count);
            token.value = std::string_view(src.data() + start, pos - start);
            return &token;
        }
        
        if (pos < src.size() && (isalpha(static_cast<unsigned char>(src[pos])) || src[pos] == '_'))
        {
            token.type = ENG;
            size_t start = pos;
            while (isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_')
            {
                pos++;
            }
            token.value = std::string_view(src.data() + start, pos - start);
            return &token;
        }
        if (src[pos] == '"')
        {
            token.type = STR;
            size_t start = pos;
            pos++;
            while (pos < src.size() && src[pos] != '"')
            {
                if (src[pos] == '\\' && pos + 1 < src.size())
                {
                    pos += 2;
                } else {
                    pos++;
                }
            }
            
            if (pos >= src.size())
            {
                token.type = UNK;
                token.value = std::string_view(src.data() + start, pos - start);
                return &token;
            }
            
            token.value = std::string_view(src.data() + start, pos - start + 1);
            pos++;
            return &token;
        }
        
        if (pos < src.size() && src[pos] == '-')
        {
            bool hasDigit = isdigit(static_cast<unsigned char>(src[pos + 1]));

            if (!hasDigit) {
                token.type = MIN;
                token.value = "-";
                pos++;
                return &token;
            }

            bool isNegCtx = (pos == 0);
            if (!isNegCtx) {
                size_t prev = pos - 1;
                while (prev > 0 && isspace(static_cast<unsigned char>(src[prev])))
                    prev--;
                for (const char& ctx : negCtx) {
                    if (src[prev] == ctx) { isNegCtx = true; break; }
                 }
            }

            if (isNegCtx) {
                token.type = NUM;
                size_t start = pos;
                int count = 0;
                pos++;
                parserNumber(count);
                token.value = std::string_view(src.data() + start, pos - start);
                return &token;
            }

            token.type = MIN;
            token.value = "-";
            pos++;
            return &token;
        }
        const char* data = src.data();
        size_t len = src.size();
        for (const auto& entry : this->wordTable) {
            const std::string_view& symT = entry.sym;
            size_t symLen = symT.size();
            if (pos + symLen <= len && memcmp(data + pos, symT.data(), symLen) == 0) {
                token.type = entry.type;
                token.value = symT;
                pos += symLen;
                return &token;
            }
        }
        token.value = std::string_view(&src[pos], 1);
        pos++;
        return &token;
    }
};

// 提供给用户的基类 不是拿来直接用的
class Parser {
private:
    const std::string& src;
    size_t pos;
protected:
    std::stack<TokenType> ops;
    std::stack<Value> vals;
    
    static inline const std::unordered_map<TokenType, std::function<Value(const Value&, const Value&)>> opTable = {
        {SUM, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x + *y;
            if (auto* x = std::get_if<std::string>(&a))
                if (auto* y = std::get_if<std::string>(&b))
                    return *x + *y;
            throw std::runtime_error("+ 类型不匹配");
        }},
        {MIN, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x - *y;
            throw std::runtime_error("- 类型不匹配");
        }},
        {MUL, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x * *y;
            throw std::runtime_error("* 类型不匹配");
        }},
        {DEL, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x / *y;
            throw std::runtime_error("/ 类型不匹配");
        }},
        {POW, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return std::pow(*x, *y);
            throw std::runtime_error("^ 类型不匹配");
        }},
        {AND, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<bool>(&a))
                if (auto* y = std::get_if<bool>(&b))
                    return *x && *y;
            throw std::runtime_error("&& 类型不匹配");
        }},
        {OR, [](const Value& a, const Value &b) -> Value {
            if (auto* x = std::get_if<bool>(&a))
                if (auto* y = std::get_if<bool>(&b))
                    return *x || *y;
            throw std::runtime_error("|| 类型不匹配");
        }},
        {EQ, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x == *y;
            throw std::runtime_error("== 类型不匹配");
        }},
        {NE, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x != *y;
            throw std::runtime_error("!= 类型不匹配");
        }},
        {GE, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x >= *y;
            throw std::runtime_error(">= 类型不匹配");
        }},
        {LE, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x <= *y;
            throw std::runtime_error("<= 类型不匹配");
        }},
        {GT, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x > *y;
            throw std::runtime_error("> 类型不匹配");
        }},
        {LT, [](const Value& a, const Value& b) -> Value {
            if (auto* x = std::get_if<double>(&a))
                if (auto* y = std::get_if<double>(&b))
                    return *x < *y;
            throw std::runtime_error("< 类型不匹配");
        }}
    };
public:
    TokenType cuTy;
    std::string_view cuVal;
    std::vector<TokenType> types;
    std::vector<std::string_view> values;
    AST ast;

    Parser(const std::string& src) : src(src), pos(0)
    {
        token::tokenizer(src, types, values);
        cuTy = types[pos];
        cuVal = values[pos];
    }

    void eat() {
        pos++;
        if (pos >= types.size()) {
            cuTy = FIN;
            cuVal = "";
            return;
        }
        cuTy = types[pos];
        cuVal = values[pos];
    }

    StatusCode is_match(size_t n, TokenType type, std::string_view value = "") {
        if (n >= types.size()) return MATCH_STATUS_FAILED;
        if (types[n] != type) return MATCH_STATUS_FAILED;
        if (!value.empty() && values[n] != value) return MATCH_STATUS_FAILED;
        return MATCH_STATUS_OK;
    }
    
    void expect(StatusCode match_return, const std::string& msg)
    {
        if (match_return == MATCH_STATUS_FAILED)
        {
            throw std::runtime_error(msg.empty() ? "语法错误" : msg);
        }
    }
    
    TokenType peekPrev()
    {
        if (pos >= 1)
        {
            return types[pos - 1];
        } else {
            return UNK;
        }
    }

    TokenType getType(size_t n)
    {
        if (n >= types.size()) return UNK;
        return types[n];
    }

    std::string_view getVal(size_t n)
    {
        if (n >= types.size()) return "";
        return values[n];
    }

    size_t getPos() const
    {
        return pos;
    }

    template <TokenType type>
    bool peek() const
    {
        if (pos + 1 >= types.size()) return false;
        return types[pos + 1] == type;
    }

    template <typename arr_t>
    void regex(const arr_t& action_t)
    {
        for (const auto& table : action_t) {
            if (pos < types.size() && cuTy == table.type) {
                table.action();
            }
        }
    }
    // 栈操作函数 请在Rule结构体的action函数使用
    void apply() {
        if (vals.size() < 2 || ops.empty()) return;
        TokenType op = ops.top(); ops.pop();
        Value b = std::move(vals.top()); vals.pop();
        Value a = std::move(vals.top()); vals.pop();
        vals.push(opTable.at(op)(a, b));
    }

    void pushVal(Value v) {
        vals.push(std::move(v));
    }

    void pushOp(TokenType ty) {
        ops.push(ty);
    }

    const Value& topVal() const {
        return vals.top();
    }

    TokenType topOp() const {
        return ops.top();
    }

    void popVal() {
        vals.pop();
    }

    void popOp() {
        ops.pop();
    }
    
    static int prec(TokenType t) {
        switch (t) {
            case OR: return 1;
            case AND: return 2;
            case EQ:
            case NE: return 3;
            case LT:
            case GT:
            case LE:
            case GE: return 4;
            case SUM:
            case MIN: return 5;
            case MUL:
            case DEL: return 6;
            case POW: return 7;
            default:  return -1;
        }
    }
};

namespace token
{
    template <typename T1, typename T2>
    void tokenizer(const std::string& src, T1& types, T2& values) noexcept
    {
        Lexer lex(src);
        Token* current;
        while (1)
        {
            current = lex.nextToken();
            types.push_back(current->type);
            values.push_back(current->value);
            if (current->type == FIN) break;
        }
    }
}
#endif // ! Token_H
