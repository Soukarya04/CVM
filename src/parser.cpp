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
    while (!check(TokenType::EOF_T)) {
        if (check(TokenType::FN))
            prog.functions.push_back(
                unique_ptr<FnDecl>(static_cast<FnDecl*>(parseFnDecl().release())));
        else
            prog.stmts.push_back(parseStmt());
    }
    return prog;
}

// statements

unique_ptr<Stmt> Parser::parseStmt() {
    if (check(TokenType::LET))   return parseLetStmt();
    if (check(TokenType::IF))    return parseIfStmt();
    if (check(TokenType::FOR)) return parseForStmt();
    if (check(TokenType::WHILE)) return parseWhileStmt();
    if (check(TokenType::PRINT)) return parsePrintStmt();
    if (check(TokenType::RETURN)) return parseReturnStmt();

    // assignment:  IDENT "=" expr ";"
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
        if (check(TokenType::IF)) {
            // else if 
            else_block.push_back(parseIfStmt());
        } else {
            // normal else
            else_block = parseBlock();
        }
    }
    return make_unique<IfStmt>(move(cond), move(then_block), move(else_block));
}

unique_ptr<Stmt> Parser::parseForStmt() {
    advance();  // consume 'for'
    expect(TokenType::LPAREN, "Expected '(' after 'for'");

    // initialization
    unique_ptr<Stmt> init;
    if (check(TokenType::LET))
        init = parseLetStmt();
    else {
        string name = advance().value;   
        advance();                       // consume '='
        auto val = parseExpr();
        expect(TokenType::SEMICOLON, "Expected ';' after for init");
        init = make_unique<AssignStmt>(move(name), move(val));
    }

    // condition
    auto cond = parseExpr();
    expect(TokenType::SEMICOLON, "Expected ';' after for condition");

    // update
    string uname = advance().value;   
    advance();                        // consume '='
    auto uval = parseExpr();
    expect(TokenType::RPAREN, "Expected ')' after for update");

    auto update = make_unique<AssignStmt>(move(uname), move(uval));

    StmtList body = parseBlock();
    return make_unique<ForStmt>(move(init), move(cond), move(update), move(body));
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

// expressions

unique_ptr<Expr> Parser::parseExpr() {
    return parseLogical();
}

// comparison -> addition ( ("==" | "<") addition )*
unique_ptr<Expr> Parser::parseComparison() {
    auto left = parseAddition();
        while (check(TokenType::EQ_EQ)      || check(TokenType::LESS) ||
        check(TokenType::GREATER)     || check(TokenType::LESS_EQ) ||
        check(TokenType::GREATER_EQ) || check(TokenType::BANG_EQ))
    {
        TokenType opType = advance().type;
        char op;
        if      (opType == TokenType::EQ_EQ)      op = '=';
        else if (opType == TokenType::BANG_EQ)     op = '!';
        else if (opType == TokenType::LESS)        op = '<';
        else if (opType == TokenType::GREATER)     op = '>';
        else if (opType == TokenType::LESS_EQ)     op = 'L';  // L = <=
        else                                       op = 'G';  // G = >=
        auto right = parseAddition();
        left = make_unique<BinaryExpr>(op, move(left), move(right));
    }
    return left;
}

// addition -> multiply ( ("+" | "-") multiply )*
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

// multiply -> primary ( ("*" | "/" | "%") primary )*
unique_ptr<Expr> Parser::parseMultiply() {
    auto left = parsePrimary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT)) {
        TokenType opType = advance().type;
        char op = (opType == TokenType::STAR) ? '*' 
                : (opType == TokenType::SLASH) ? '/' 
                : '%';
        auto right = parsePrimary();
        left = make_unique<BinaryExpr>(op, move(left), move(right));
    }
    return left;
}
unique_ptr<Expr> Parser::parseLogical() {
    auto left = parseComparison();
    while (check(TokenType::AND) || check(TokenType::OR)) {
        TokenType opType = advance().type;
        auto right = parseComparison();
        if (opType == TokenType::AND)
            left = make_unique<AndExpr>(move(left), move(right));
        else
            left = make_unique<OrExpr>(move(left), move(right));
    }
    return left;
}

// func declaration
unique_ptr<Stmt> Parser::parseFnDecl() {
    advance();  // consume 'fn'
    string name = expect(TokenType::IDENT, "Expected function name").value;
    expect(TokenType::LPAREN, "Expected '(' after function name");

    // parse parameters
    vector<string> params;
    if (!check(TokenType::RPAREN)) {
        do {
            params.push_back(
                expect(TokenType::IDENT, "Expected parameter name").value);
        } while (match(TokenType::COMMA));  // ← needs COMMA token (see below)
    }
    expect(TokenType::RPAREN, "Expected ')' after parameters");

    StmtList body = parseBlock();
    return make_unique<FnDecl>(move(name), move(params), move(body));
}

unique_ptr<Expr> Parser::parseCall(string name) {
    advance();  // consume '('
    vector<unique_ptr<Expr>> args;
    if (!check(TokenType::RPAREN)) {
        do {
            args.push_back(parseExpr());
        } while (match(TokenType::COMMA));
    }
    expect(TokenType::RPAREN, "Expected ')' after arguments");
    return make_unique<CallExpr>(move(name), move(args));
}

// func return statement
unique_ptr<Stmt> Parser::parseReturnStmt() {
    advance();  // consume 'return'
    unique_ptr<Expr> val;
    if (!check(TokenType::SEMICOLON))
        val = parseExpr();
    expect(TokenType::SEMICOLON, "Expected ';' after return");
    return make_unique<ReturnStmt>(move(val));
}

// primary -> INT_LIT | "true" | "false" | "input" | IDENT | "(" expr ")"
unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::BANG)) {
        advance();
        auto operand = parsePrimary();
        return make_unique<UnaryExpr>('!', move(operand));
    }
    if (check(TokenType::MINUS)) {
        advance();  
        auto operand = parsePrimary();
        return make_unique<UnaryExpr>('-', move(operand));
    }
    if (check(TokenType::INT_LIT)) {
        int32_t val = stoi(advance().value);
        return make_unique<IntLitExpr>(val);
    }
    if (check(TokenType::STRING_LIT)) {
        string val = advance().value;
        return make_unique<StringLitExpr>(move(val));
    }
    if (check(TokenType::TRUE_KW))  { advance(); return make_unique<BoolLitExpr>(true);  }
    if (check(TokenType::FALSE_KW)) { advance(); return make_unique<BoolLitExpr>(false); }
    if (check(TokenType::INPUT))    { advance(); return make_unique<InputExpr>(); }
    if (check(TokenType::SINPUT)) { advance(); return make_unique<SInputExpr>(); }
    if (check(TokenType::IDENT)) {
        string name = advance().value;
        if (check(TokenType::LPAREN))          
            return parseCall(move(name));
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
