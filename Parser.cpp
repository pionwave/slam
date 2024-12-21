
#include "Parser.hpp"
#include <iostream>

std::string tokenTypeName(TokenType t);

Parser::Parser(const std::vector<std::string>& lines, bool debug) : debug_(debug)
{
    for (size_t i = 0; i < lines.size(); i++) {
        int currentLine = (int)i + 1;

        if (debug_)
        {
            Lexer debugLexer(lines[i], currentLine);
            std::cout << "Line " << currentLine << " tokens:\n";
            while (!debugLexer.ended()) {
                Token tk = debugLexer.currentToken();
                std::cout << "  " << tokenTypeName(tk.type) << " \"" << tk.text << "\" (line " << tk.line << ", col " << tk.col << ")\n";
                debugLexer.nextToken();
            }
        }

        Lexer lexer(lines[i], currentLine);
        parseLine(lexer);
    }
}

const std::vector<Instruction>& Parser::getInstructions() const {
    return instructions_;
}

const std::vector<int32_t>& Parser::getDataSegment() const {
    return dataSegment_;
}

const std::unordered_map<std::string, int>& Parser::getDataLabels() const
{
    return dataLabels_;
}


void Parser::parseLine(Lexer& lex) {
    if (lex.ended()) return;

    Token first = lex.currentToken();
    if (first.type == TokenType::T_END_OF_FILE) {
        return; // empty line
    }

    if (first.type == TokenType::T_DIRECTIVE) {
        handleDirective(first);
        lex.nextToken();
        return;
    }

    // Check which section we're currently in
    if (currentSection_ == Section::DATA) {
        // In data section, use parseDataLine to handle this line
        parseDataLine(lex);
        return;
    }

    if (first.type == TokenType::T_LABEL) {
        std::string labelName = first.text;
        int lline = first.line;
        int lcol = first.col;
        lex.nextToken();

        if (lex.currentToken().type == TokenType::T_COLON) {
            lex.nextToken(); // consume colon
            Instruction ins{};
            ins.type = InstructionType::INVALID;
            ins.label = labelName;
            ins.line = lline;
            ins.col = lcol;
            instructions_.push_back(ins);

            // check if there's an instruction after the label on the same line
            if (!lex.ended() && lex.currentToken().type == TokenType::T_INSTRUCTION) {
                parseInstructionAfterIdent(lex, lex.currentToken().text, lex.currentToken().line, lex.currentToken().col);
            }
            return;
        }
        else {
            error("Expected ':' after label", first.line, first.col);
        }
    }

    if (first.type == TokenType::T_INSTRUCTION) {
        parseInstructionAfterIdent(lex, first.text, first.line, first.col);
    }
    else {
        error("Invalid line start", first.line, first.col);
    }
}

void Parser::handleDirective(const Token& t) {
    if (t.text == "DATA") {
        currentSection_ = Section::DATA;
    }
    else if (t.text == "CODE") {
        currentSection_ = Section::CODE;
    }
    else {
        error("Unknown directive: " + t.text, t.line, t.col);
    }
}

InstructionType Parser::strToInstr(const std::string& s) {
    static std::unordered_map<std::string, InstructionType> m = {
        {"MOV", InstructionType::MOV}, {"ADD", InstructionType::ADD}, {"SUB", InstructionType::SUB},
        {"MUL", InstructionType::MUL}, {"DIV", InstructionType::DIV}, {"AND", InstructionType::AND},
        {"OR", InstructionType::OR}, {"XOR", InstructionType::XOR}, {"SHL", InstructionType::SHL},
        {"SHR", InstructionType::SHR}, {"CMP", InstructionType::CMP}, {"JMP", InstructionType::JMP},
        {"JE", InstructionType::JE}, {"JNE", InstructionType::JNE}, {"JG", InstructionType::JG},
        {"JL", InstructionType::JL}, {"JLE", InstructionType::JLE}, {"JGE", InstructionType::JGE},
        {"LOAD", InstructionType::LOAD}, {"STORE", InstructionType::STORE},
        {"PUSH", InstructionType::PUSH}, {"POP", InstructionType::POP}, {"CALL", InstructionType::CALL},
        {"RET", InstructionType::RET}
    };
    auto it = m.find(s);
    if (it != m.end()) return it->second;
    return InstructionType::INVALID;
}

int Parser::regNameToIndex(const std::string& r, int line, int col) {
    int idx = std::stoi(r.substr(1));
    return idx;
}

