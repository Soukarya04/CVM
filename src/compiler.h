#pragma once
#include "opcode.h"
#include "ast.h"
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>

struct Chunk {
    std::vector<uint8_t> code;

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
};

class Compiler {
public:
    Chunk compile(const Program& prog);

private:
    Chunk   chunk;
    std::unordered_map<std::string, uint8_t> varMap;
    uint8_t nextVar = 0;

    uint8_t declareVar(const std::string& name);  // create new slot
    uint8_t lookupVar (const std::string& name);  // must already exist

    void compileStmts(const StmtList& stmts);
    void compileStmt (const Stmt&     stmt);
    void compileExpr (const Expr&     expr);
};
