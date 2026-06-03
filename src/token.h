#pragma once
#include <string>

enum class TokenType {
    // literals
    INT_LIT,                // 42
    STRING_LIT,             // "cat"
    TRUE_KW, FALSE_KW,      // true false

    // identifiers and keywords
    IDENT,                  // variable names
    LET, IF, ELSE,
    FOR, WHILE, PRINT, INPUT, SINPUT,
    FN,
    RETURN,

    // operators
    PLUS, MINUS,            // +  -
    STAR, SLASH,            // *  /
    PERCENT,                // %
    ASSIGN,                 // =
    EQ_EQ,                  // ==
    BANG_EQ,                // !=
    BANG,                   // !
    LESS,                   // <
    GREATER,                // >
    LESS_EQ,                // <=
    GREATER_EQ,             // >=
    AND,                    // &&
    OR,                     // ||

    // delimiters
    LPAREN, RPAREN,         // (  )
    LBRACE, RBRACE,         // {  }
    SEMICOLON,              // ;
    COMMA,                  // ,

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
