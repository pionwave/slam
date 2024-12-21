// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "BytecodeOp.hpp"

std::string bcOpName(BytecodeOp op);
int operandCountForOp(BytecodeOp op);
int32_t readInt32(const std::vector<uint8_t>& bytecode, size_t index);

void printBytecode(const std::vector<uint8_t>& bytecode, size_t codeSize);
