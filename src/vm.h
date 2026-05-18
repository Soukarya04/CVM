#pragma once
#include <vector>
#include <cstdint>
#include <string>

enum class ValType { INT, BOOL };
struct Value {
    ValType type = ValType::INT;
    union { int32_t i = 0; bool b; };

    static Value fromInt (int32_t v) { Value val; val.type = ValType::INT;  val.i = v; return val; }
    static Value fromBool(bool    v) { Value val; val.type = ValType::BOOL; val.b = v; return val; }

    // in cvm++, any non-zero int or true is truthy
    bool truthy() const {
        return (type == ValType::BOOL) ? b : (i != 0);
    }

    std::string toString() const {
        if (type == ValType::BOOL) return b ? "true" : "false";
        return std::to_string(i);
    }
};

class VM {
public:
    void run(const std::vector<uint8_t>& code);

private:
    std::vector<Value> stack;
    std::vector<Value> vars;   // 256 variable slots

    void  push(Value v);
    Value pop();
};
