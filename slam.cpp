// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <stdexcept>
#include <cstring>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Compiler.hpp"
#include "Linker.hpp"
#include "VM.hpp"

std::string stripExtension(const std::string& path) {
    size_t last_slash = path.find_last_of("/\\");

    size_t last_dot = path.find_last_of('.');

    if (last_dot == std::string::npos ||
        (last_slash != std::string::npos && last_dot < last_slash)) {
        return path;
    }

    return path.substr(0, last_dot);
}

ObjectFile compileFile(const std::string& str, bool debug = false)
{
    std::ifstream file(str);
    if (!file) {
        std::string err = "Failed to open " + str;
        throw std::exception(err.c_str());
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        // Strip comments starting with ';'
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        // Trim trailing spaces
        while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
            line.pop_back();
        lines.push_back(line);
    }

    Parser parser(lines, debug);
    auto instructions = parser.getInstructions();

        /*
    for (auto& instr : instructions)
    {
        std::cout << (int)instr.type << std::endl;

        std::cout << "    ";

        for (auto& oper : instr.operands)
        {
            if (oper.isLabel)
            {
                std::cout << oper.labelName;
            }
            else if (oper.isMemory)
            {
                std::cout << oper.immediate;
            }
            else if (oper.isRegister)
            {
                std::cout << "R" << oper.regIndex;
            }

            std::cout << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Data segment: \n";

    for (auto& seg : parser.getDataSegment())
    {
        std::cout << seg << " ";
    }

    std::cout << "\n\n";
    std::cout << "Data labels: \n";

    for (auto& lab : parser.getDataLabels())
    {
        std::cout << lab.first << " " << lab.second << std::endl;
    }

    std::cout << "\n";
    */

    Compiler compiler(instructions, parser.getDataSegment(), parser.getDataLabels(), debug);

    return compiler.compile();
}

int32_t main(int32_t argc, int8_t *argv[])
{
    try {
        auto obj = compileFile("memtest.asm");
        auto obj2 = compileFile("fib.asm");
        auto obj3 = compileFile("euclid.asm");

        WriteObjectFile(obj, stripExtension("memtest.asm") + ".obj");
        WriteObjectFile(obj2, stripExtension("fib.asm") + ".obj");
        WriteObjectFile(obj3, stripExtension("euclid.asm") + ".obj");

        Linker linker(false);

        linker.addObjectFile(obj);
        linker.addObjectFile(obj2);
        linker.addObjectFile(obj3);

        auto bytecode = linker.link();

        VM vm(bytecode, 1048576, 65536, false);
        vm.run();
        vm.printRegisters();

        std::cout << "Program finished successfully.\n";
    }
    catch (std::exception& ex) 
    {
        std::cerr << ex.what() << "\n";
    }

    return 0;
}
