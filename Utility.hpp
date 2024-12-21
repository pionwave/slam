
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "BytecodeOp.hpp"

// Converts a BytecodeOp to its string representation
std::string bcOpName(BytecodeOp op);

// Returns the number of operands for a given BytecodeOp
int operandCountForOp(BytecodeOp op);

// Reads a 32-bit integer from bytecode starting at a given index (little endian)
int32_t readInt32(const std::vector<uint8_t>& bytecode, size_t index);

// Prints the bytecode in a readable, disassembled format
void printBytecode(const std::vector<uint8_t>& bytecode, size_t codeSize);