Operand Parser::parseOperand(Lexer& lex) {
    Token t = lex.currentToken();
    Operand op{};
    op.line = t.line;
    op.col = t.col;

    if (t.type == TokenType::T_REGISTER) {
        op.isRegister = true;
        op.regIndex = regNameToIndex(t.text, t.line, t.col);
        lex.nextToken();
    }
    else if (t.type == TokenType::T_LABEL) {
        op.isLabel = true;
        op.labelName = t.text;
        lex.nextToken();
    }
    else if (t.type == TokenType::T_INT) {
        op.immediate = t.value;
        lex.nextToken();
    }
    else if (t.type == TokenType::T_LBRACKET) {
        lex.nextToken(); // '['
        Token inner = lex.currentToken();
        if (inner.type == TokenType::T_REGISTER) {
            op.isMemory = true;
            op.isRegister = true;
            op.regIndex = regNameToIndex(inner.text, inner.line, inner.col);
            lex.nextToken();
        }
        else if (inner.type == TokenType::T_INT) {
            op.isMemory = true;
            op.immediate = inner.value;
            lex.nextToken();
        }
        else if (inner.type == TokenType::T_LABEL) {
            op.isMemory = true;
            op.isLabel = true;
            op.labelName = inner.text;
            lex.nextToken();
        }
        else {
            error("Invalid memory operand", inner.line, inner.col);
        }

        if (lex.currentToken().type != TokenType::T_RBRACKET) {
            Token cur = lex.currentToken();
            error("Missing closing bracket in memory operand", cur.line, cur.col);
        }
        lex.nextToken(); // ']'
    }
    else {
        error("Invalid operand", t.line, t.col);
    }

    return op;
}

void Parser::parseInstructionAfterIdent(Lexer& lex, const std::string& mnemonic, int line, int col) {
    InstructionType itype = strToInstr(mnemonic);
    if (itype == InstructionType::INVALID) {
        error("Invalid instruction: " + mnemonic, line, col);
    }

    Instruction ins{};
    ins.type = itype;
    ins.line = line;
    ins.col = col;

    lex.nextToken(); // consume the instruction

    int expectedOperands = 0;
    switch (itype)
    {
    case InstructionType::ADD: case InstructionType::SUB:
    case InstructionType::MUL: case InstructionType::DIV:
        expectedOperands = 3; break;

    case InstructionType::MOV: case InstructionType::AND:
    case InstructionType::OR: case InstructionType::XOR: case InstructionType::SHL:
    case InstructionType::SHR: case InstructionType::CMP: case InstructionType::LOAD:
    case InstructionType::STORE:
        expectedOperands = 2; break;

    case InstructionType::PUSH: case InstructionType::POP:
    case InstructionType::JMP: case InstructionType::JE: case InstructionType::JNE:
    case InstructionType::JG: case InstructionType::JL: case InstructionType::CALL:
    case InstructionType::JLE: case InstructionType::JGE:
        expectedOperands = 1; break;

    case InstructionType::RET:
        expectedOperands = 0; break;

    default: break;
    }

    for (int i = 0; i < expectedOperands; i++) {
        if (lex.ended() || lex.currentToken().type == TokenType::T_END_OF_FILE) {
            error("Not enough operands for " + mnemonic, line, col);
        }

        Operand op = parseOperand(lex);
        ins.operands.push_back(op);

        if (i < expectedOperands - 1) {
            if (lex.currentToken().type != TokenType::T_COMMA) {
                Token cur = lex.currentToken();
                error("Expected comma after operand", cur.line, cur.col);
            }
            lex.nextToken(); // consume comma
        }
    }

    instructions_.push_back(ins);
}

void Parser::parseDataLine(Lexer& lex) {
    if (lex.ended()) return;

    std::string labelName;
    Token first = lex.currentToken();
    if (first.type == TokenType::T_LABEL) {
        labelName = first.text;
        lex.nextToken();
        if (lex.ended() || lex.currentToken().type != TokenType::T_COLON) {
            error("Expected ':' after label", first.line, first.col);
        }
        lex.nextToken(); // consume colon
    }

    if (lex.ended()) {
        // Just a label line in data, no data directive
        if (!labelName.empty()) {
            dataLabels_[labelName] = dataOffset_;
        }
        return;
    }

    // Expect a data directive like "WORD"
    Token dtok = lex.currentToken();
    if (dtok.type == TokenType::T_DIRECTIVE && dtok.text == "WORD") {
        lex.nextToken(); // consume WORD directive
        parseWordList(lex, labelName);
    }
    else {
        error("Expected .word directive in data section", dtok.line, dtok.col);
    }
}

void Parser::parseWordList(Lexer& lex, const std::string& labelName) {
    bool firstVal = true;
    while (!lex.ended()) {
        Token val = lex.currentToken();
        if (val.type == TokenType::T_INT) {
            int32_t wordVal = (int32_t)val.value;
            dataSegment_.push_back(wordVal);
            dataOffset_ += 4;
            lex.nextToken();
            if (lex.ended()) break;
            if (lex.currentToken().type == TokenType::T_COMMA) {
                lex.nextToken();
                continue;
            }
            else {
                break;
            }
        }
        else {
            break;
        }
    }

    if (!labelName.empty()) {
        // label points to start of this data
        dataLabels_[labelName] = dataOffset_ - 4; // label at first word offset
    }
}

void Parser::error(const std::string& msg, int line, int col) const {
    throw std::runtime_error("Parse error at line " + std::to_string(line) +
        ", col " + std::to_string(col) + ": " + msg);
}

