#include "compiler.h"
#include <stdexcept>
using namespace std;

// variable assignment

uint8_t Compiler::declareVar(const string& name) {
    if (varMap.count(name))
        return varMap[name];                          // re-use existing slot
    if (nextVar == 255)
        throw runtime_error("Too many variables (max 255)");
    varMap[name] = nextVar;
    return nextVar++;
}

uint8_t Compiler::lookupVar(const string& name) {
    auto it = varMap.find(name);
    if (it == varMap.end())
        throw runtime_error("Undefined variable: '" + name + "'");
    return it->second;
}

// entry point

Chunk Compiler::compile(const Program& prog) {
    compileStmts(prog.stmts);
    chunk.emit(OpCode::HALT);
    return move(chunk);
}

// statement compilation

void Compiler::compileStmts(const StmtList& stmts) {
    for (const auto& s : stmts)
        compileStmt(*s);
}

void Compiler::compileStmt(const Stmt& stmt) {

    if (const auto* s = dynamic_cast<const LetStmt*>(&stmt)) {
        compileExpr(*s->init);
        uint8_t idx = declareVar(s->name);
        chunk.emit(OpCode::STORE);
        chunk.emit(idx);
        return;
    }

    if (const auto* s = dynamic_cast<const AssignStmt*>(&stmt)) {
        compileExpr(*s->value);
        uint8_t idx = lookupVar(s->name);
        chunk.emit(OpCode::STORE);
        chunk.emit(idx);
        return;
    }

    if (const auto* s = dynamic_cast<const PrintStmt*>(&stmt)) {
        compileExpr(*s->expr);
        chunk.emit(OpCode::PRINT);
        return;
    }

    if (const auto* s = dynamic_cast<const IfStmt*>(&stmt)) {
        compileExpr(*s->cond);

        chunk.emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_false = chunk.here() - 2;   

        compileStmts(s->then_block);

        if (!s->else_block.empty()) {
            chunk.emitJump(OpCode::JUMP);
            size_t patch_end = chunk.here() - 2;

            chunk.patchJump(patch_false, chunk.here());  
            compileStmts(s->else_block);
            chunk.patchJump(patch_end,   chunk.here());  
        } else {
            chunk.patchJump(patch_false, chunk.here());  
        }
        return;
    }

    if (const auto* s = dynamic_cast<const WhileStmt*>(&stmt)) {
        size_t loop_start = chunk.here();

        compileExpr(*s->cond);

        chunk.emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_exit = chunk.here() - 2;

        compileStmts(s->body);

        chunk.emitJump(OpCode::JUMP);
        chunk.patchJump(chunk.here() - 2, loop_start);  // loop back

        chunk.patchJump(patch_exit, chunk.here());       // exit jump
        return;
    }

    throw runtime_error("Compiler: unknown statement type");
}

// expression compilation

void Compiler::compileExpr(const Expr& expr) {

    if (const auto* e = dynamic_cast<const IntLitExpr*>(&expr)) {
        chunk.emit(OpCode::PUSH_INT);
        chunk.emitInt32(e->value);
        return;
    }

    if (const auto* e = dynamic_cast<const BoolLitExpr*>(&expr)) {
        chunk.emit(e->value ? OpCode::PUSH_TRUE : OpCode::PUSH_FALSE);
        return;
    }

    if (const auto* e = dynamic_cast<const VarExpr*>(&expr)) {
        chunk.emit(OpCode::LOAD);
        chunk.emit(lookupVar(e->name));
        return;
    }

    if (dynamic_cast<const InputExpr*>(&expr)) {
        chunk.emit(OpCode::INPUT);
        return;
    }

    if (const auto* e = dynamic_cast<const BinaryExpr*>(&expr)) {
        compileExpr(*e->left);
        compileExpr(*e->right);
        switch (e->op) {
            case '+': chunk.emit(OpCode::ADD);  break;
            case '-': chunk.emit(OpCode::SUB);  break;
            case '*': chunk.emit(OpCode::MUL);  break;
            case '/': chunk.emit(OpCode::DIV);  break;
            case '=': chunk.emit(OpCode::EQ);   break;  // '=' represents '=='
            case '<': chunk.emit(OpCode::LESS); break;
            default:
                throw runtime_error(
                    string("Compiler: unknown binary op '") + e->op + "'");
        }
        return;
    }

    throw runtime_error("Compiler: unknown expression type");
}
