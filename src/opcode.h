#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    PUSH_INT,
    PUSH_TRUE,
    PUSH_FALSE,
    LOAD,
    STORE,
    ADD, SUB, MUL, DIV, MOD,
    NEG, NOT,
    EQ, NEQ, LESS, GREATER, LESS_EQ, GREATER_EQ,
    PRINT,
    INPUT,
    SINPUT,
    JUMP,
    JUMP_IF_FALSE,
    PUSH_STR,
    CALL,
    RETURN,
    POP,
    HALT 
};
