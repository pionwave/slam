
#pragma once

#include "ObjectFile.hpp"
#include "Utility.hpp" 
#include <vector>
#include <unordered_map>
#include <string>

class Linker {
public:
    Linker(bool debug = false) // Added debug parameter
        : debug_(debug) {
    }

    // Adds an object file to the linker
    void addObjectFile(const ObjectFile& objFile);

    // Performs the linking process and returns the final memory image
    std::vector<uint8_t> link();

private:
    std::vector<ObjectFile> objectFiles_;
    std::unordered_map<std::string, int32_t> globalSymbolTable_;
    std::vector<std::pair<int, Fixup>> allFixups_; // Pair of object file index and Fixup
    bool debug_; // Debug flag

    // Helper methods
    void buildGlobalSymbolTable();
    void collectFixups();
    void resolveFixups(std::vector<uint8_t>& finalImage, int32_t codeOffset);
    void debugPrint(const std::string& message) const;
};
