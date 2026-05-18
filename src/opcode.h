#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    PUSH_INT,
    PUSH_TRUE,
    PUSH_FALSE,
    LOAD,
    STORE,
    ADD, SUB, MUL, DIV,
    EQ, LESS,
    PRINT,
    INPUT,
    JUMP,
    JUMP_IF_FALSE,
    POP,
    HALT };
