#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <compiler.h>
#include <unordered_map>


enum class ValType { INT, BOOL, STRING };
struct Value {
    ValType type = ValType::INT;
    int32_t i = 0;      
    bool b = false;     
    std::string s;      

    static Value fromInt (int32_t v) { Value val; val.type = ValType::INT;  val.i = v; return val; }
    static Value fromBool(bool v) { Value val; val.type = ValType::BOOL; val.b = v; return val; }
    static Value fromString(std::string v) { Value val; val.type = ValType::STRING; val.s = v; return val; }

    // in cvm++, any "non-zero int" or "true" or "non-empty string" is truthy
    bool truthy() const {
        if (type == ValType::BOOL)   return b;
        if (type == ValType::STRING) return !s.empty(); 
        return i != 0;
    }

    std::string toString() const {
        if (type == ValType::BOOL)   return b ? "true" : "false";
        if (type == ValType::STRING) return s;         
        return std::to_string(i);
    }
};

struct CallFrame {
    const Chunk* chunk;          // which chunk
    size_t ip;                   // instruction ptr
    size_t stackBase;            // where frame's locals start on the stack
    std::vector<Value> vars;     // frame's variable slots
};

class VM {
public:
    void run(const Chunk& chunk, const std::unordered_map<std::string, FunctionObject>& functions);

private:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;

    void  push(Value v);
    Value pop();
};
