// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include "ObjectFile.hpp"
#include "Utility.hpp" 
#include <vector>
#include <unordered_map>
#include <string>

class Linker {
public:
    Linker(bool debug = false)
        : debug_(debug) {
    }

    void addObjectFile(const ObjectFile& objFile);

    std::vector<uint8_t> link();

private:
    std::vector<ObjectFile> objectFiles_;
    std::unordered_map<std::string, int32_t> globalSymbolTable_;
    std::vector<std::pair<int32_t, Fixup>> allFixups_;
    bool debug_;

    void buildGlobalSymbolTable();
    void collectFixups();
    void resolveFixups(std::vector<uint8_t>& finalImage, int32_t codeOffset);
    void debugPrint(const std::string& message) const;
};
