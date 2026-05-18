#pragma once
#include "token.h"
#include <vector>
#include <string>

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos  = 0;
    int line = 1;
    char peek(int offset = 0) const;
    char advance();
    void skipWhitespaceAndComments();
    Token readNumber();
    Token readIdentOrKeyword();
};
