#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
using namespace std;

static bool runSource(const string& source) {
    try {
        // stage 1: lex
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // stage 2: parse
        Parser parser(move(tokens));
        Program prog = parser.parse();

        // stage 3: compile
        Compiler compiler;
        Chunk chunk = compiler.compile(prog);

        // stage 4: execute
        VM vm;
        vm.run(chunk, compiler.functions);

        return true;

    } catch (const exception& e) {
        cerr << "\033[1;31mError:\033[0m " << e.what() << "\n";
        return false;
    }
}

int main(int argc, char* argv[]) {

    // file mode
    if (argc == 2) {
        ifstream file(argv[1]);
        if (!file) {
            cerr << "Cannot open file: " << argv[1] << "\n";
            return 1;
        }
        ostringstream ss;
        ss << file.rdbuf();
        return runSource(ss.str()) ? 0 : 1;
    }

    // repl mode
    cout << "================================\n";
    cout << "   CVM++ Interactive REPL\n";
    cout << "   Type 'exit' to quit\n";
    cout << "================================\n\n";

    string line;
    while (true) {
        cout << ">> ";
        cout.flush();
        if (!getline(cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        runSource(line);
    }

    return 0;
}
