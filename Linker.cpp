// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#include "Linker.hpp"
#include <stdexcept>
#include <iostream>

void Linker::addObjectFile(const ObjectFile& objFile) {  
    objectFiles_.push_back(objFile);
    if (debug_) {
        std::cout << "[Linker Debug] Added object file " << objectFiles_.size() << "\n";
        std::cout << "[Linker Debug] Bytecode of object file " << objectFiles_.size() << ":\n";
        //printBytecode(objFile.codeSegment, objFile.codeSize);

        std::cout << "\n--- Data Segment of Object File " << objectFiles_.size() << " ---\n";
        size_t dataStart = objFile.codeSize;
        size_t dataSize = objFile.dataSegment.size();
        std::cout << "Data segment size: " << dataSize << " bytes\n";
        for (size_t i = 0; i < objFile.dataSegment.size(); i += 4) {
            if (i + 3 < objFile.dataSegment.size()) {
                int32_t val = readInt32(objFile.dataSegment, i);
                std::cout << "0x" << std::hex << (dataStart + i) << std::dec << ": " << val << "\n";
            }
        }
        std::cout << "--- End of Data Segment ---\n\n";
    }
}

void Linker::debugPrint(const std::string& message) const {
    if (debug_) {
        std::cout << "[Linker Debug] " << message << "\n";
    }
}

void Linker::buildGlobalSymbolTable() {
    debugPrint("Building global symbol table...");
    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        const auto& obj = objectFiles_[i];
        for (const auto& sym : obj.symbolTable) {
            if (!sym.isExternal) {
                if (globalSymbolTable_.find(sym.name) != globalSymbolTable_.end()) {
                    throw std::runtime_error("Multiple definitions of symbol: " + sym.name);
                }
                globalSymbolTable_[sym.name] = sym.address;
                debugPrint("Registered symbol '" + sym.name + "' at address " + std::to_string(sym.address));
            }
        }
    }
    debugPrint("Global symbol table built.");
}

void Linker::collectFixups() {
    debugPrint("Collecting all fixups from object files...");

    int32_t accumulatedCodeSegment = 0;
    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        for (const auto& fix : objectFiles_[i].fixups) {
            allFixups_.emplace_back(static_cast<int>(i), fix);
            debugPrint("Collected fixup for symbol '" + fix.symbolName + "' in object file " + std::to_string(i + 1));
        }
    }
    debugPrint("All fixups collected.");
}

void Linker::resolveFixups(std::vector<uint8_t>& finalImage, int32_t codeOffset) {
    debugPrint("Resolving fixups...");

    std::vector<int32_t> cumulativeCodeOffsets(objectFiles_.size(), 0);
    int32_t currentOffset = codeOffset;
    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        cumulativeCodeOffsets[i] = currentOffset;
        currentOffset += static_cast<int32_t>(objectFiles_[i].codeSegment.size());
    }

    for (const auto& [objIndex, fix] : allFixups_) {
        int32_t adjustedOffset = fix.bytecodeOffset;
        if (objIndex != -1)
        {
            adjustedOffset = fix.bytecodeOffset + cumulativeCodeOffsets[objIndex];
        }

        auto it = globalSymbolTable_.find(fix.symbolName);
        if (it != globalSymbolTable_.end()) {
            int32_t symbolAddr = it->second;
            debugPrint("Resolved symbol '" + fix.symbolName + "' to address " + std::to_string(symbolAddr));

            if (adjustedOffset + 3 >= static_cast<int32_t>(finalImage.size())) {
                throw std::runtime_error("Fixup address out of range for symbol: " + fix.symbolName);
            }

            uint8_t operandType = fix.isMemoryReference ? static_cast<uint8_t>(2) : static_cast<uint8_t>(0);
            finalImage[adjustedOffset - 1] = operandType;

            finalImage[adjustedOffset] = static_cast<uint8_t>(symbolAddr & 0xFF);
            finalImage[adjustedOffset + 1] = static_cast<uint8_t>((symbolAddr >> 8) & 0xFF);
            finalImage[adjustedOffset + 2] = static_cast<uint8_t>((symbolAddr >> 16) & 0xFF);
            finalImage[adjustedOffset + 3] = static_cast<uint8_t>((symbolAddr >> 24) & 0xFF);

            debugPrint("Updated operand at bytecode offset " + std::to_string(adjustedOffset) +
                " with address " + std::to_string(symbolAddr));
        }
        else {
            throw std::runtime_error("Undefined symbol during linking: " + fix.symbolName);
        }
    }
    debugPrint("All fixups resolved.");
}

