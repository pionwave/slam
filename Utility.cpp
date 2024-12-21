// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#include "Utility.hpp"
#include <iostream>
#include <iomanip>
#include <stdexcept>

std::string bcOpName(BytecodeOp op) {
    switch (op) {
    case BC_MOV: return "MOV";
    case BC_ADD: return "ADD";
    case BC_SUB: return "SUB";
    case BC_MUL: return "MUL";
    case BC_DIV: return "DIV";
    case BC_AND: return "AND";
    case BC_OR:  return "OR";
    case BC_XOR: return "XOR";
    case BC_SHL: return "SHL";
    case BC_SHR: return "SHR";
    case BC_CMP: return "CMP";
    case BC_JMP: return "JMP";
    case BC_JE:  return "JE";
    case BC_JNE: return "JNE";
    case BC_JG:  return "JG";
    case BC_JL:  return "JL";
    case BC_JLE: return "JLE";
    case BC_JGE: return "JGE";
    case BC_LOAD:return "LOAD";
    case BC_STORE:return "STORE";
    case BC_PUSH:return "PUSH";
    case BC_POP: return "POP";
    case BC_CALL:return "CALL";
    case BC_RET: return "RET";
    default:    return "UNKNOWN";
    }
}

int operandCountForOp(BytecodeOp op) {
    switch (op) {
    case BC_MOV: return 2;
    case BC_ADD: return 3;
    case BC_SUB: return 3;
    case BC_MUL: return 3;
    case BC_DIV: return 3;
    case BC_AND: return 3;
    case BC_OR:  return 3;
    case BC_XOR: return 3;
    case BC_SHL: return 3;
    case BC_SHR: return 3;
    case BC_CMP: return 2;
    case BC_JMP: return 1;
    case BC_JE:  return 1;
    case BC_JNE: return 1;
    case BC_JG:  return 1;
    case BC_JL:  return 1;
    case BC_JLE: return 1;
    case BC_JGE: return 1;
    case BC_LOAD:return 2;
    case BC_STORE:return 2;
    case BC_PUSH:return 1;
    case BC_POP: return 1;
    case BC_CALL:return 1;
    case BC_RET: return 0;
    default:    return 0;
    }
}

int32_t readInt32(const std::vector<uint8_t>& bytecode, size_t index) {
    if (index + 4 > bytecode.size()) {
        throw std::out_of_range("Attempt to read beyond bytecode size: " + std::to_string(index));
    }
    return static_cast<int32_t>(bytecode[index]) |
        (static_cast<int32_t>(bytecode[index + 1]) << 8) |
        (static_cast<int32_t>(bytecode[index + 2]) << 16) |
        (static_cast<int32_t>(bytecode[index + 3]) << 24);
}

void printBytecode(const std::vector<uint8_t>& bytecode, size_t codeSize) {
    std::cout << "\n--- Bytecode Debug Output ---\n";
    std::cout << "Total bytecode size: " << bytecode.size() << " bytes\n";

    size_t ip = 0;
    while (ip < codeSize) {
        size_t instrStart = ip;
        if (ip >= bytecode.size()) break;

        uint8_t opByte = bytecode[ip++];
        BytecodeOp op = static_cast<BytecodeOp>(opByte);
        std::string opName = bcOpName(op);

        std::cout << "0x" << std::hex << std::setw(4) << std::setfill('0') << instrStart << std::dec
            << ": " << opName;

        int operandCount = operandCountForOp(op);

        for (int i = 0; i < operandCount; ++i) {
            if (ip >= bytecode.size()) {
                std::cout << " [Truncated]";
                break;
            }
            uint8_t type = bytecode[ip++];
            int32_t val = readInt32(bytecode, ip);
            ip += 4;

            std::cout << "  [type=" << static_cast<int>(type) << ", val=" << val << "]";
        }

        std::cout << "\n";
    }

    std::cout << "--- End of CODE Disassembly ---\n\n";

    size_t dataStart = codeSize;
    size_t dataSize = bytecode.size() - dataStart;
    std::cout << "Data segment starts at 0x" << std::hex << dataStart << std::dec << ":\n";

    if (dataSize % 4 != 0) {
        std::cout << "Warning: Data size not a multiple of 4 bytes.\n";
    }

    //for (size_t i = 0; i + 3 < bytecode.size(); i += 4) {
        //int32_t val = readInt32(bytecode, dataStart + i);
        //std::cout << "0x" << std::hex << (dataStart + i) << std::dec << ": " << val << "\n";
    //}

    std::cout << "--- End of Bytecode Debug Output ---\n\n";
}
