# CVM++ — A Stack-Based Virtual Machine in C++

A complete, from-scratch implementation of a mini programming language and its
virtual machine, written in C++17.

---

## Architecture

```
Source Code (.cvm)
        │
        ▼
  ┌──────────┐
  │  Lexer   │  Tokenises raw text into a flat list of Tokens
  └──────────┘
        │  vector<Token>
        ▼
  ┌──────────┐
  │  Parser  │  Recursive-descent parser → Abstract Syntax Tree (AST)
  └──────────┘
        │  Program (tree of Stmt / Expr nodes)
        ▼
  ┌──────────────┐
  │   Compiler   │  Walks the AST and emits bytecode into a Chunk
  └──────────────┘
        │  vector<uint8_t>  (flat bytecode array)
        ▼
  ┌─────────────────────────────┐
  │  Stack-Based Virtual Machine│  Interprets bytecode with a value stack
  └─────────────────────────────┘
        │
        ▼
     Output
```

---

## Language Reference

### Data types
| Type    | Examples             |
|---------|----------------------|
| Integer | `0`, `42`, `-1`      |
| Boolean | `true`, `false`      |

### Operators
| Operator | Meaning    |
|----------|------------|
| `+`      | Add        |
| `-`      | Subtract   |
| `*`      | Multiply   |
| `/`      | Divide     |
| `==`     | Equal      |
| `<`      | Less than  |

### Statements

```
// Declare a variable
let x = 10;

// Assign to an existing variable
x = x + 1;

// Print a value
print x;

// Read an integer from stdin
let n = input;

// Conditional
if (x < 20) {
    print x;
} else {
    print 0;
}

// Loop
while (x < 100) {
    x = x * 2;
}

// Single-line comment
```

### Example Programs

**Fibonacci sequence**
```
let a = 0;
let b = 1;
let i = 0;
while (i < 10) {
    print a;
    let tmp = a + b;
    a = b;
    b = tmp;
    i = i + 1;
}
```

---

## Bytecode Instruction Set

| Opcode          | Bytes          | Effect                            |
|-----------------|----------------|-----------------------------------|
| `PUSH_INT`      | `[b3][b2][b1][b0]` | Push 32-bit int (big-endian)  |
| `PUSH_TRUE`     | —              | Push boolean `true`               |
| `PUSH_FALSE`    | —              | Push boolean `false`              |
| `LOAD`          | `[idx]`        | Push variable at slot *idx*       |
| `STORE`         | `[idx]`        | Pop → variable slot *idx*         |
| `ADD`           | —              | Pop b, a → push a+b               |
| `SUB`           | —              | Pop b, a → push a-b               |
| `MUL`           | —              | Pop b, a → push a*b               |
| `DIV`           | —              | Pop b, a → push a/b               |
| `EQ`            | —              | Pop b, a → push a==b              |
| `LESS`          | —              | Pop b, a → push a<b               |
| `PRINT`         | —              | Pop and print to stdout           |
| `INPUT`         | —              | Read int from stdin, push         |
| `JUMP`          | `[hi][lo]`     | ip = address                      |
| `JUMP_IF_FALSE` | `[hi][lo]`     | Pop; if falsy → ip = address      |
| `POP`           | —              | Discard top of stack              |
| `HALT`          | —              | Stop execution                    |

---

## Building

### With CMake (recommended)
```bash
mkdir build && cd build
cmake ..
cmake --build .
./cvm ../examples/fibonacci.cvm
```

### Direct g++ compile
```bash
g++ -std=c++17 -Isrc src/main.cpp src/lexer.cpp src/parser.cpp \
    src/compiler.cpp src/vm.cpp -o cvm
```

### Running
```bash
./cvm script.cvm          # execute a file
./cvm                     # interactive REPL
```

---

## File Structure

```
cvm_plus_plus/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── opcode.h       ← shared opcode enum
│   ├── token.h        ← Token struct & TokenType enum
│   ├── lexer.h/cpp    ← Lexer (tokenizer)
│   ├── ast.h          ← AST node hierarchy
│   ├── parser.h/cpp   ← Recursive-descent parser
│   ├── compiler.h/cpp ← AST → bytecode compiler
│   ├── vm.h/cpp       ← Stack-based VM
│   └── main.cpp       ← Entry point (REPL + file runner)
└── examples/
    ├── arithmetic.cvm
    ├── fibonacci.cvm
    └── fizzbuzz.cvm
```

---

## Key Concepts Used

- **Lexical Analysis** — character-by-character scanning, keyword table lookup
- **Recursive Descent Parsing** — each grammar rule maps to a `parse*` method
- **Abstract Syntax Tree** — polymorphic node hierarchy using `unique_ptr`
- **Backpatching** — emit placeholder jump addresses, fill them in later
- **Stack-Based VM** — all operations work on an implicit value stack
- **Tagged Values** — `Value` union with a type tag for int/bool

---

*Reference: "Crafting Interpreters" by Robert Nystrom — highly recommended for
extending this project with strings, functions, and closures.*
