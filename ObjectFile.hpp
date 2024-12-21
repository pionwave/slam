// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

struct Symbol {
    std::string name;
    int32_t address;
    bool isExternal = false;
    bool isData = false;
};

struct Fixup {
    int32_t bytecodeOffset;
    std::string symbolName;
    bool isDataLabel;
    bool isMemoryReference;
};

struct ObjectFile {
    std::vector<uint8_t> codeSegment;
    std::vector<uint8_t> dataSegment;
    std::vector<Symbol> symbolTable;
    std::vector<Fixup> fixups;
    size_t codeSize;
};

void WriteObjectFile(const ObjectFile& obj, const std::string& path);
ObjectFile ReadObjectFile(const std::string& path);