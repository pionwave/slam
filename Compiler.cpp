// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#include "Compiler.hpp"
#include "Utility.hpp"
#include <stdexcept>
#include <iostream>
#include <iomanip>

std::string Compiler::instructionTypeName(InstructionType it) const {
    switch (it) {
    case InstructionType::INVALID: return "INVALID";
    case InstructionType::MOV:     return "MOV";
    case InstructionType::ADD:     return "ADD";
    case InstructionType::SUB:     return "SUB";
    case InstructionType::MUL:     return "MUL";
    case InstructionType::DIV:     return "DIV";
    case InstructionType::AND:     return "AND";
    case InstructionType::OR:      return "OR";
    case InstructionType::XOR:     return "XOR";
    case InstructionType::SHL:     return "SHL";
    case InstructionType::SHR:     return "SHR";
    case InstructionType::CMP:     return "CMP";
    case InstructionType::JMP:     return "JMP";
    case InstructionType::JE:      return "JE";
    case InstructionType::JNE:     return "JNE";
    case InstructionType::JG:      return "JG";
    case InstructionType::JL:      return "JL";
    case InstructionType::JLE:     return "JLE";
    case InstructionType::JGE:     return "JGE";
    case InstructionType::LOAD:    return "LOAD";
    case InstructionType::STORE:   return "STORE";
    case InstructionType::PUSH:    return "PUSH";
    case InstructionType::POP:     return "POP";
    case InstructionType::CALL:    return "CALL";
    case InstructionType::RET:     return "RET";
    default:                        return "UNKNOWN";
    }
}

BytecodeOp Compiler::instrToBCOp(InstructionType it) {
    switch (it) {
    case InstructionType::MOV: return BC_MOV;
    case InstructionType::ADD: return BC_ADD;
    case InstructionType::SUB: return BC_SUB;
    case InstructionType::MUL: return BC_MUL;
    case InstructionType::DIV: return BC_DIV;
    case InstructionType::AND: return BC_AND;
    case InstructionType::OR:  return BC_OR;
    case InstructionType::XOR: return BC_XOR;
    case InstructionType::SHL: return BC_SHL;
    case InstructionType::SHR: return BC_SHR;
    case InstructionType::CMP: return BC_CMP;
    case InstructionType::JMP: return BC_JMP;
    case InstructionType::JE:  return BC_JE;
    case InstructionType::JNE: return BC_JNE;
    case InstructionType::JG:  return BC_JG;
    case InstructionType::JL:  return BC_JL;
    case InstructionType::JLE: return BC_JLE;
    case InstructionType::JGE: return BC_JGE;
    case InstructionType::LOAD:return BC_LOAD;
    case InstructionType::STORE:return BC_STORE;
    case InstructionType::PUSH:return BC_PUSH;
    case InstructionType::POP: return BC_POP;
    case InstructionType::CALL:return BC_CALL;
    case InstructionType::RET: return BC_RET;
    default:
        throw std::runtime_error("Invalid instruction type in compiler");
    }
}

void Compiler::emitByte(std::vector<uint8_t>& segment, uint8_t byte) {
    segment.push_back(byte);
    if (debug_) {
        std::cout << "[Compiler Debug] Emitted byte: 0x"
            << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << std::dec << "\n";
    }
}

void Compiler::emitInt32(std::vector<uint8_t>& segment, int32_t value) {
    for (int i = 0; i < 4; ++i) {
        uint8_t byte = static_cast<uint8_t>((value >> (i * 8)) & 0xFF);
        segment.push_back(byte);
        if (debug_) {
            std::cout << "[Compiler Debug] Emitted int32 byte: 0x"
                << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << std::dec
                << " (" << byte << ")\n";
        }
    }
}

