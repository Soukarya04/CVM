#include "lexer.h"
#include <stdexcept>
#include <cctype>
#include <unordered_map>
using namespace std;

// keywords
static const unordered_map<string, TokenType> KEYWORDS = {
    {"let",  TokenType::LET},
    {"if",  TokenType::IF},
    {"else",  TokenType::ELSE},
    {"while",  TokenType::WHILE},
    {"print",  TokenType::PRINT},
    {"input",  TokenType::INPUT},
    {"true",  TokenType::TRUE_KW},
    {"false",  TokenType::FALSE_KW},
};

Lexer::Lexer(string source) : src(move(source)) {}


// helpers :

char Lexer::peek(int offset) const {
    size_t idx = pos + static_cast<size_t>(offset);
    return (idx < src.size()) ? src[idx] : '\0';
}

char Lexer::advance() {
    char c = src[pos++];
    if (c == '\n') ++line;
    return c;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < src.size()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else if (c == '/' && peek(1) == '/') {
            // single line comment - skip to end of line
            while (pos < src.size() && peek() != '\n') advance();
        } else {
            break;
        }
    }
}

Token Lexer::readNumber() {
    size_t start = pos;
    while (pos < src.size() && isdigit(static_cast<unsigned char>(peek())))
        advance();
    return Token(TokenType::INT_LIT, src.substr(start, pos - start), line);
}

Token Lexer::readIdentOrKeyword() {
    size_t start = pos;
    while (pos < src.size() &&
           (isalnum(static_cast<unsigned char>(peek())) || peek() == '_'))
        advance();

    string word = src.substr(start, pos - start);
    auto it = KEYWORDS.find(word);
    if (it != KEYWORDS.end())
        return Token(it->second, word, line);
    return Token(TokenType::IDENT, word, line);
}

// main lexer loop

vector<Token> Lexer::tokenize() {
    vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (pos >= src.size()) {
            tokens.emplace_back(TokenType::EOF_T, "", line);
            break;
        }

        char c = peek();

        // numbers
        if (isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(readNumber());
            continue;
        }

        // identifiers / keywords
        if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(readIdentOrKeyword());
            continue;
        }

        // single-character and two-character tokens
        advance();
        switch (c) {
            case '+': tokens.emplace_back(TokenType::PLUS,  "+", line); break;
            case '-': tokens.emplace_back(TokenType::MINUS,  "-", line); break;
            case '*': tokens.emplace_back(TokenType::STAR,  "*", line); break;
            case '/': tokens.emplace_back(TokenType::SLASH,  "/", line); break;
            case '<': tokens.emplace_back(TokenType::LESS,  "<", line); break;
            case '(': tokens.emplace_back(TokenType::LPAREN,  "(", line); break;
            case ')': tokens.emplace_back(TokenType::RPAREN,  ")", line); break;
            case '{': tokens.emplace_back(TokenType::LBRACE,  "{", line); break;
            case '}': tokens.emplace_back(TokenType::RBRACE,  "}", line); break;
            case ';': tokens.emplace_back(TokenType::SEMICOLON,  ";", line); break;
            case '=':
                if (peek() == '=') {
                    advance();
                    tokens.emplace_back(TokenType::EQ_EQ, "==", line);
                } else {
                    tokens.emplace_back(TokenType::ASSIGN, "=", line);
                }
                break;
            default:
                throw runtime_error(
                    "Lexer error at line " + to_string(line) +
                    ": unknown character '" + c + "'");
        }
    }

    return tokens;
}
