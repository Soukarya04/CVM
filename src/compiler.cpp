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
    for (const auto& fn : prog.functions)
        compileFn(*fn);
    
    compileStmts(prog.stmts);
    chunk.emit(OpCode::HALT);
    return move(chunk);
}

// function compilation :-

void Compiler::compileFn(const FnDecl& fn) {
    FunctionObject fnObj;
    fnObj.name  = fn.name;
    fnObj.arity = fn.params.size();

    currentFn = &functions[fn.name];
    *currentFn = move(fnObj);

    auto savedVarMap  = varMap;
    auto savedNextVar = nextVar;
    varMap.clear();
    nextVar = 0;

    for (const auto& param : fn.params)
        declareVar(param);

    compileStmts(fn.body);

    current().emit(OpCode::PUSH_INT);
    current().emitInt32(0);
    current().emit(OpCode::RETURN);

    currentFn = nullptr;
    varMap    = savedVarMap;
    nextVar   = savedNextVar;
}

// statement compilation :-

void Compiler::compileStmts(const StmtList& stmts) {
    for (const auto& s : stmts)
        compileStmt(*s);
}

void Compiler::compileStmt(const Stmt& stmt) {

    if (const auto* s = dynamic_cast<const ReturnStmt*>(&stmt)) {
        if (s->value)
            compileExpr(*s->value);
        else {
            current().emit(OpCode::PUSH_INT);
            current().emitInt32(0);
        }
        current().emit(OpCode::RETURN);
        return;
    }

    if (const auto* s = dynamic_cast<const LetStmt*>(&stmt)) {
        compileExpr(*s->init);
        uint8_t idx = declareVar(s->name);
        current().emit(OpCode::STORE);
        current().emit(idx);
        return;
    }

    if (const auto* s = dynamic_cast<const AssignStmt*>(&stmt)) {
        compileExpr(*s->value);
        uint8_t idx = lookupVar(s->name);
        current().emit(OpCode::STORE);
        current().emit(idx);
        return;
    }

    if (const auto* s = dynamic_cast<const PrintStmt*>(&stmt)) {
        compileExpr(*s->expr);
        current().emit(OpCode::PRINT);
        return;
    }

    if (const auto* s = dynamic_cast<const IfStmt*>(&stmt)) {
        compileExpr(*s->cond);

        current().emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_false = current().here() - 2;   

        compileStmts(s->then_block);

        if (!s->else_block.empty()) {
            current().emitJump(OpCode::JUMP);
            size_t patch_end = current().here() - 2;

            current().patchJump(patch_false, current().here());  
            compileStmts(s->else_block);
            current().patchJump(patch_end,   current().here());  
        } else {
            current().patchJump(patch_false, current().here());  
        }
        return;
    }

    if (const auto* s = dynamic_cast<const ForStmt*>(&stmt)) {
        compileStmt(*s->init);

        size_t loop_start = current().here();

        compileExpr(*s->cond);
        current().emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_exit = current().here() - 2;

        compileStmts(s->body);

        compileStmt(*s->update);

        current().emitJump(OpCode::JUMP);
        current().patchJump(current().here() - 2, loop_start);

        current().patchJump(patch_exit, current().here());
        return;
    }

    if (const auto* s = dynamic_cast<const WhileStmt*>(&stmt)) {
        size_t loop_start = current().here();

        compileExpr(*s->cond);

        current().emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_exit = current().here() - 2;

        compileStmts(s->body);

        current().emitJump(OpCode::JUMP);
        current().patchJump(current().here() - 2, loop_start);  // loop back

        current().patchJump(patch_exit, current().here());       // exit jump
        return;
    }

    throw runtime_error("Compiler: unknown statement type");
}


// expression compilation :-

void Compiler::compileExpr(const Expr& expr) {

    if (const auto* e = dynamic_cast<const CallExpr*>(&expr)) {
        for (const auto& arg : e->args)
            compileExpr(*arg);
        auto it = functions.find(e->name);
        if (it == functions.end())
            throw runtime_error("Compiler: undefined function '" + e->name + "'");
        current().emit(OpCode::CALL);
        current().emit(static_cast<uint8_t>(e->args.size()));  // arg count
        current().emit(current().addString(e->name));
        return;
    }

    if (const auto* e = dynamic_cast<const AndExpr*>(&expr)) {
        compileExpr(*e->left);
        current().emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch = current().here() - 2;
        compileExpr(*e->right);
        current().emitJump(OpCode::JUMP);
        size_t patch_end = current().here() - 2;
        current().patchJump(patch, current().here());
        current().emit(OpCode::PUSH_FALSE);
        current().patchJump(patch_end, current().here());
        return;
    }

    if (const auto* e = dynamic_cast<const OrExpr*>(&expr)) {
        compileExpr(*e->left);
        current().emitJump(OpCode::JUMP_IF_FALSE);
        size_t patch_false = current().here() - 2;
        current().emit(OpCode::PUSH_TRUE);
        current().emitJump(OpCode::JUMP);
        size_t patch_end = current().here() - 2;
        current().patchJump(patch_false, current().here());
        compileExpr(*e->right);
        current().patchJump(patch_end, current().here());
        return;
    }

    if (const auto* e = dynamic_cast<const UnaryExpr*>(&expr)) {
        compileExpr(*e->operand);
        if (e->op == '-') current().emit(OpCode::NEG);
        if (e->op == '!') current().emit(OpCode::NOT);
        return;
    }

    if (const auto* e = dynamic_cast<const IntLitExpr*>(&expr)) {
        current().emit(OpCode::PUSH_INT);
        current().emitInt32(e->value);
        return;
    }

    if (const auto* e = dynamic_cast<const BoolLitExpr*>(&expr)) {
        current().emit(e->value ? OpCode::PUSH_TRUE : OpCode::PUSH_FALSE);
        return;
    }

    if (const auto* e = dynamic_cast<const StringLitExpr*>(&expr)) {
        uint8_t idx = current().addString(e->value);
        current().emit(OpCode::PUSH_STR);
        current().emit(idx);
        return;
    }

    if (const auto* e = dynamic_cast<const VarExpr*>(&expr)) {
        current().emit(OpCode::LOAD);
        current().emit(lookupVar(e->name));
        return;
    }

    if (dynamic_cast<const InputExpr*>(&expr)) {
        current().emit(OpCode::INPUT);
        return;
    }
    if (dynamic_cast<const SInputExpr*>(&expr)) {   
        current().emit(OpCode::SINPUT);
        return;
    }

    if (const auto* e = dynamic_cast<const BinaryExpr*>(&expr)) {
        compileExpr(*e->left);
        compileExpr(*e->right);
        switch (e->op) {
            case '+': current().emit(OpCode::ADD);  break;
            case '-': current().emit(OpCode::SUB);  break;
            case '*': current().emit(OpCode::MUL);  break;
            case '/': current().emit(OpCode::DIV);  break;
            case '%': current().emit(OpCode::MOD); break;   
            case '=': current().emit(OpCode::EQ);   break;  // '=' represents '=='
            case '!': current().emit(OpCode::NEQ); break;
            case '<': current().emit(OpCode::LESS); break;
            case '>': current().emit(OpCode::GREATER);      break;
            case 'L': current().emit(OpCode::LESS_EQ);      break;
            case 'G': current().emit(OpCode::GREATER_EQ);   break;
            default:
                throw runtime_error(
                    string("Compiler: unknown binary op '") + e->op + "'");
        }
        return;
    }

    throw runtime_error("Compiler: unknown expression type");
}
