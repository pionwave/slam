
#include "VM.hpp"
#include <iostream>
#include <string>

    
VM::VM(const std::vector<uint8_t>& memoryImage, std::size_t memSize, std::size_t stackSize, bool debug)
    : memory_(memoryImage), stack_(stackSize, 0), debug_(debug)
{
    memset(regs_, 0, sizeof(regs_));

    memory_.resize(memSize - memory_.size());

    // R15 is the instruction pointer, always starts at 0
    regs_[15] = 0;

    // Initialize stack pointer (R14) at the end of the memory
    regs_[14] = static_cast<int32_t>(stackSize);

    // Push a -1 return address as a sentinel to end the program
    regs_[14] -= 4;
    storeStack(regs_[14], -1);

    // Clear comparison flags
    flags_[0] = flags_[1] = flags_[2] = 0;
}

void VM::run() 
{

    while (true) {
        int32_t curIp = regs_[15];
        if (regs_[15] < 0 || static_cast<size_t>(regs_[15]) >= memory_.size()) {
            throw std::runtime_error("Instruction pointer out of range");
        }
        uint8_t op = fetchByte();
        int operandCount = operandCountForOp(static_cast<BytecodeOp>(op));

        if (debug_)
            std::cout << "Operand count: " << std::to_string(operandCount) << "\n";

        std::vector<uint8_t> types;
        std::vector<int32_t> values;
        for (int i = 0; i < operandCount; i++) {
            if (regs_[15] + 4 > memory_.size()) {
                throw std::runtime_error("Operand fetch out of memory bounds");
            }
            uint8_t t = fetchByte();
            int32_t v = fetchInt32();
            types.push_back(t);
            values.push_back(v);
        }

        if (debug_)
        {
            debugInstruction(curIp, static_cast<BytecodeOp>(op), types, values);
        }

        if (static_cast<BytecodeOp>(op) == BC_RET) {
            if (opRetImpl()) {
                break;
            }
        }
        else {
            execInstruction(static_cast<BytecodeOp>(op), types, values);
        }
    }
}

void VM::printRegisters() {
    std::cout << "Register values after execution:\n";
    for (int i = 0; i < 16; i++) {
        std::cout << "R" << i << " = " << regs_[i] << "\n";
    }
}


uint8_t VM::fetchByte() {
    return memory_[regs_[15]++];
}


int32_t VM::fetchInt32() {
    int32_t v = 0;
    for (int i = 0; i < 4; i++) {
        v |= ((int32_t)fetchByte()) << (i * 8);
    }
    return v;
}

void VM::checkMem(int32_t addr) {
    if (addr < 0 || (size_t)(addr + 3) >= memory_.size()) {
        throw std::runtime_error("Memory out of range at address " + std::to_string(addr));
    }
}

int32_t VM::loadMem(int32_t addr) {
    checkMem(addr);
    int32_t v = 0;
    for (int i = 0; i < 4; i++) {
        v |= ((int32_t)memory_[addr + i]) << (i * 8);
    }
    return v;
}

void VM::storeMem(int32_t addr, int32_t val) {
    checkMem(addr);
    for (int i = 0; i < 4; i++) {
        memory_[addr + i] = (uint8_t)((val >> (i * 8)) & 0xFF);
    }
}

void VM::checkStack(int32_t addr) {
    if (addr < 0 || (size_t)(addr + 3) >= stack_.size()) {
        throw std::runtime_error("Stack out of range at address " + std::to_string(addr));
    }
}

int32_t VM::loadStack(int32_t addr) {
    checkStack(addr);
    int32_t v = 0;
    for (int i = 0; i < 4; i++) {
        v |= ((int32_t)stack_[addr + i]) << (i * 8);
    }
    return v;
}

void VM::storeStack(int32_t addr, int32_t val) {
    checkStack(addr);
    for (int i = 0; i < 4; i++) {
        stack_[addr + i] = (uint8_t)((val >> (i * 8)) & 0xFF);
    }
}

int32_t VM::operandValue(uint8_t type, int32_t val) {
    switch (type) {
    case 0: return val; // imm
    case 1: return regs_[val]; // reg
    case 2: return loadMem(val); // mem(imm)
    case 3: return loadMem(regs_[val]); // mem(reg)
    case 4: return val; // label address
    default: throw std::runtime_error("Invalid operand type");
    }
}

void VM::setOperandDest(uint8_t type, int32_t valDescriptor, int32_t value) {
    switch (type) {
    case 1: regs_[valDescriptor] = value; break; // reg
    case 2: storeMem(valDescriptor, value); break; // mem(imm)
    case 3: storeMem(regs_[valDescriptor], value); break; // mem(reg)
    default:
        throw std::runtime_error("Destination operand must be reg or mem: " + std::to_string(type));
    }
}

