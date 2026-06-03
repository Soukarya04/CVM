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

void VM::run(const Chunk& mainChunk,
             const unordered_map<string, FunctionObject>& functions) {

    // push the main frame
    frames.push_back({ &mainChunk, 0, 0, vector<Value>(256) });

    while (!frames.empty()) {
        CallFrame& frame = frames.back();
        const auto& code = frame.chunk->code;

        // helper lambdas
        auto readByte = [&]() -> uint8_t {
            if (frame.ip >= code.size())
                throw runtime_error("VM: ip out of bounds (readByte)");
            return code[frame.ip++];
        };

        auto readInt32 = [&]() -> int32_t {
            if (frame.ip + 3 >= code.size())
                throw runtime_error("VM: ip out of bounds (readInt32)");
            int32_t v = (int32_t(code[frame.ip    ]) << 24) |
                        (int32_t(code[frame.ip + 1]) << 16) |
                        (int32_t(code[frame.ip + 2]) <<  8) |
                         int32_t(code[frame.ip + 3]);
            frame.ip += 4;
            return v;
        };

        auto readUInt16 = [&]() -> uint16_t {
            if (frame.ip + 1 >= code.size())
                throw runtime_error("VM: ip out of bounds (readUInt16)");
            uint16_t v = (uint16_t(code[frame.ip]) << 8) |
                          uint16_t(code[frame.ip + 1]);
            frame.ip += 2;
            return v;
        };

        // dispatch
        bool halt = false;
        while (!halt && frame.ip < code.size()) {
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

                // strings
                case OpCode::PUSH_STR: {
                    uint8_t idx = readByte();
                    push(Value::fromString(frame.chunk->stringPool[idx]));
                    break;
                }

                // variable access
                case OpCode::LOAD: {
                    uint8_t idx = readByte();
                    push(frame.vars[idx]);
                    break;
                }

                case OpCode::STORE: {
                    uint8_t idx = readByte();
                    frame.vars[idx] = pop();
                    break;
                }

                // arithmetic
                case OpCode::ADD: {                 // used for concatenation too
                    Value b = pop(), a = pop();
                    if (a.type == ValType::STRING || b.type == ValType::STRING)
                        push(Value::fromString(a.toString() + b.toString()));
                    else
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

                case OpCode::MOD: {
                    Value b = pop(), a = pop();
                    if (b.i == 0)
                        throw runtime_error("VM: modulo by zero");
                    push(Value::fromInt(a.i % b.i));
                    break;
                }

                case OpCode::NEG: {
                    Value a = pop();
                    push(Value::fromInt(-a.i));
                    break;
                }

                case OpCode::NOT: {                 // used for truthy/falsy logic
                    Value a = pop();
                    push(Value::fromBool(!a.truthy()));
                    break;
                }

                // comparison
                case OpCode::EQ: {
                    Value b = pop(), a = pop();
                    bool result;
                    if (a.type != b.type)               result = false;
                    else if (a.type == ValType::INT)    result = (a.i == b.i);
                    else if (a.type == ValType::BOOL)   result = (a.b == b.b);
                    else                                result = (a.s == b.s);
                    push(Value::fromBool(result));
                    break;
                }

                case OpCode::NEQ: {
                    Value b = pop(), a = pop();
                    bool result;
                    if (a.type != b.type)               result = true;
                    else if (a.type == ValType::INT)    result = (a.i != b.i);
                    else if (a.type == ValType::BOOL)   result = (a.b != b.b);
                    else                                result = (a.s != b.s);
                    push(Value::fromBool(result));
                    break;
                }

                case OpCode::LESS: {
                    Value b = pop(), a = pop();
                    if (a.type != ValType::INT || b.type != ValType::INT)
                        throw runtime_error("VM: '<' requires integers");
                    push(Value::fromBool(a.i < b.i));
                    break;
                }

                case OpCode::GREATER: {
                    Value b = pop(), a = pop();
                    if (a.type != ValType::INT || b.type != ValType::INT)
                        throw runtime_error("VM: '>' requires integers");
                    push(Value::fromBool(a.i > b.i));
                    break;
                }

                case OpCode::LESS_EQ: {
                    Value b = pop(), a = pop();
                    if (a.type != ValType::INT || b.type != ValType::INT)
                        throw runtime_error("VM: '<=' requires integers");
                    push(Value::fromBool(a.i <= b.i));
                    break;
                }

                case OpCode::GREATER_EQ: {
                    Value b = pop(), a = pop();
                    if (a.type != ValType::INT || b.type != ValType::INT)
                        throw runtime_error("VM: '>=' requires integers");
                    push(Value::fromBool(a.i >= b.i));
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
                        throw runtime_error("VM: failed to read integer");
                    cin.ignore();
                    push(Value::fromInt(n));
                    break;
                }
                case OpCode::SINPUT: {
                    string line;
                    getline(cin, line);
                    push(Value::fromString(line));
                    break;
                }

                // control flow
                case OpCode::JUMP: {
                    uint16_t target = readUInt16();
                    frame.ip = target;
                    break;
                }

                case OpCode::JUMP_IF_FALSE: {
                    uint16_t target = readUInt16();
                    if (!pop().truthy()) frame.ip = target;
                    break;
                }
                
                // functions
                case OpCode::CALL: {
                    uint8_t argCount  = readByte();
                    uint8_t nameIdx   = readByte();
                    const string& fnName = frame.chunk->stringPool[nameIdx];

                    auto it = functions.find(fnName);
                    if (it == functions.end())
                        throw runtime_error("VM: undefined function '" + fnName + "'");

                    const FunctionObject& fnObj = it->second;
                    if (argCount != fnObj.arity)
                        throw runtime_error("VM: function '" + fnName +
                            "' expects " + to_string(fnObj.arity) +
                            " args, got " + to_string(argCount));

                    CallFrame newFrame;
                    newFrame.chunk     = &fnObj.chunk;
                    newFrame.ip        = 0;
                    newFrame.stackBase = stack.size() - argCount;
                    newFrame.vars.resize(256);

                    for (int i = argCount - 1; i >= 0; i--)
                        newFrame.vars[i] = pop();

                    frames.push_back(move(newFrame));
                    halt = true;  
                    break;
                }

                case OpCode::RETURN: {
                    Value retVal = pop();   
                    frames.pop_back();      

                    if (frames.empty()) return;  

                    push(retVal);  
                    halt = true;   
                    break;
                }

                // miscellaneous
                case OpCode::POP:
                    pop();
                    break;

                case OpCode::HALT:
                    return;     // normal exit

                default:
                    throw runtime_error(
                        "VM: unknown opcode " +
                        to_string(static_cast<int>(op)));
            }
        }
    }
}
