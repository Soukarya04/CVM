#pragma once
#include <memory>
#include <vector>
#include <string>
#include <cstdint>

// base classes
struct Expr {
    virtual ~Expr() = default;
};

struct Stmt {
    virtual ~Stmt() = default;
};

using StmtList = std::vector<std::unique_ptr<Stmt>>;


// expression nodes :-

struct IntLitExpr : Expr {
    int32_t value;
    explicit IntLitExpr(int32_t v) : value(v) {}
};

struct BoolLitExpr : Expr {
    bool value;
    explicit BoolLitExpr(bool v) : value(v) {}
};

struct StringLitExpr : Expr {
    std::string value;
    explicit StringLitExpr(std::string v) : value(std::move(v)) {}
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string n) : name(std::move(n)) {}
};

struct BinaryExpr : Expr {
    char op;
    std::unique_ptr<Expr> left, right;

    BinaryExpr(char o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

struct UnaryExpr : Expr {
    char op;  // for only '-'
    std::unique_ptr<Expr> operand;
    UnaryExpr(char o, std::unique_ptr<Expr> e)
        : op(o), operand(std::move(e)) {}
};

struct AndExpr : Expr {
    std::unique_ptr<Expr> left, right;
    AndExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : left(std::move(l)), right(std::move(r)) {}
};

struct OrExpr : Expr {
    std::unique_ptr<Expr> left, right;
    OrExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : left(std::move(l)), right(std::move(r)) {}
};


// read integer/string
struct InputExpr : Expr {};
struct SInputExpr : Expr {};

// statement nodes
struct LetStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> init;
    LetStmt(std::string n, std::unique_ptr<Expr> i)
        : name(std::move(n)), init(std::move(i)) {}
};

struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignStmt(std::string n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit PrintStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> cond;
    StmtList then_block;
    StmtList else_block;  // may be empty

    IfStmt(std::unique_ptr<Expr> c, StmtList t, StmtList e)
        : cond(std::move(c)), then_block(std::move(t)), else_block(std::move(e)) {}
};

// for loop 
struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;       
    std::unique_ptr<Expr> cond;       
    std::unique_ptr<Stmt> update;     
    StmtList body;

    ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> c,
            std::unique_ptr<Stmt> u, StmtList b)
        : init(std::move(i)), cond(std::move(c)),
          update(std::move(u)), body(std::move(b)) {}
};

// while loop
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> cond;
    StmtList body;
    WhileStmt(std::unique_ptr<Expr> c, StmtList b)
        : cond(std::move(c)), body(std::move(b)) {}
};

// function declaration
struct FnDecl : Stmt {
    std::string name;
    std::vector<std::string> params;
    StmtList body;

    FnDecl(std::string n, std::vector<std::string> p, StmtList b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
};

// function call
struct CallExpr : Expr {
    std::string name;
    std::vector<std::unique_ptr<Expr>> args;

    CallExpr(std::string n, std::vector<std::unique_ptr<Expr>> a)
        : name(std::move(n)), args(std::move(a)) {}
};

// return statement
struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;  // can be nullptr for bare return
    explicit ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
};

// program root
struct Program {
    std::vector<std::unique_ptr<FnDecl>> functions;
    StmtList stmts;
};
