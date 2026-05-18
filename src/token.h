#pragma once
#include <string>

enum class TokenType {
    // literals
    INT_LIT,                // 42
    TRUE_KW, FALSE_KW,      // true false

    // identifiers and keywords
    IDENT,                  // variable names
    LET, IF, ELSE,
    WHILE, PRINT, INPUT,

    // operators
    PLUS, MINUS,            // +  -
    STAR, SLASH,            // *  /
    ASSIGN,                 // =
    EQ_EQ,                  // ==
    LESS,                   // <

    // delimiters
    LPAREN, RPAREN,         // (  )
    LBRACE, RBRACE,         // {  }
    SEMICOLON,              // ;

    // end of file
    EOF_T
};

struct Token {
    TokenType   type;
    std::string value;  
    int         line;

    Token(TokenType t, std::string v, int l)
        : type(t), value(std::move(v)), line(l) {}
};
