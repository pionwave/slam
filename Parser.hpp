// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include "Lexer.hpp"

enum class InstructionType {
    MOV = 0, ADD = 1, SUB = 2, MUL = 3, DIV = 4, AND = 5, OR = 6, XOR = 7, SHL = 8, SHR = 9, CMP = 10,
    JMP = 11, JE = 12, JNE = 13, JG = 14, JL = 15, JLE = 16, JGE = 17,
    LOAD = 18, STORE = 19,
    PUSH = 20, POP = 21,
    CALL = 22, RET = 23,
    INVALID = 24
};


struct Operand {
    bool isRegister = false;
    bool isMemory = false;
    int32_t regIndex = -1;
    int64_t immediate = 0;
    bool isLabel = false;
    std::string labelName;
    int32_t line;
    int32_t col;
};


struct Instruction {
    InstructionType type;
    std::vector<Operand> operands;
    std::string label;
    int32_t line;
    int32_t col;
};

enum class Section {
    CODE = 0,
    DATA
};

class Parser {
public:
    Parser(const std::vector<std::string>& lines, bool debug = false);

    const std::vector<Instruction>& getInstructions() const;
    const std::vector<int32_t>& getDataSegment() const;
    const std::unordered_map<std::string, int32_t>& getDataLabels() const;

private:
    void parseLine(Lexer& lex);
    void handleDirective(const Token& t);

    InstructionType strToInstr(const std::string& s);
    int regNameToIndex(const std::string& r, int32_t line, int32_t col);

    Operand parseOperand(Lexer& lex);
    void parseInstructionAfterIdent(Lexer& lex, const std::string& mnemonic, int32_t line, int32_t col);
    void parseDataLine(Lexer& lex);
    void parseWordList(Lexer& lex, const std::string& labelName);

    void error(const std::string& msg, int32_t line, int32_t col) const;

    std::vector<Instruction> instructions_;
    Section currentSection_ = Section::CODE;

    std::vector<int32_t> dataSegment_;
    std::unordered_map<std::string, int32_t> dataLabels_;
    int32_t dataOffset_ = 0;
    bool debug_ = false;
};