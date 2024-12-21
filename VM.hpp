
#pragma once

#include <vector>
#include "BytecodeOp.hpp"
#include <stdexcept>

class VM {
public:
    // 1MB memory model with 64kb stack
    VM(const std::vector<uint8_t>& memoryImage, std::size_t memSize = 1048576, std::size_t stackSize = 65536, bool debug = false);

    void run();

    void printRegisters();

private:
    int32_t regs_[16]; // R0-R15
    std::vector<uint8_t> memory_;
    std::vector<uint8_t> stack_;
    int32_t flags_[3]; // 0=ZF, 1=GF, 2=LF
    bool debug_ = false;

    uint8_t fetchByte();
    int32_t fetchInt32();

    void checkMem(int32_t addr);
    int32_t loadMem(int32_t addr);
    void storeMem(int32_t addr, int32_t val);
    void checkStack(int32_t addr);
    int32_t loadStack(int32_t addr);
    void storeStack(int32_t addr, int32_t val);

    int32_t operandValue(uint8_t type, int32_t val);
    void setOperandDest(uint8_t type, int32_t valDescriptor, int32_t value);

    // Instruction execution methods
    void execInstruction(BytecodeOp op, const std::vector<uint8_t>& types, const std::vector<int32_t>& vals);

    void binOp(void (VM::* f)(uint8_t, int32_t, int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v);
    void unaryOp(void (VM::* f)(uint8_t, int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v);
    void jumpOp(void (VM::* f)(int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v);
    void triOp(void (VM::* f)(uint8_t, int32_t, int32_t, int32_t),
        const std::vector<uint8_t>& t, const std::vector<int32_t>& v);

    void opAddThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2);
    void opSubThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2);
    void opMulThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2);
    void opDivThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2);

    // Arithmetic ops:
    void opMov(uint8_t dt, int32_t dv, int32_t sv);
    void opAdd(uint8_t dt, int32_t dv, int32_t sv);
    void opSub(uint8_t dt, int32_t dv, int32_t sv);
    void opMul(uint8_t dt, int32_t dv, int32_t sv);
    void opDiv(uint8_t dt, int32_t dv, int32_t sv);
    void opAnd(uint8_t dt, int32_t dv, int32_t sv);
    void opOr(uint8_t dt, int32_t dv, int32_t sv);
    void opXor(uint8_t dt, int32_t dv, int32_t sv);
    void opShl(uint8_t dt, int32_t dv, int32_t sv);
    void opShr(uint8_t dt, int32_t dv, int32_t sv);
    void opCmp(uint8_t dt, int32_t dv, int32_t sv);

    void opLoad(uint8_t dt, int32_t dv, int32_t sv);
    void opStore(uint8_t dt, int32_t dv, int32_t sv);

    void opPush(uint8_t t, int32_t v);
    void opPop(uint8_t t, int32_t v);


    void opJmp(int32_t addr);
    void opJe(int32_t addr);
    void opJne(int32_t addr);
    void opJg(int32_t addr);
    void opJl(int32_t addr);
    void opJle(int32_t addr);

    void opJge(int32_t addr);
    void opCall(int32_t addr);

    // Instead of opRet, we have:
    bool opRetImpl();

    int operandCountForOp(BytecodeOp op);
    std::string bcOpName(BytecodeOp op);

    void debugInstruction(int32_t ip, BytecodeOp op, const std::vector<uint8_t>& types, const std::vector<int32_t>& vals);
    void printOperand(uint8_t t, int32_t v);
};