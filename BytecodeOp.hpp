
#pragma once

enum BytecodeOp {
    BC_MOV, BC_ADD, BC_SUB, BC_MUL, BC_DIV,
    BC_AND, BC_OR, BC_XOR, BC_SHL, BC_SHR,
    BC_CMP,
    BC_JMP, BC_JE, BC_JNE, BC_JG, BC_JL, BC_JLE, BC_JGE,
    BC_LOAD, BC_STORE,
    BC_PUSH, BC_POP,
    BC_CALL, BC_RET
};