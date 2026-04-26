#ifndef Token_H
#define Token_H

#include <string>
#include <cctype>
#include <array>
#include <string_view>
#include <variant>

enum TokenType {
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
    ENG,   // hello等英文句子
    NE,   // !=
    GTOET,   // >=
    LTOET,   // <=
    ET,   // ==
    GT,   // >
    LT,    // <
    ASS,   // =
    SEM,    // ;
    E_FLOAT,    //错误小数(如1.2.3.4)
    COL,  // :
    SCO,  // ::
    PRE,  // #
    SHLE,   // <<
    SHRI,   // >>
    AND,   // &
    UNKNOWN   // 未知
};

struct OpEntry {
    std::string_view sym;
    TokenType type;
};

struct Token {
    TokenType type;
    std::string value;
};

class Lexer {
private:
    size_t pos;
    Token token;
    std::string src;
    static constexpr inline const std::array<OpEntry, 23> wordTable = {{
        {">=", GTOET},
        {"<=", LTOET},
        {"!=", NE},
        {"::", SCO},
        {"==", ET},
        {"<<", SHLE},
        {">>", SHRI},
        {"×", MUL},
        {"÷", DEL},
        {"^", POW},
        {".", PUN},
        {"+", SUM},
        {"*", MUL},
        {"/", DEL},
        {"(", LP},
        {")", RP},
        {",", COM},
        {"<", LT},
        {"#", PRE},
        {">", GT},
        {"=", ASS},
        {";", SEM},
        {":", COL},
        {"&", AND}
    }};
    static constexpr inline const std::array<char, 8> negCtx = {{
        '+', '-', '*', '/', '^',
        '(', ',', '='
    }};
public:
    Lexer(const std::string& src) : src(src), pos(0) {
        token.value.clear();
        token.type = UNKNOWN;
    }
    void ParserNumber(int& count, size_t& start) {
        while (pos < src.size() && (isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) {
            if (src[pos] == '.') {
                count++;
            }
            if (count >= 2) {
                token.type = E_FLOAT;
            }
            pos++;
        }
    }
    
    Token* nextToken() {
        // 这段逻辑从别的Lexer偷的(
        token.type = UNKNOWN;
        token.value.clear();
        while (pos < src.size() && isspace(src[pos])) pos++;
        
        if (pos >= src.size()) {
            token.type = END;
            return &token;
        }
        
        if (isdigit(src[pos])) {
            token.type = NUM;
            size_t start = pos;
            int count = 0;
            ParserNumber(count, start);
            token.value = src.substr(start, pos - start);
            return &token;
        }
        
        if (pos < src.size() && isalpha(src[pos])) {
            token.type = ENG;
            size_t start = pos;
            while (isalpha(src[pos])) {
                pos++;
            }
            token.value = src.substr(start, pos - start);
            return &token;
        }
        if (pos < src.size() && src[pos] == '-') {
            if (pos == 0) {
                token.type = NEG;
                size_t start = pos;
                int count = 0;
                pos++;
                ParserNumber(count, start);
                token.value = src.substr(start, pos - start);
                return &token;
            } else {
                size_t prev = pos - 1;
                while (prev > 0 && isspace(src[prev])) {
                    prev--;
                }
                for (const char& ctx : negCtx) {
                    if (src[prev] == ctx) {
                        token.type = NEG;
                        size_t start = pos;
                        int count = 0;
                        pos++;
                        ParserNumber(count, start);
                        token.value = src.substr(start, pos - start);
                        return &token;
                    }
                }
            }
            token.type = MIN;
            token.value = '-';
            pos++;
            return &token;
        }
        for (const auto& entry : this->wordTable) {
            const std::string_view& symT = entry.sym;
            if (pos < src.size() && src.compare(pos, symT.size(), symT) == 0) {
                token.type = entry.type;
                token.value = symT;
                pos += symT.size();
                return &token;
            }
        }
        token.value = src[pos];
        pos++;
        return &token;
    }
};

namespace token {
    template <typename T1, typename T2>
    void tokenizer(const std::string& src, T1& tokenVa, T2& tokenTy) noexcept {
        Lexer lex(src);
        Token* current;
        while (1) {
            current = lex.nextToken();
            tokenVa.push_back(current->value);
            tokenTy.push_back(current->type);
            if (current->type == END) break;
        }
    }
}

#endif // ! Token_H