void Compiler::emitOperand(ObjectFile& obj, const Operand& op, std::vector<Fixup>& fixups) {
    if (op.isLabel) {
        // Type 4: Label Address (to be fixed up)
        uint8_t type = 4;
        emitByte(obj.codeSegment, type);
        int32_t placeholder = 0;
        int32_t bytecodeOffset = static_cast<int32_t>(obj.codeSegment.size());
        emitInt32(obj.codeSegment, placeholder);

        Fixup fix;
        fix.bytecodeOffset = bytecodeOffset;
        fix.symbolName = op.labelName;
        fix.isDataLabel = (dataLabels_.find(op.labelName) != dataLabels_.end());
        fix.isMemoryReference = op.isMemory;
        fixups.push_back(fix);

        if (debug_) {
            std::cout << "[Compiler Debug] Recorded fixup for label '"
                << op.labelName << "' at bytecode offset "
                << bytecodeOffset << "\n";
        }
    }
    else if (op.isMemory) {
        if (op.isRegister) {
            // Type 3: mem(reg)
            uint8_t type = 3;
            emitByte(obj.codeSegment, type);
            emitInt32(obj.codeSegment, op.regIndex);
            if (debug_) {
                std::cout << "[Compiler Debug] Emitted memory operand with register R"
                    << op.regIndex << "\n";
            }
        }
        else {
            // Type 2: mem(imm)
            uint8_t type = 2;
            emitByte(obj.codeSegment, type);
            emitInt32(obj.codeSegment, static_cast<int32_t>(op.immediate));
            if (debug_) {
                std::cout << "[Compiler Debug] Emitted memory operand with immediate "
                    << op.immediate << "\n";
            }
        }
    }
    else if (op.isRegister) {
        // Type 1: Register
        uint8_t type = 1;
        emitByte(obj.codeSegment, type);
        emitInt32(obj.codeSegment, op.regIndex);
        if (debug_) {
            std::cout << "[Compiler Debug] Emitted register operand R"
                << op.regIndex << "\n";
        }
    }
    else {
        // Type 0: Immediate
        uint8_t type = 0;
        emitByte(obj.codeSegment, type);
        emitInt32(obj.codeSegment, static_cast<int32_t>(op.immediate));
        if (debug_) {
            std::cout << "[Compiler Debug] Emitted immediate operand "
                << op.immediate << "\n";
        }
    }
}

void Compiler::debugPrint(const std::string& message) const {
    if (debug_) {
        std::cout << "[Compiler Debug] " << message << "\n";
    }
}

ObjectFile Compiler::compile() {
    ObjectFile objFile;
    std::vector<Fixup> fixups;

    debugPrint("Starting compilation process...");

    for (const auto& ins : instructions_) {
        if (ins.type == InstructionType::INVALID) {
            Symbol sym;
            sym.name = ins.label;
            sym.address = static_cast<int32_t>(objFile.codeSegment.size());
            sym.isExternal = false;
            objFile.symbolTable.push_back(sym);

            debugPrint("Defined label '" + ins.label + "' at address " + std::to_string(sym.address));
            continue;
        }

        BytecodeOp op = instrToBCOp(ins.type);
        emitByte(objFile.codeSegment, static_cast<uint8_t>(op));
        debugPrint("Emitted opcode: " + bcOpName(op) + " for instruction " + instructionTypeName(ins.type));

        for (const auto& operand : ins.operands) {
            emitOperand(objFile, operand, fixups);
        }
    }

    debugPrint("All instructions emitted. Handling data segment...");


    size_t dataBaseAddress = objFile.codeSegment.size();
    objFile.codeSize = dataBaseAddress;

    for (const auto& [label, offset] : dataLabels_) {
        Symbol sym;
        sym.name = label;
        sym.address = static_cast<int32_t>(dataBaseAddress + offset * 4);
        sym.isExternal = false;
        sym.isData = true;
        objFile.symbolTable.push_back(sym);

        debugPrint("Defined data label '" + label + "' at address " + std::to_string(sym.address));
    }

    for (const auto& dataWord : dataSegment_) {
        emitInt32(objFile.dataSegment, dataWord);
    }

    debugPrint("Data segment appended. Total bytecode size: " +
        std::to_string(objFile.codeSegment.size() + objFile.dataSegment.size()) + " bytes");

    objFile.fixups = fixups;

    debugPrint("Compilation process completed.");

    if (debug_) {
        printBytecode(objFile.codeSegment, objFile.codeSize);
        std::cout << "\n--- Data Segment ---\n";
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

    return objFile;
}
