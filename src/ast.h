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

// expression nodes :

struct IntLitExpr : Expr {
    int32_t value;
    explicit IntLitExpr(int32_t v) : value(v) {}
};

struct BoolLitExpr : Expr {
    bool value;
    explicit BoolLitExpr(bool v) : value(v) {}
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

// read integer
struct InputExpr : Expr {};

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

// while loop
struct WhileStmt : Stmt {
    std::unique_ptr<Expr> cond;
    StmtList body;
    WhileStmt(std::unique_ptr<Expr> c, StmtList b)
        : cond(std::move(c)), body(std::move(b)) {}
};

// program root
struct Program {
    StmtList stmts;
};