// Instruction execution methods
void VM::execInstruction(BytecodeOp op, const std::vector<uint8_t>& types, const std::vector<int32_t>& vals)
{
    switch (op) {
    case BC_MOV: binOp(&VM::opMov, types, vals); break;
    case BC_ADD: triOp(&VM::opAddThree, types, vals); break;
    case BC_SUB: triOp(&VM::opSubThree, types, vals); break;
    case BC_MUL: triOp(&VM::opMulThree, types, vals); break;
    case BC_DIV: triOp(&VM::opDivThree, types, vals); break;
    case BC_AND: binOp(&VM::opAnd, types, vals); break;
    case BC_OR:  binOp(&VM::opOr, types, vals); break;
    case BC_XOR: binOp(&VM::opXor, types, vals); break;
    case BC_SHL: binOp(&VM::opShl, types, vals); break;
    case BC_SHR: binOp(&VM::opShr, types, vals); break;
    case BC_CMP: binOp(&VM::opCmp, types, vals); break;
    case BC_JMP: jumpOp(&VM::opJmp, types, vals); break;
    case BC_JE:  jumpOp(&VM::opJe, types, vals); break;
    case BC_JNE: jumpOp(&VM::opJne, types, vals); break;
    case BC_JG:  jumpOp(&VM::opJg, types, vals); break;
    case BC_JL:  jumpOp(&VM::opJl, types, vals); break;
    case BC_JLE: jumpOp(&VM::opJle, types, vals); break;
    case BC_JGE: jumpOp(&VM::opJge, types, vals); break;
    case BC_LOAD:binOp(&VM::opLoad, types, vals); break;
    case BC_STORE:binOp(&VM::opStore, types, vals); break;
    case BC_PUSH: unaryOp(&VM::opPush, types, vals); break;
    case BC_POP:  unaryOp(&VM::opPop, types, vals); break;
    case BC_CALL: jumpOp(&VM::opCall, types, vals); break;
        // BC_RET handled in run()
    default:
        throw std::runtime_error("Invalid opcode");
    }
}

void VM::binOp(void (VM::* f)(uint8_t, int32_t, int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v) {
    (this->*f)(t[0], v[0], operandValue(t[1], v[1]));
}

void VM::unaryOp(void (VM::* f)(uint8_t, int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v) {
    (this->*f)(t[0], v[0]);
}

void VM::jumpOp(void (VM::* f)(int32_t), const std::vector<uint8_t>& t, const std::vector<int32_t>& v) {
    (this->*f)(v[0]);
}

void VM::triOp(void (VM::* f)(uint8_t, int32_t, int32_t, int32_t),
    const std::vector<uint8_t>& t, const std::vector<int32_t>& v) {
    // t[0] = destType, v[0] = destVal
    // t[1], v[1] = first source
    // t[2], v[2] = second source
    int32_t src1 = operandValue(t[1], v[1]);
    int32_t src2 = operandValue(t[2], v[2]);
    (this->*f)(t[0], v[0], src1, src2);
}

void VM::opAddThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2) {
    setOperandDest(dt, dv, src1 + src2);
}

void VM::opSubThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2) {
    setOperandDest(dt, dv, src1 - src2);
}

void VM::opMulThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2) {
    setOperandDest(dt, dv, src1 * src2);
}

void VM::opDivThree(uint8_t dt, int32_t dv, int32_t src1, int32_t src2) {
    setOperandDest(dt, dv, src1 / src2);
}

// Arithmetic ops:
void VM::opMov(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, sv); }
void VM::opAdd(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) + sv); }
void VM::opSub(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) - sv); }
void VM::opMul(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) * sv); }
void VM::opDiv(uint8_t dt, int32_t dv, int32_t sv) {
    int32_t dval = operandValue(dt, dv);
    if (sv == 0) throw std::runtime_error("Division by zero");
    setOperandDest(dt, dv, dval / sv);
}
void VM::opAnd(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) & sv); }
void VM::opOr(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) | sv); }
void VM::opXor(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) ^ sv); }
void VM::opShl(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, operandValue(dt, dv) << sv); }
void VM::opShr(uint8_t dt, int32_t dv, int32_t sv) {
    uint32_t val = (uint32_t)operandValue(dt, dv);
    setOperandDest(dt, dv, (int32_t)(val >> sv));
}
void VM::opCmp(uint8_t dt, int32_t dv, int32_t sv) {
    int32_t res = operandValue(dt, dv) - sv;
    flags_[0] = (res == 0) ? 1 : 0; // ZF
    flags_[1] = (res > 0) ? 1 : 0;  // GF
    flags_[2] = (res < 0) ? 1 : 0;  // LF
}