std::vector<uint8_t> Linker::link() {
    debugPrint("Starting linking process...");

    buildGlobalSymbolTable();

    auto mainIt = globalSymbolTable_.find("MAIN");
    if (mainIt == globalSymbolTable_.end()) {
        throw std::runtime_error("Entry point 'main' not found.");
    }

    collectFixups();

    std::vector<uint8_t> finalImage;

    uint32_t mainOffset = 6;

    for (auto& [findex, fix] : allFixups_)
    {
        fix.bytecodeOffset += mainOffset;
    }

    
    finalImage.push_back(BC_JMP); // JMP opcode
    finalImage.push_back(0x00); // Operand type 0 (Immediate)
    finalImage.insert(finalImage.end(), { 0x00, 0x00, 0x00, 0x00 }); 

    Fixup mainFix;
    mainFix.bytecodeOffset = 2;
    mainFix.symbolName = "MAIN";
    mainFix.isDataLabel = false;
    mainFix.isMemoryReference = false;
    allFixups_.emplace_back(-1, mainFix);
    

    debugPrint("Inserted JMP to 'main' at bytecode offset 0");

    int32_t totalCodeSize = mainOffset;
    int32_t totalDataSize = 0;
    for (const auto& obj : objectFiles_) {
        totalCodeSize += static_cast<int32_t>(obj.codeSegment.size());
        totalDataSize += static_cast<int32_t>(obj.dataSegment.size());
    }

    debugPrint("Total code size (including JMP): " + std::to_string(totalCodeSize) + " bytes");
    debugPrint("Total data size: " + std::to_string(totalDataSize) + " bytes");

    std::vector<int32_t> codeOffsets(objectFiles_.size(), mainOffset);
    int32_t currentCodeOffset = mainOffset;

    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        codeOffsets[i] = currentCodeOffset;
        finalImage.insert(finalImage.end(), objectFiles_[i].codeSegment.begin(), objectFiles_[i].codeSegment.end());
        debugPrint("Appended code segment from object file " + std::to_string(i + 1) +
            " (size: " + std::to_string(objectFiles_[i].codeSegment.size()) + " bytes)");
        currentCodeOffset += static_cast<int32_t>(objectFiles_[i].codeSegment.size());
    }

    int32_t dataStartOffset = currentCodeOffset;
    std::vector<int32_t> dataOffsets;

    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        finalImage.insert(finalImage.end(), objectFiles_[i].dataSegment.begin(), objectFiles_[i].dataSegment.end());
        debugPrint("Appended data segment from object file " + std::to_string(i + 1) +
            " (size: " + std::to_string(objectFiles_[i].dataSegment.size()) + " bytes)");
        currentCodeOffset += static_cast<int32_t>(objectFiles_[i].dataSegment.size());
    }

    int32_t dataOffset = 0;
    for (size_t i = 0; i < objectFiles_.size(); ++i)
    {
        dataOffsets.push_back(dataOffset);
        dataOffset += static_cast<int32_t>(objectFiles_[i].dataSegment.size());
    }

    for (size_t i = 0; i < objectFiles_.size(); ++i) {
        for (const auto& sym : objectFiles_[i].symbolTable) {
            if (!sym.isExternal) 
            {
                globalSymbolTable_[sym.name] = sym.address;

                if (sym.isData)
                {
                    globalSymbolTable_[sym.name] += dataStartOffset + dataOffsets[i] - static_cast<int32_t>(objectFiles_[i].codeSize);//dataOffsets[i] - objectFiles_[i].codeSize;
                }
                else
                {
                    globalSymbolTable_[sym.name] += codeOffsets[i]; // need to use the codeoffset tables as we link multiple code segments and the offset is variable
                }
                debugPrint("Symbol '" + sym.name + "' assigned address " + std::to_string(globalSymbolTable_[sym.name]));
            }
        }
    }

    resolveFixups(finalImage, 0);

    debugPrint("Linking process completed successfully.");
    debugPrint("Final memory image size: " + std::to_string(finalImage.size()) + " bytes");

    if (debug_) {
        printBytecode(finalImage, totalCodeSize);

        std::cout << "\n--- Final Data Segment ---\n";
        size_t dataStart = totalCodeSize;
        size_t dataSize = finalImage.size() - dataStart;
        std::cout << "Data segment size: " << dataSize << " bytes\n";
        for (size_t i = dataStart; i < finalImage.size(); i += 4) {
            if (i + 3 < finalImage.size()) {
                int32_t val = readInt32(finalImage, i);
                std::cout << "0x" << std::hex << i << std::dec << ": " << val << "\n";
            }
        }
        std::cout << "--- End of Final Data Segment ---\n\n";
    }

    return finalImage;
}
