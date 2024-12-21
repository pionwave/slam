

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
    std::vector<uint8_t> codeSegment;   // Code instructions
    std::vector<uint8_t> dataSegment;   // Data words
    std::vector<Symbol> symbolTable;
    std::vector<Fixup> fixups;
    size_t codeSize;                    // Size of codeSegment before dataSegment
};

void WriteObjectFile(const ObjectFile& obj, const std::string& path);
ObjectFile ReadObjectFile(const std::string& path);