void VM::opLoad(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, sv); }
void VM::opStore(uint8_t dt, int32_t dv, int32_t sv) { setOperandDest(dt, dv, sv); }

void VM::opPush(uint8_t t, int32_t v) {
    int32_t val = operandValue(t, v);
    regs_[14] -= 4;
    storeStack(regs_[14], val);
}

void VM::opPop(uint8_t t, int32_t v) {
    int32_t val = loadStack(regs_[14]);
    regs_[14] += 4;
    setOperandDest(t, v, val);
}


void VM::opJmp(int32_t addr) { regs_[15] = addr; }
void VM::opJe(int32_t addr) { if (flags_[0]) regs_[15] = addr; }
void VM::opJne(int32_t addr) { if (!flags_[0]) regs_[15] = addr; }
void VM::opJg(int32_t addr) { if (flags_[1]) regs_[15] = addr; }
void VM::opJl(int32_t addr) { if (flags_[2]) regs_[15] = addr; }
void VM::opJle(int32_t addr)
{
    if (flags_[0] || flags_[2]) { // ZF or LF

        regs_[15] = addr;
    }
}

void VM::opJge(int32_t addr) {
    if (flags_[0] || flags_[1]) { // ZF or GF
        regs_[15] = addr;
    }
}
void VM::opCall(int32_t addr) {
    int32_t retAddr = regs_[15];
    regs_[14] -= 4;
    storeStack(regs_[14], retAddr);
    regs_[15] = addr;
}

// Instead of opRet, we have:
bool VM::opRetImpl() {
    int32_t retAddr = loadStack(regs_[14]);
    regs_[14] += 4;
    if (retAddr == -1) {
        // End program
        return true;
    }
    regs_[15] = retAddr;
    return false;
}

int VM::operandCountForOp(BytecodeOp op) {
    switch (op)
    {
    case BC_ADD: case BC_SUB: case BC_MUL: case BC_DIV:
        return 3;
    case BC_MOV:
    case BC_AND: case BC_OR: case BC_XOR: case BC_SHL: case BC_SHR:
    case BC_CMP: case BC_LOAD: case BC_STORE:
        return 2;
    case BC_PUSH: case BC_POP:
    case BC_JMP: case BC_JE: case BC_JNE: case BC_JG: case BC_JL:
    case BC_JLE: case BC_JGE:
    case BC_CALL:
        return 1;
    case BC_RET:
        return 0;
    default:
        return 0;
    }
}

std::string VM::bcOpName(BytecodeOp op) {
    switch (op) {
    case BC_MOV:return "MOV";
    case BC_ADD:return "ADD";
    case BC_SUB:return "SUB";
    case BC_MUL:return "MUL";
    case BC_DIV:return "DIV";
    case BC_AND:return "AND";
    case BC_OR:return "OR";
    case BC_XOR:return "XOR";
    case BC_SHL:return "SHL";
    case BC_SHR:return "SHR";
    case BC_CMP:return "CMP";
    case BC_JMP:return "JMP";
    case BC_JE:return "JE";
    case BC_JNE:return "JNE";
    case BC_JG:return "JG";
    case BC_JL:return "JL";
    case BC_JLE:return "JLE";
    case BC_JGE:return "JGE";
    case BC_LOAD:return "LOAD";
    case BC_STORE:return "STORE";
    case BC_PUSH:return "PUSH";
    case BC_POP:return "POP";
    case BC_CALL:return "CALL";
    case BC_RET:return "RET";
    default:return "UNKNOWN";
    }
}

void VM::debugInstruction(int32_t ip, BytecodeOp op, const std::vector<uint8_t>& types, const std::vector<int32_t>& vals) {
    std::cout << "Executing at IP=0x" << std::hex << ip << std::dec << ": " << bcOpName(op);

    for (size_t i = 0; i < types.size(); i++) {
        std::cout << " ";
        printOperand(types[i], vals[i]);
    }

    std::cout << "\n";
}

void VM::printOperand(uint8_t t, int32_t v) {
    int32_t mem = 0;

    if (t == 2)
    {
        mem = loadMem(v);
    }

    switch (t) {
    case 0: std::cout << "imm(" << v << ")"; break;
    case 1: std::cout << "reg(R" << v << ")=" << regs_[v]; break;
    case 2: std::cout << "mem(" << v << ")=" << mem; break;
    case 3: std::cout << "mem(R" << v << ")"; break;
    case 4: std::cout << "labelAddr(" << v << ")"; break;
    default: std::cout << "invalid_type(" << (int)t << ")";
    }
}
