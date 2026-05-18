#include "vm.h"
#include "opcode.h"
#include <iostream>
#include <stdexcept>
using namespace std;

void VM::push(Value v) {
    stack.push_back(v);
}

Value VM::pop() {
    if (stack.empty())
        throw runtime_error("VM: stack underflow");
    Value v = stack.back();
    stack.pop_back();
    return v;
}

// main dispatch loop

void VM::run(const vector<uint8_t>& code) {
    vars.resize(256);   // pre-allocate all variable slots
    size_t ip = 0;

    // helper lambdas
    auto readByte = [&]() -> uint8_t {
        if (ip >= code.size())
            throw runtime_error("VM: ip out of bounds (readByte)");
        return code[ip++];
    };

    auto readInt32 = [&]() -> int32_t {
        if (ip + 3 >= code.size())
            throw runtime_error("VM: ip out of bounds (readInt32)");
        int32_t v = (int32_t(code[ip    ]) << 24) |
                    (int32_t(code[ip + 1]) << 16) |
                    (int32_t(code[ip + 2]) <<  8) |
                     int32_t(code[ip + 3]);
        ip += 4;
        return v;
    };

    auto readUInt16 = [&]() -> uint16_t {
        if (ip + 1 >= code.size())
            throw runtime_error("VM: ip out of bounds (readUInt16)");
        uint16_t v = (uint16_t(code[ip]) << 8) | uint16_t(code[ip + 1]);
        ip += 2;
        return v;
    };

    // dispatch
    while (true) {
        if (ip >= code.size())
            throw runtime_error("VM: execution past end of bytecode");

        auto op = static_cast<OpCode>(readByte());

        switch (op) {

            // stack push
            case OpCode::PUSH_INT:
                push(Value::fromInt(readInt32()));
                break;

            case OpCode::PUSH_TRUE:
                push(Value::fromBool(true));
                break;

            case OpCode::PUSH_FALSE:
                push(Value::fromBool(false));
                break;

            // variable access
            case OpCode::LOAD: {
                uint8_t idx = readByte();
                push(vars[idx]);
                break;
            }
            case OpCode::STORE: {
                uint8_t idx = readByte();
                vars[idx] = pop();
                break;
            }

            // arithmetic
            case OpCode::ADD: {
                Value b = pop(), a = pop();
                push(Value::fromInt(a.i + b.i));
                break;
            }
            case OpCode::SUB: {
                Value b = pop(), a = pop();
                push(Value::fromInt(a.i - b.i));
                break;
            }
            case OpCode::MUL: {
                Value b = pop(), a = pop();
                push(Value::fromInt(a.i * b.i));
                break;
            }
            case OpCode::DIV: {
                Value b = pop(), a = pop();
                if (b.i == 0)
                    throw runtime_error("VM: division by zero");
                push(Value::fromInt(a.i / b.i));
                break;
            }

            // comparison
            case OpCode::EQ: {
                Value b = pop(), a = pop();
                bool result;
                if (a.type != b.type) {
                    result = false;
                } else if (a.type == ValType::INT) {
                    result = (a.i == b.i);
                } else {
                    result = (a.b == b.b);
                }
                push(Value::fromBool(result));
                break;
            }
            case OpCode::LESS: {
                Value b = pop(), a = pop();
                push(Value::fromBool(a.i < b.i));
                break;
            }

            // input/output
            case OpCode::PRINT: {
                Value v = pop();
                cout << v.toString() << "\n";
                break;
            }
            case OpCode::INPUT: {
                int32_t n;
                if (!(cin >> n))
                    throw runtime_error("VM: failed to read integer from stdin");
                push(Value::fromInt(n));
                break;
            }

            // control flow
            case OpCode::JUMP: {
                uint16_t target = readUInt16();
                ip = target;
                break;
            }
            case OpCode::JUMP_IF_FALSE: {
                uint16_t target = readUInt16();
                if (!pop().truthy()) ip = target;
                break;
            }

            // miscellaneous
            case OpCode::POP:
                pop();
                break;

            case OpCode::HALT:
                return;   // normal exit

            default:
                throw runtime_error(
                    "VM: unknown opcode " +
                    to_string(static_cast<int>(op)));
        }
    }
}
