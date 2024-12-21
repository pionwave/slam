// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include "ObjectFile.hpp"
#include "BytecodeOp.hpp"
#include "Parser.hpp"
#include <vector>
#include <unordered_map>
#include <string>

class Compiler {
public:
    Compiler(const std::vector<Instruction>& instructions,
        const std::vector<int32_t>& dataSegment,
        const std::unordered_map<std::string, int>& dataLabels,
        bool debug = false)
        : instructions_(instructions),
        dataSegment_(dataSegment),
        dataLabels_(dataLabels),
        debug_(debug) {
    }

    ObjectFile compile();

private:
    void emitByte(std::vector<uint8_t>& segment, uint8_t byte);
    void emitInt32(std::vector<uint8_t>& segment, int32_t value);
    BytecodeOp instrToBCOp(InstructionType it);
    void emitOperand(ObjectFile& obj, const Operand& op, std::vector<Fixup>& fixups);
    void debugPrint(const std::string& message) const;
    std::string instructionTypeName(InstructionType it) const;

    const std::vector<Instruction>& instructions_;
    const std::vector<int32_t>& dataSegment_;
    const std::unordered_map<std::string, int>& dataLabels_;
    bool debug_;
};
