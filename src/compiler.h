#pragma once
#include "opcode.h"
#include "ast.h"
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<std::string> stringPool;

    // emit one byte
    void emit(uint8_t b)   { code.push_back(b); }
    void emit(OpCode op)   { code.push_back(static_cast<uint8_t>(op)); }

    // emit a 32-bit integer in big-endian order (for PUSH_INT)
    void emitInt32(int32_t v) {
        code.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
        code.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
        code.push_back(static_cast<uint8_t>((v >>  8) & 0xFF));
        code.push_back(static_cast<uint8_t>( v        & 0xFF));
    }

    // emit a jump opcode with a placeholder address (0xFFFF)
    void emitJump(OpCode op) {
        emit(op);
        code.push_back(0xFF);
        code.push_back(0xFF);
    }

    // current write position (= next instruction index)
    size_t here() const { return code.size(); }

    // backpatch a previously-emitted jump to point at "target"
    void patchJump(size_t patch_pos, size_t target) {
        code[patch_pos]     = static_cast<uint8_t>((target >> 8) & 0xFF);
        code[patch_pos + 1] = static_cast<uint8_t>( target       & 0xFF);
    }

    // add a string to the pool, return its index
    uint8_t addString(const std::string& s) {
        stringPool.push_back(s);
        return static_cast<uint8_t>(stringPool.size() - 1);
    }
};

// represents a compiled function
struct FunctionObject {
    std::string name;
    int arity;                  // number of parameters
    Chunk chunk;                // the function's own bytecode
};

class Compiler {
public:
    Chunk compile(const Program& prog);

    // function table -> filled during compilation, read by VM
    std::unordered_map<std::string, FunctionObject> functions;

private:
    Chunk   chunk;                                        // main chunk
    std::unordered_map<std::string, uint8_t> varMap;     // current scope vars
    uint8_t nextVar = 0;

    // which function we're currently compiling (nullptr = main)
    FunctionObject* currentFn = nullptr;

    // emit to wherever we're currently compiling
    Chunk& current() {
        return currentFn ? currentFn->chunk : chunk;
    }

    uint8_t declareVar(const std::string& name);
    uint8_t lookupVar (const std::string& name);

    void compileStmts(const StmtList& stmts);
    void compileStmt (const Stmt&     stmt);
    void compileExpr (const Expr&     expr);
    void compileFn   (const FnDecl&   fn);    
};
