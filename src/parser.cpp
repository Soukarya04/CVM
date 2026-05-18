#include "parser.h"
#include <stdexcept>
#include <string>
using namespace std;

Parser::Parser(vector<Token> toks) : tokens(move(toks)) {}

// token helpers
Token& Parser::cur() {
    return tokens[pos];
}

Token& Parser::advance() {
    Token& t = tokens[pos];
    if (pos < tokens.size() - 1) ++pos;
    return t;
}

bool Parser::check(TokenType t) const {
    return tokens[pos].type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

Token& Parser::expect(TokenType t, const string& msg) {
    if (!check(t))
        throw runtime_error(
            "Parse error at line " + to_string(cur().line) +
            ": " + msg + " (got '" + cur().value + "')");
    return advance();
}

// top-level

Program Parser::parse() {
    Program prog;
    while (!check(TokenType::EOF_T))
        prog.stmts.push_back(parseStmt());
    return prog;
}

// ── Statements ─────────────────────────────────────────────────────────────

unique_ptr<Stmt> Parser::parseStmt() {
    if (check(TokenType::LET))   return parseLetStmt();
    if (check(TokenType::IF))    return parseIfStmt();
    if (check(TokenType::WHILE)) return parseWhileStmt();
    if (check(TokenType::PRINT)) return parsePrintStmt();

    // Assignment:  IDENT "=" expr ";"
    if (check(TokenType::IDENT) &&
        pos + 1 < tokens.size() &&
        tokens[pos + 1].type == TokenType::ASSIGN)
    {
        string name = advance().value;   // consume IDENT
        advance();                             // consume "="
        auto val = parseExpr();
        expect(TokenType::SEMICOLON, "Expected ';' after assignment");
        return make_unique<AssignStmt>(move(name), move(val));
    }

    throw runtime_error(
        "Parse error at line " + to_string(cur().line) +
        ": unexpected token '" + cur().value + "'");
}

unique_ptr<Stmt> Parser::parseLetStmt() {
    advance();  // consume "let"
    string name = expect(TokenType::IDENT, "Expected variable name after 'let'").value;
    expect(TokenType::ASSIGN, "Expected '=' after variable name");
    auto init = parseExpr();
    expect(TokenType::SEMICOLON, "Expected ';' after let statement");
    return make_unique<LetStmt>(move(name), move(init));
}

unique_ptr<Stmt> Parser::parseIfStmt() {
    advance();  // consume "if"
    expect(TokenType::LPAREN, "Expected '(' after 'if'");
    auto cond = parseExpr();
    expect(TokenType::RPAREN, "Expected ')' after if condition");
    StmtList then_block = parseBlock();
    StmtList else_block;
    if (check(TokenType::ELSE)) {
        advance();  // consume "else"
        else_block = parseBlock();
    }
    return make_unique<IfStmt>(move(cond), move(then_block), move(else_block));
}

unique_ptr<Stmt> Parser::parseWhileStmt() {
    advance();  // consume "while"
    expect(TokenType::LPAREN, "Expected '(' after 'while'");
    auto cond = parseExpr();
    expect(TokenType::RPAREN, "Expected ')' after while condition");
    StmtList body = parseBlock();
    return make_unique<WhileStmt>(move(cond), move(body));
}

unique_ptr<Stmt> Parser::parsePrintStmt() {
    advance();  // consume "print"
    auto expr = parseExpr();
    expect(TokenType::SEMICOLON, "Expected ';' after print");
    return make_unique<PrintStmt>(move(expr));
}

StmtList Parser::parseBlock() {
    expect(TokenType::LBRACE, "Expected '{'");
    StmtList stmts;
    while (!check(TokenType::RBRACE) && !check(TokenType::EOF_T))
        stmts.push_back(parseStmt());
    expect(TokenType::RBRACE, "Expected '}'");
    return stmts;
}

// ── Expressions ────────────────────────────────────────────────────────────

unique_ptr<Expr> Parser::parseExpr() {
    return parseComparison();
}

// comparison → addition ( ("==" | "<") addition )*
unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddition();
    while (check(TokenType::EQ_EQ) || check(TokenType::LESS)) {
        TokenType opType = advance().type;          // consume operator
        char op = (opType == TokenType::EQ_EQ) ? '=' : '<';
        auto right = parseAddition();
        left = make_unique<BinaryExpr>(op, move(left), move(right));
    }
    return left;
}

// addition → multiply ( ("+" | "-") multiply )*
unique_ptr<Expr> Parser::parseAddition() {
    auto left = parseMultiply();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        TokenType opType = advance().type;
        char op = (opType == TokenType::PLUS) ? '+' : '-';
        auto right = parseMultiply();
        left = make_unique<BinaryExpr>(op, move(left), move(right));
    }
    return left;
}

// multiply → primary ( ("*" | "/") primary )*
unique_ptr<Expr> Parser::parseMultiply() {
    auto left = parsePrimary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        TokenType opType = advance().type;
        char op = (opType == TokenType::STAR) ? '*' : '/';
        auto right = parsePrimary();
        left = make_unique<BinaryExpr>(op, move(left), move(right));
    }
    return left;
}

// primary → INT_LIT | "true" | "false" | "input" | IDENT | "(" expr ")"
unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::INT_LIT)) {
        int32_t val = stoi(advance().value);
        return make_unique<IntLitExpr>(val);
    }
    if (check(TokenType::TRUE_KW))  { advance(); return make_unique<BoolLitExpr>(true);  }
    if (check(TokenType::FALSE_KW)) { advance(); return make_unique<BoolLitExpr>(false); }
    if (check(TokenType::INPUT))    { advance(); return make_unique<InputExpr>();         }
    if (check(TokenType::IDENT)) {
        string name = advance().value;
        return make_unique<VarExpr>(move(name));
    }
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpr();
        expect(TokenType::RPAREN, "Expected ')' after grouped expression");
        return expr;
    }
    throw runtime_error(
        "Parse error at line " + to_string(cur().line) +
        ": unexpected token '" + cur().value + "' in expression");
}
