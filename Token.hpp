#ifndef Token_H
#define Token_H

#include <cstddef>
#include <functional>
#include <cstring>
#include <string>
#include <cctype>
#include <algorithm>
#include <array>
#include <string_view>
#include <any>
#include <stdexcept>
#include <system_error>
#include <vector>

enum TokenType
{
    NUM,  // 数字(包括小数)
    SUM,  // +
    MIN,  // -
    MUL,  // *或×
    DEL,    // /或÷
    LP,    // (
    RP,    // )
    COM,  // ,
    POW,  // ^
    END, // 啥也没有
    PUN,   // .
    NEG,    // -
    STR,  // "a"
    ENG,   // hello等英文句子
    NE,   // !=
    GTOET,   // >=
    LTOET,   // <=
    ET,   // ==
    GT,   // >
    LT,    // <
    ASS,   // =
    SEM,    // ;
    NOT,    // !
    E_FLOAT,    //错误小数(如1.2.3.4)
    COL,  // :
    SCO,  // ::
    LCB,   // {
    RCB,   // }
    PRE,  // #
    SHLE,   // <<
    SHRI,   // >>
    AND,   // &/&&
    OR,   // ||
    AT,   //@
    UNKNOWN   // 未知
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

struct Token 
{
    TokenType type;
    std::string_view value;
};

struct Rule
{
    TokenType type;
    std::function<void()> action;
};

class Lexer
{
private:
    size_t pos;
    Token token;
    const std::string& src;
    static constexpr inline const std::array<OpEntry, 30> wordTable = {{
        {">=", GTOET},
        {"<=", LTOET},
        {"!=", NE},
        {"==", ET},
        {"::", SCO},
        {"<<", SHLE},
        {">>", SHRI},
        {"&&", AND},
        {"||", OR},
        {"+", SUM},
        {"*", MUL},
        {"×", MUL},
        {"÷", DEL},
        {"^", POW},
        {"&", AND},
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
        token.type = UNKNOWN;
    }
    void ParserNumber(int& count)
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
        token.type = UNKNOWN;
        token.value = {};
        while (pos < src.size() && isspace(src[pos])) pos++;
        
        if (pos >= src.size())
        {
            token.type = END;
            return &token;
        }
        
        if (isdigit(src[pos]))
        {
            token.type = NUM;
            size_t start = pos;
            int count = 0;
            ParserNumber(count);
            token.value = std::string_view(src.data() + start, pos - start);
            return &token;
        }
        
        if (pos < src.size() && isalpha(src[pos]))
        {
            token.type = ENG;
            size_t start = pos;
            while (isalpha(src[pos]) || src[pos] == '_')
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
            while (pos < src.size() && src[pos] != '"') pos++;
            token.value = std::string_view(src.data() + start, pos - start + 1);
            pos++;
            return &token;
        }
        
        if (pos < src.size() && src[pos] == '-')
        {
            if (pos == 0)
            {
                token.type = NEG;
                size_t start = pos;
                int count = 0;
                pos++;
                ParserNumber(count);
                token.value = std::string_view(src.data() + start, pos - start);
                return &token;
            } else {
                size_t prev = pos - 1;
                while (prev > 0 && isspace(src[prev]))
                {
                    prev--;
                }
                for (const char& ctx : negCtx)
                {
                    if (src[prev] == ctx)
                    {
                        token.type = NEG;
                        size_t start = pos;
                        int count = 0;
                        pos++;
                        ParserNumber(count);
                        token.value = std::string_view(src.data() + start, pos - start);
                        return &token;
                    }
                }
            }
            token.type = MIN;
            token.value = std::string_view(&src[pos], 1);
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
    std::string src;
    int pos;
public:
    TokenType cuTy;
    std::string_view cuVal;
    std::vector<TokenType> types;
    std::vector<std::string_view> values;

    Parser(const std::string& src) : src(src), pos(0)
    {
        token::tokenizer(src, types, values);
        cuTy = types[pos];
        cuVal = values[pos];
    }

    void eat() {
        pos++;
        if (pos >= types.size()) {
            cuTy = END;
            cuVal = "";
            return;
        }
        cuTy = types[pos];
        cuVal = values[pos];
    }

    StatusCode is_match(TokenType type, std::string_view value = "", int n = -1) {
        if (n < 0) n = pos;
        if (n < 0 || n >= (int)types.size()) return MATCH_STATUS_FAILED;
        if (types[n] != type) return MATCH_STATUS_FAILED;
        if (!value.empty() && values[n] != value) return MATCH_STATUS_FAILED;
        return MATCH_STATUS_OK;
    }
    
    void except(StatusCode match_return, const std::string& msg)
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
            return UNKNOWN;
        }
    }

    TokenType getType(int n)
    {
        if (n < 0 || n >= (int)types.size()) return UNKNOWN;
        return types[n];
    }

    std::string_view getVal(int n)
    {
        if (n < 0 || n >= (int)types.size()) return "";
        return values[n];
    }

    int getPos()
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
            if (current->type == END) break;
        }
    }
}
#endif // ! Token_H
