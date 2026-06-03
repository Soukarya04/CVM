#pragma once
#include "token.h"
#include "ast.h"
#include <vector>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    Program parse();

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    // token helpers
    Token& cur();                   // current token
    Token& advance();                   // consume & return current
    bool check(TokenType t) const;
    bool match(TokenType t);                    // consume if matching
    Token& expect(TokenType t, const std::string& msg);

    // statement parsers
    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<Stmt> parseLetStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseForStmt();
    std::unique_ptr<Stmt> parseWhileStmt();
    std::unique_ptr<Stmt> parsePrintStmt();
    StmtList parseBlock();

    // expression parsers
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAddition();
    std::unique_ptr<Expr> parseMultiply();
    std::unique_ptr<Expr> parsePrimary();

    // function parsers
    std::unique_ptr<Stmt> parseFnDecl();
    std::unique_ptr<Stmt> parseReturnStmt();
    std::unique_ptr<Expr> parseCall(std::string name);  
    std::unique_ptr<Expr> parseLogical();  
};
